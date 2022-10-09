#include "./KrunkerWindow.h"
#include "../utils/Base64.h"
#include "../utils/JsonUtil.h"
#include "../utils/StringUtil.h"
#include "../utils/Uri.h"
#include "./LoadRes.h"
#include "./Log.h"
#include "./Site.h"
#include "./resource.h"
#include <WebView2EnvironmentOptions.h>
#include <rapidjson/writer.h>
#include <regex>
#include <sstream>

using Microsoft::WRL::Callback;
using Microsoft::WRL::Make;

long long KrunkerWindow::now() {
  return duration_cast<std::chrono::milliseconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}

KrunkerWindow::KrunkerWindow(ClientFolder &_folder, Type _type, Vector2 s,
                             std::wstring _title,
                             std::function<void()> _on_startup,
                             std::function<bool(JSMessage)> _on_unknown_message,
                             std::function<void()> _on_destroy_callback)
    : type(_type), title(_title), og_title(_title), scale(s), folder(_folder),
      last_pointer_poll(now()), on_webview2_startup(_on_startup),
      on_unknown_message(_on_unknown_message),
      on_destroy_callback(_on_destroy_callback) {
  std::string str_js_frontend;

  if (load_resource(JS_FRONTEND, str_js_frontend)) {
    js_frontend = ST::wstring(str_js_frontend) +
                  L"//# sourceMappingURL=https://chief/frontend.js.map";
  } else {
    clog::error << "Failure loading frontend" << clog::endl;
  }

  if (!load_resource(CSS_CLIENT, css_builtin))
    clog::error << "Unable to load built-in CSS" << clog::endl;

  raw_input.usUsagePage = 0x01;
  raw_input.usUsage = 0x02;
}

KrunkerWindow::~KrunkerWindow() {
  if (mouse_hooked)
    unhook_mouse();
  if (::IsWindow(m_hWnd))
    DestroyWindow();
}

HINSTANCE KrunkerWindow::get_hinstance() {
  return (HINSTANCE)GetWindowLong(GWL_HINSTANCE);
}

bool KrunkerWindow::create_window(HINSTANCE inst, int cmdshow) {
  Create(NULL, NULL, title.c_str(), WS_OVERLAPPEDWINDOW);

  Vector2 scr_pos;
  Vector2 scr_size;

  if (monitor_data(scr_pos, scr_size)) {
    Rect2D r;

    r.width = long(scr_size.x * scale.x);
    r.height = long(scr_size.y * scale.y);

    r.x = long(scr_pos.x + ((scr_size.x - r.width) / 2));
    r.y = long(scr_pos.y + ((scr_size.y - r.height) / 2));

    SetWindowPos(NULL, r.get(), 0);
  } else
    ResizeClient(700, 500);

  ShowWindow(cmdshow);
  UpdateWindow();

  return true;
}

COREWEBVIEW2_COLOR KrunkerWindow::ColorRef(COLORREF color) {
  return COREWEBVIEW2_COLOR{255, GetRValue(color), GetGValue(color),
                            GetBValue(color)};
}

bool KrunkerWindow::enter_fullscreen() {
  if (fullscreen)
    return false;

  RECT screen;

  if (!monitor_data(screen))
    return false;

  GetClientRect(&saved_size);
  ClientToScreen(&saved_size);

  saved_style = GetWindowLong(GWL_STYLE);
  saved_ex_style = GetWindowLong(GWL_EXSTYLE);
  SetWindowLong(GWL_EXSTYLE,
                saved_ex_style & ~(WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE |
                                   WS_EX_CLIENTEDGE | WS_EX_STATICEDGE));
  SetWindowLong(GWL_STYLE, saved_style & ~(WS_CAPTION | WS_THICKFRAME));
  SetWindowPos(0, &screen, SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);

  resize_wv();

  return fullscreen = true;
}

bool KrunkerWindow::exit_fullscreen() {
  if (!fullscreen)
    return false;

  SetWindowLong(GWL_STYLE, saved_style);
  SetWindowLong(GWL_EXSTYLE, saved_ex_style);
  SetWindowPos(NULL, RECT_ARGS(saved_size),
               SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
  resize_wv();

  fullscreen = false;
  return true;
}

bool KrunkerWindow::resize_wv() {
  if (control == nullptr)
    return false;

  RECT bounds;
  GetClientRect(&bounds);
  control->put_Bounds(bounds);

  return true;
}

bool KrunkerWindow::monitor_data(RECT &rect) {
  HMONITOR monitor = MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);
  MONITORINFO info;
  info.cbSize = sizeof(info);

  if (!GetMonitorInfo(monitor, &info)) {
    clog::error << "Can't get monitor info" << clog::endl;
    return false;
  }

  rect = info.rcMonitor;

  return true;
}

bool KrunkerWindow::monitor_data(Vector2 &pos, Vector2 &size) {
  RECT r;

  if (!monitor_data(r))
    return false;

  pos.x = r.left;
  pos.y = r.top;

  size.x = r.right - r.left;
  size.y = r.bottom - r.top;

  return true;
}

LRESULT KrunkerWindow::on_resize(UINT uMsg, WPARAM wParam, LPARAM lParam,
                                 BOOL &fHandled) {
  return resize_wv();
}

Vector2 movebuffer;

JSMessage msgFct(unsigned short event, std::vector<double> nums) {
  JSMessage msg(event);
  for (double num : nums)
    msg.args.PushBack(rapidjson::Value(num), msg.allocator);
  return msg;
}

LRESULT KrunkerWindow::on_input(UINT uMsg, WPARAM wParam, LPARAM lParam,
                                BOOL &fHandled) {
  unsigned size = sizeof(RAWINPUT);
  static RAWINPUT raw[sizeof(RAWINPUT)];
  GetRawInputData((HRAWINPUT)lParam, RID_INPUT, raw, &size,
                  sizeof(RAWINPUTHEADER));

  if (raw->header.dwType == RIM_TYPEMOUSE) {
    RAWMOUSE mouse = raw->data.mouse;
    USHORT flags = mouse.usButtonFlags;

    if (flags & RI_MOUSE_WHEEL)
      msgFct(IM::mousewheel,
             {double((*(short *)&mouse.usButtonData) / WHEEL_DELTA) * -100});
    if (flags & RI_MOUSE_BUTTON_1_DOWN)
      sendMessage(msgFct(IM::mousedown, {0}));
    if (flags & RI_MOUSE_BUTTON_1_UP)
      sendMessage(msgFct(IM::mouseup, {0}));
    if (flags & RI_MOUSE_BUTTON_2_DOWN)
      sendMessage(msgFct(IM::mousedown, {2}));
    if (flags & RI_MOUSE_BUTTON_2_UP)
      sendMessage(msgFct(IM::mouseup, {2}));
    if (flags & RI_MOUSE_BUTTON_3_DOWN)
      sendMessage(msgFct(IM::mousedown, {1}));
    if (flags & RI_MOUSE_BUTTON_3_UP)
      sendMessage(msgFct(IM::mouseup, {1}));
    if (flags & RI_MOUSE_BUTTON_4_DOWN)
      sendMessage(msgFct(IM::mousedown, {3}));
    if (flags & RI_MOUSE_BUTTON_4_UP)
      sendMessage(msgFct(IM::mouseup, {3}));
    if (flags & RI_MOUSE_BUTTON_5_DOWN)
      sendMessage(msgFct(IM::mousedown, {4}));
    if (flags & RI_MOUSE_BUTTON_5_UP)
      sendMessage(msgFct(IM::mouseup, {4}));
    if (mouse.lLastX || mouse.lLastY)
      movebuffer += Vector2(raw->data.mouse.lLastX, raw->data.mouse.lLastY);
  }

  return S_OK;
}

LRESULT CALLBACK KrunkerWindow::mouse_message(int code, WPARAM wParam,
                                              LPARAM lParam) {
  return 1;
}

void KrunkerWindow::hook_mouse() {
  clog::debug << "Hooking mouse" << clog::endl;

  INPUT input = {};
  input.type = INPUT_MOUSE;
  input.mi.dx = 0;
  input.mi.time = 0;
  input.mi.dwExtraInfo = NULL;
  input.mi.dwFlags = (MOUSEEVENTF_MOVE | MOUSEEVENTF_LEFTUP |
                      MOUSEEVENTF_RIGHTUP | MOUSEEVENTF_MIDDLEUP);
  SendInput(1, &input, sizeof(INPUT));

  raw_input.dwFlags = RIDEV_INPUTSINK;
  raw_input.hwndTarget = m_hWnd;
  RegisterRawInputDevices(&raw_input, 1, sizeof(raw_input));

  mouse_hook =
      SetWindowsHookEx(WH_MOUSE_LL, *mouse_message, get_hinstance(), NULL);
  mouse_hooked = true;
}

void KrunkerWindow::unhook_mouse() {
  clog::debug << "Unhooked mouse" << clog::endl;

  raw_input.dwFlags = RIDEV_REMOVE;
  raw_input.hwndTarget = NULL;
  RegisterRawInputDevices(&raw_input, 1, sizeof(raw_input));

  UnhookWindowsHookEx(mouse_hook);
  mouse_hooked = false;
}

// https://peter.sh/experiments/chromium-command-line-switches/
std::wstring KrunkerWindow::cmdline() {
  std::vector<std::wstring> cmds = {
      L"--disable-background-timer-throttling",
      L"--disable-features=msSmartScreenProtection",
      L"--ignore-gpu-blacklist",
      L"--enable-zero-copy",
      L"--webrtc-max-cpu-consumption-percentage=100",
      L"--autoplay-policy=no-user-gesture-required",
  };

  if (folder.config["render"]["uncap_fps"].GetBool()) {
    cmds.push_back(L"--disable-frame-rate-limit");
    // if (!folder.config["render"]["vsync"])
    cmds.push_back(L"--disable-gpu-vsync");
  }

  if (folder.config["render"]["angle"] != "default")
    cmds.push_back(L"--use-angle=" +
                   JT::wstring(folder.config["render"]["angle"]));
  if (folder.config["render"]["color"] != "default")
    cmds.push_back(L"--force-color-profile=" +
                   JT::wstring(folder.config["render"]["color"]));

  std::wstring cmdline;
  bool first = false;

  for (std::wstring cmd : cmds) {
    if (first)
      first = false;
    else
      cmdline += L" ";

    cmdline += cmd;
  }

  return cmdline;
}

bool KrunkerWindow::send_resource(
    ICoreWebView2WebResourceRequestedEventArgs *args, int resource,
    std::wstring mime) {
  IStream *stream = nullptr;
  std::string data;

  if (!load_resource(resource, data)) {
    clog::error << "Unable to load resource " << resource << clog::endl;
    return false;
  }

  if (CreateStreamOnHGlobal(NULL, TRUE, (LPSTREAM *)&stream) != S_OK) {
    clog::error << "Unable to create IStream on HGlobal" << clog::endl;
    return false;
  }

  ULONG written = 0;

  if (stream->Write(data.data(), data.size(), &written) != S_OK) {
    clog::error << "Unable to write data to IStream" << clog::endl;
    return false;
  }

  wil::com_ptr<ICoreWebView2WebResourceResponse> response;
  env->CreateWebResourceResponse(stream, 200, L"OK",
                                 (L"Content-Type: " + mime).c_str(), &response);
  args->put_Response(response.get());

  return true;
}

std::string status_name(COREWEBVIEW2_WEB_ERROR_STATUS status) {
  switch (status) {
  case COREWEBVIEW2_WEB_ERROR_STATUS_CERTIFICATE_COMMON_NAME_IS_INCORRECT:
    return "CertificateErrCommonNameIsIncorrect";
    break;
  case COREWEBVIEW2_WEB_ERROR_STATUS_CERTIFICATE_EXPIRED:
    return "CertificateExpired";
    break;
  case COREWEBVIEW2_WEB_ERROR_STATUS_CLIENT_CERTIFICATE_CONTAINS_ERRORS:
    return "ClientCertificateContainsErrors";
    break;
  case COREWEBVIEW2_WEB_ERROR_STATUS_CERTIFICATE_REVOKED:
    return "CertificateRevoked";
    break;
  case COREWEBVIEW2_WEB_ERROR_STATUS_CERTIFICATE_IS_INVALID:
    return "CertificateIsInvalid";
    break;
  case COREWEBVIEW2_WEB_ERROR_STATUS_SERVER_UNREACHABLE:
    return "ServerUnreachable";
    break;
  case COREWEBVIEW2_WEB_ERROR_STATUS_TIMEOUT:
    return "Timeout";
    break;
  case COREWEBVIEW2_WEB_ERROR_STATUS_ERROR_HTTP_INVALID_SERVER_RESPONSE:
    return "ErrorHttpInvalidServerResponse";
    break;
  case COREWEBVIEW2_WEB_ERROR_STATUS_CONNECTION_ABORTED:
    return "ConnectionAborted";
    break;
  case COREWEBVIEW2_WEB_ERROR_STATUS_CONNECTION_RESET:
    return "ConnectionReset";
    break;
  case COREWEBVIEW2_WEB_ERROR_STATUS_DISCONNECTED:
    return "Disconnected";
    break;
  case COREWEBVIEW2_WEB_ERROR_STATUS_CANNOT_CONNECT:
    return "CannotConnect";
    break;
  case COREWEBVIEW2_WEB_ERROR_STATUS_HOST_NAME_NOT_RESOLVED:
    return "HostNameNotResolved";
    break;
  case COREWEBVIEW2_WEB_ERROR_STATUS_OPERATION_CANCELED:
    return "OperationCanceled";
    break;
  case COREWEBVIEW2_WEB_ERROR_STATUS_REDIRECT_FAILED:
    return "RedirectFailed";
    break;
  case COREWEBVIEW2_WEB_ERROR_STATUS_UNEXPECTED_ERROR:
    return "UnexpectedError";
    break;
  case COREWEBVIEW2_WEB_ERROR_STATUS_UNKNOWN:
  default:
    return "Unknown";
    break;
  }
}

bool KrunkerWindow::sendMessage(const JSMessage &message) {
  return SUCCEEDED(
      webview->PostWebMessageAsJson(ST::wstring(message.dump()).c_str()));
}

bool KrunkerWindow::postMessage(
    const JSMessage &message,
    std::function<void(const rapidjson::Value &)> then,
    std::function<void(const rapidjson::Value &)> catchError) {
  short id = 0xff; // reserved

  for (; id < 0xffff; id++)
    if (!postedMessages.contains(id))
      break;

  postedMessages[id] = {then, catchError};

  JSMessage send(message.event);

  send.args.PushBack(rapidjson::Value(id), send.allocator);

  for (rapidjson::Value::ConstValueIterator it = message.args.Begin();
       it != message.args.End(); ++it)
    send.args.PushBack(rapidjson::Value(*it, send.allocator), send.allocator);

  return SUCCEEDED(
      webview->PostWebMessageAsJson(ST::wstring(send.dump()).c_str()));
}

std::wstring encodeURIComponent(std::wstring decoded) {
  std::wostringstream oss;
  std::wregex r(L"[!'\\(\\)*-.0-9A-Za-z_~]");

  for (wchar_t &c : decoded) {
    std::wstring cw;
    cw += c;
    if (std::regex_match(cw, r))
      oss << c;
    else
      oss << "%" << std::uppercase << std::hex << (0xff & c);
  }

  return oss.str();
}

const char *getCtxString(COREWEBVIEW2_WEB_RESOURCE_CONTEXT ctx) {
  switch (ctx) {
  case COREWEBVIEW2_WEB_RESOURCE_CONTEXT::COREWEBVIEW2_WEB_RESOURCE_CONTEXT_ALL:
    return "all";
  case COREWEBVIEW2_WEB_RESOURCE_CONTEXT::
      COREWEBVIEW2_WEB_RESOURCE_CONTEXT_DOCUMENT:
    return "document";
  case COREWEBVIEW2_WEB_RESOURCE_CONTEXT::
      COREWEBVIEW2_WEB_RESOURCE_CONTEXT_STYLESHEET:
    return "stylesheet";
  case COREWEBVIEW2_WEB_RESOURCE_CONTEXT::
      COREWEBVIEW2_WEB_RESOURCE_CONTEXT_IMAGE:
    return "image";
  case COREWEBVIEW2_WEB_RESOURCE_CONTEXT::
      COREWEBVIEW2_WEB_RESOURCE_CONTEXT_MEDIA:
    return "media";
  case COREWEBVIEW2_WEB_RESOURCE_CONTEXT::
      COREWEBVIEW2_WEB_RESOURCE_CONTEXT_FONT:
    return "font";
  case COREWEBVIEW2_WEB_RESOURCE_CONTEXT::
      COREWEBVIEW2_WEB_RESOURCE_CONTEXT_SCRIPT:
    return "script";
  case COREWEBVIEW2_WEB_RESOURCE_CONTEXT::
      COREWEBVIEW2_WEB_RESOURCE_CONTEXT_XML_HTTP_REQUEST:
    return "xml-http-request";
  case COREWEBVIEW2_WEB_RESOURCE_CONTEXT::
      COREWEBVIEW2_WEB_RESOURCE_CONTEXT_FETCH:
    return "fetch";
  case COREWEBVIEW2_WEB_RESOURCE_CONTEXT::
      COREWEBVIEW2_WEB_RESOURCE_CONTEXT_TEXT_TRACK:
    return "text-track";
  case COREWEBVIEW2_WEB_RESOURCE_CONTEXT::
      COREWEBVIEW2_WEB_RESOURCE_CONTEXT_EVENT_SOURCE:
    return "event-source";
  case COREWEBVIEW2_WEB_RESOURCE_CONTEXT::
      COREWEBVIEW2_WEB_RESOURCE_CONTEXT_WEBSOCKET:
    return "websocket";
  case COREWEBVIEW2_WEB_RESOURCE_CONTEXT::
      COREWEBVIEW2_WEB_RESOURCE_CONTEXT_MANIFEST:
    return "manifest";
  case COREWEBVIEW2_WEB_RESOURCE_CONTEXT::
      COREWEBVIEW2_WEB_RESOURCE_CONTEXT_SIGNED_EXCHANGE:
    return "signed";
  case COREWEBVIEW2_WEB_RESOURCE_CONTEXT::
      COREWEBVIEW2_WEB_RESOURCE_CONTEXT_PING:
    return "ping";
  case COREWEBVIEW2_WEB_RESOURCE_CONTEXT::
      COREWEBVIEW2_WEB_RESOURCE_CONTEXT_CSP_VIOLATION_REPORT:
    return "csp-violation-report";
  case COREWEBVIEW2_WEB_RESOURCE_CONTEXT::
      COREWEBVIEW2_WEB_RESOURCE_CONTEXT_OTHER:
    return "other";
  default:
    return "unknown";
  }
}

void KrunkerWindow::register_events() {
  EventRegistrationToken token;

  webview->add_WebMessageReceived(
      Callback<ICoreWebView2WebMessageReceivedEventHandler>(
          [this](ICoreWebView2 *sender,
                 ICoreWebView2WebMessageReceivedEventArgs *args) {
            wil::unique_cotaskmem_string mpt;
            args->TryGetWebMessageAsString(&mpt);

            if (!mpt)
              return S_OK;

            handle_message(ST::string(mpt.get()));

            return S_OK;
          })
          .Get(),
      &token);

  webview->add_NavigationStarting(
      Callback<ICoreWebView2NavigationStartingEventHandler>(
          [this](ICoreWebView2 *sender,
                 ICoreWebView2NavigationStartingEventArgs *args) -> HRESULT {
            if (mouse_hooked)
              unhook_mouse();
            return S_OK;
          })
          .Get(),
      &token);

  webview->add_PermissionRequested(
      Callback<ICoreWebView2PermissionRequestedEventHandler>(
          [this](ICoreWebView2 *sender,
                 ICoreWebView2PermissionRequestedEventArgs *args) -> HRESULT {
            COREWEBVIEW2_PERMISSION_KIND kind;
            args->get_PermissionKind(&kind);

            switch (kind) {
            case COREWEBVIEW2_PERMISSION_KIND::
                COREWEBVIEW2_PERMISSION_KIND_MICROPHONE:
            case COREWEBVIEW2_PERMISSION_KIND::
                COREWEBVIEW2_PERMISSION_KIND_CLIPBOARD_READ:
              args->put_State(COREWEBVIEW2_PERMISSION_STATE::
                                  COREWEBVIEW2_PERMISSION_STATE_ALLOW);
              break;
            }

            return S_OK;
          })
          .Get(),
      &token);

  // look into:
  /*ICoreWebView2_2* a;
  ICoreWebView2_3* b;
  ICoreWebView2_4* c;
  ICoreWebView2_5* d;
  ICoreWebView2_6* e;
  */

  if (wil::com_ptr<ICoreWebView2_2> webview_2 =
          webview.query<ICoreWebView2_2>()) {
    webview_2->add_ContentLoading(
        Callback<ICoreWebView2ContentLoadingEventHandler>(
            [this](ICoreWebView2 *sender,
                   ICoreWebView2ContentLoadingEventArgs *args) -> HRESULT {
              wil::unique_cotaskmem_string urir;
              webview->get_Source(&urir);
              UriW uri(urir.get());

              if (uri.host() == L"krunker.io") {
                std::wstring setup = L"window._RUNTIME_DATA_ = " +
                                     ST::wstring(runtime_data()) + L";";

                webview->ExecuteScript(setup.c_str(), nullptr);

                webview->ExecuteScript(js_frontend.c_str(), nullptr);
              }

              return S_OK;
            })
            .Get(),
        &token);
  }

  webview->add_NavigationCompleted(
      Callback<ICoreWebView2NavigationCompletedEventHandler>(
          [this](ICoreWebView2 *sender,
                 ICoreWebView2NavigationCompletedEventArgs *args) -> HRESULT {
            BOOL success = true;
            args->get_IsSuccess(&success);

            wil::unique_cotaskmem_string urir;
            webview->get_Source(&urir);
            std::string uri(ST::string(urir.get()));

            if (!success) {
              COREWEBVIEW2_WEB_ERROR_STATUS status;
              args->get_WebErrorStatus(&status);

              if (status ==
                      COREWEBVIEW2_WEB_ERROR_STATUS::
                          COREWEBVIEW2_WEB_ERROR_STATUS_CONNECTION_ABORTED ||
                  status ==
                      COREWEBVIEW2_WEB_ERROR_STATUS::
                          COREWEBVIEW2_WEB_ERROR_STATUS_OPERATION_CANCELED) {
                return S_OK;
              }

              // renderer freezes when navigating from an error page that
              // occurs on startup
              control->Close();
              call_create_webview([this, status, uri]() {
                // = {status}
                rapidjson::Document data(rapidjson::kArrayType);
                rapidjson::Document::AllocatorType allocator =
                    data.GetAllocator();
                std::string name = status_name(status);

                data.PushBack(rapidjson::Value(status), allocator);
                data.PushBack(rapidjson::Value(name.data(), name.size()),
                              allocator);
                data.PushBack(rapidjson::Value(uri.data(), uri.size()),
                              allocator);

                rapidjson::StringBuffer buffer;
                rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                data.Accept(writer);

                webview->Navigate((L"https://chief/error?data=" +
                                   encodeURIComponent(ST::wstring(
                                       {buffer.GetString(), buffer.GetSize()})))
                                      .c_str());
              });
            }

            return S_OK;
          })
          .Get(),
      &token);

  webview->AddWebResourceRequestedFilter(L"*",
                                         COREWEBVIEW2_WEB_RESOURCE_CONTEXT_ALL);

  webview->add_WebResourceRequested(
      Callback<ICoreWebView2WebResourceRequestedEventHandler>(
          [this](ICoreWebView2 *sender,
                 ICoreWebView2WebResourceRequestedEventArgs *args) -> HRESULT {
            wil::com_ptr<ICoreWebView2WebResourceRequest> request;
            args->get_Request(&request);
            wil::unique_cotaskmem_string urir;
            request->get_Uri(&urir);
            UriW uri(urir.get());

            if (uri.path() == L"/favicon.ico") {
              wil::com_ptr<ICoreWebView2WebResourceResponse> response;
              env->CreateWebResourceResponse(nullptr, 403, L"Unused", L"",
                                             &response);
              args->put_Response(response.get());
            } else if (uri.host() == L"chief") {
              if (uri.path() == L"/error")
                send_resource(args, HTML_ERROR, L"text/html");
              else if (uri.path() == L"/GameFont.ttf")
                send_resource(args, FONT_GAME, L"font/ttf");
              else if (uri.path() == L"/frontend.js.map")
                send_resource(args, JS_FRONTEND_MAP, L"application/json");
            } else if (krunker::host_is_krunker(uri.host())) {
              std::wstring swap =
                  folder.directory + folder.p_swapper + uri.path();

              if (IOUtil::file_exists(swap)) {
                clog::info << "Swapping " << ST::string(uri.path())
                           << clog::endl;
                // Create an empty IStream:
                IStream *stream;

                if (SHCreateStreamOnFileEx(swap.c_str(),
                                           STGM_READ | STGM_SHARE_DENY_WRITE, 0,
                                           false, 0, &stream) == S_OK) {
                  wil::com_ptr<ICoreWebView2WebResourceResponse> response;
                  env->CreateWebResourceResponse(
                      stream, 200, L"OK",
                      L"access-control-allow-origin: "
                      L"https://krunker.io\naccess-control-expose-headers: "
                      L"Content-Length, Content-Type, Date, Server, "
                      L"Transfer-Encoding, X-GUploader-UploadID, "
                      L"X-Google-Trace",
                      &response);
                  args->put_Response(response.get());
                } else
                  clog::error << "Unable to create IStream for file: "
                              << ST::string(swap) << clog::endl;
              }
            }

            return S_OK;
          })
          .Get(),
      &token);
}

KrunkerWindow::Status KrunkerWindow::create(HINSTANCE inst, int cmdshow,
                                            std::function<void()> callback) {
  bool was_open = open;

  if (folder.config["window"]["meta"]["replace"].GetBool())
    title = JT::wstring(folder.config["window"]["meta"]["title"]);

  create_window(inst, cmdshow);

  SetClassLongPtr(m_hWnd, GCLP_HBRBACKGROUND,
                  (LONG_PTR)CreateSolidBrush(background));

  if (can_fullscreen && folder.config["render"]["fullscreen"].GetBool())
    enter_fullscreen();

  if (folder.config["window"]["meta"]["replace"].GetBool())
    SetIcon((HICON)LoadImage(
        inst,
        folder
            .resolve_path(JT::wstring(folder.config["window"]["meta"]["icon"]))
            .c_str(),
        IMAGE_ICON, 32, 32, LR_LOADFROMFILE));
  else
    SetIcon(LoadIcon(inst, MAKEINTRESOURCE(MAINICON)));

  open = true;

  return call_create_webview(callback);
  /*else {
          callback();
          return Status::Ok;
  }*/
}

KrunkerWindow::Status
KrunkerWindow::call_create_webview(std::function<void()> callback) {
  auto options = Make<CoreWebView2EnvironmentOptions>();

  options->put_AdditionalBrowserArguments(cmdline().c_str());

  HRESULT create = CreateCoreWebView2EnvironmentWithOptions(
      nullptr, (folder.directory + folder.p_profile).c_str(), options.Get(),
      Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
          [this, callback](HRESULT result,
                           ICoreWebView2Environment *envp) -> HRESULT {
            if (envp == nullptr) {
              clog::error << "Env was nullptr" << clog::endl;

              return S_FALSE;
            }

            env = envp;
            env->CreateCoreWebView2Controller(
                m_hWnd,
                Callback<
                    ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                    [this,
                     callback](HRESULT result,
                               ICoreWebView2Controller *controller) -> HRESULT {
                      if (controller == nullptr) {
                        clog::error << "Controller was nullptr. Error code: 0x"
                                    << std::hex << result << clog::endl;
                        // call_create_webview(callback);
                        return S_FALSE;
                      }

                      control = controller;
                      control->get_CoreWebView2(&webview);

                      if (wil::com_ptr<ICoreWebView2Controller2> control2 =
                              control.query<ICoreWebView2Controller2>()) {
                        control2->put_DefaultBackgroundColor(
                            ColorRef(background));
                      }

                      if (wil::com_ptr<ICoreWebView2Controller3> control3 =
                              control.query<ICoreWebView2Controller3>()) {
                        control3->put_ShouldDetectMonitorScaleChanges(false);
                      }

                      wil::com_ptr<ICoreWebView2Settings> settings;
                      if (SUCCEEDED(webview->get_Settings(&settings))) {
                        settings->put_AreDefaultContextMenusEnabled(false);
                        settings->put_IsScriptEnabled(true);
                        settings->put_AreDefaultScriptDialogsEnabled(true);
                        settings->put_IsWebMessageEnabled(true);
                        settings->put_IsZoomControlEnabled(false);
                        settings->put_AreDefaultContextMenusEnabled(false);
                        settings->put_IsStatusBarEnabled(false);
#if _DEBUG != 1
                        settings->put_AreDevToolsEnabled(false);
#else
                        webview->OpenDevToolsWindow();
#endif

                        if (wil::com_ptr<ICoreWebView2Settings3> settings3 =
                                settings.query<ICoreWebView2Settings3>()) {
                          settings3->put_AreBrowserAcceleratorKeysEnabled(
                              false);
                        }
                        if (wil::com_ptr<ICoreWebView2Settings4> settings4 =
                                settings.query<ICoreWebView2Settings4>()) {
                          settings4->put_IsGeneralAutofillEnabled(false);
                          settings4->put_IsPasswordAutosaveEnabled(false);
                        }
                      }

                      resize_wv();
                      register_events();

                      clog::debug << "KrunkerWindow created" << clog::endl;

                      if (on_webview2_startup)
                        on_webview2_startup();
                      if (callback)
                        callback();

                      return S_OK;
                    })
                    .Get());

            return S_OK;
          })
          .Get());

  if (!SUCCEEDED(create)) {
    last_herror = create;

    switch (create) {
    case HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND):
      return Status::MissingRuntime;
      break;
    case HRESULT_FROM_WIN32(ERROR_FILE_EXISTS):
      return Status::UserDataExists;
      break;
    case E_ACCESSDENIED:
      return Status::FailCreateUserData;
      break;
    case E_FAIL:
      return Status::RuntimeFatal;
      break;
    default:
      return Status::UnknownError;
      break;
    }
  } else
    return Status::Ok;
}

KrunkerWindow::Status KrunkerWindow::get(HINSTANCE inst, int cmdshow,
                                         std::function<void(bool)> callback) {
  if (open) {
    // documentation for editor opens twice
    if (!webview) {
      return Status::RuntimeFatal;
    }

    callback(false);
    return Status::AlreadyOpen;
  } else {
    return create(inst, cmdshow, [this, callback]() { callback(true); });
  }
}

long long mouse_hz = 244;
long long mouse_interval = 1000 / mouse_hz;
long long then = KrunkerWindow::now();

void KrunkerWindow::on_dispatch() {
  if (!open)
    return;

  mtx.lock();

  for (JSMessage msg : pending_messages)
    if (!sendMessage(msg))
      clog::error << "Unable to send " << msg.dump() << clog::endl;

  pending_messages.clear();

  for (std::wstring url : pending_navigations) {
    webview->Stop();
    webview->Navigate(url.c_str());
  }

  pending_navigations.clear();

  mtx.unlock();

  bool active = GetActiveWindow() == m_hWnd;

  if (!active && mouse_hooked)
    unhook_mouse();

  time_t last_poll = now() - last_pointer_poll;

  if (last_poll > 1500 && mouse_hooked) {
    clog::error << "Pointer lock timeout: " << last_poll << "ms" << clog::endl;
    unhook_mouse();
  }

  if (mouse_hooked) {
    long long nw = now();
    long long delta = nw - then;

    if (delta > mouse_interval) {
      then = nw - (delta % mouse_interval);
      sendMessage(msgFct(IM::mousemove, {movebuffer.x, movebuffer.y}));
      movebuffer.clear();
    }
  }
}

LRESULT KrunkerWindow::on_destroy(UINT uMsg, WPARAM wParam, LPARAM lParam,
                                  BOOL &fHandled) {
  open = false;
  if (on_destroy_callback)
    on_destroy_callback();
  return true;
}