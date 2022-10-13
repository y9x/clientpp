#include "./KrunkerWindow.h"
#include "../utils/Base64.h"
#include "../utils/JsonUtil.h"
#include "../utils/StringUtil.h"
#include "../utils/Uri.h"
#include "./LoadRes.h"
#include "./Log.h"
#include "./Site.h"
#include "./main.h"
#include "./resource.h"
#include <WebView2EnvironmentOptions.h>
#include <chrono>
#include <rapidjson/writer.h>
#include <regex>
#include <sstream>

constexpr const wchar_t *addScripts =
    LR"(document.write('<link rel=\'stylesheet\' href=\'https://chief.krunker.io/main.css\'></script><script src=\'https://chief.krunker.io/main.js\'></script>');)";

long long now() {
  return duration_cast<std::chrono::milliseconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}

ChWindow::ChWindow(ClientFolder &_folder, ChWindows &_windows, Vector2 _scale,
                   std::wstring _title)
    : folder(_folder), windows(_windows), title(_title), og_title(_title),
      scale(_scale){};

ChScriptedWindow::ChScriptedWindow(ClientFolder &_folder,
                                   AccountManager &_accounts,
                                   ChWindows &_windows, Vector2 _scale,
                                   std::wstring _title)
    : ChWindow(_folder, _windows, _scale, _title), accounts(_accounts),
      last_pointer_poll(now()) {
  // convert to wstring immediately
  std::string sMainJS;

  if (loadResource(JS_MAIN, sMainJS))
    mainJS = ST::wstring(sMainJS);
  else
    clog::error << "Failure loading main.js" << clog::endl;

  raw_input.usUsagePage = 0x01;
  raw_input.usUsage = 0x02;
}

ChWindow::~ChWindow() {
  if (::IsWindow(m_hWnd))
    DestroyWindow();
}

ChScriptedWindow::~ChScriptedWindow() {
  if (mouse_hooked)
    unhook_mouse();
}

HINSTANCE ChWindow::getHinstance() {
  return (HINSTANCE)GetWindowLong(GWL_HINSTANCE);
}

bool ChWindow::createWindow() {
  Create(NULL, NULL, title.c_str(), WS_OVERLAPPEDWINDOW);

  Vector2 scr_pos;
  Vector2 scr_size;

  if (monitorData(scr_pos, scr_size)) {
    Rect2D r;

    r.width = long(scr_size.x * scale.x);
    r.height = long(scr_size.y * scale.y);

    r.x = long(scr_pos.x + ((scr_size.x - r.width) / 2));
    r.y = long(scr_pos.y + ((scr_size.y - r.height) / 2));

    SetWindowPos(NULL, r.get(), 0);
  } else
    ResizeClient(700, 500);

  ShowWindow(SW_SHOW);
  UpdateWindow();

  return true;
}

COREWEBVIEW2_COLOR ColorRef(COLORREF color) {
  return COREWEBVIEW2_COLOR{255, GetRValue(color), GetGValue(color),
                            GetBValue(color)};
}

// generic background
constexpr const COLORREF background = RGB(28, 28, 28);

bool ChScriptedWindow::enterFullscreen() {
  if (fullscreen)
    return false;

  RECT screen;

  if (!monitorData(screen))
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

  resizeWV();

  fullscreen = true;

  return true;
}

bool ChScriptedWindow::exitFullscreen() {
  if (!fullscreen)
    return false;

  SetWindowLong(GWL_STYLE, saved_style);
  SetWindowLong(GWL_EXSTYLE, saved_ex_style);
  SetWindowPos(NULL, RECT_ARGS(saved_size),
               SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
  resizeWV();

  fullscreen = false;
  return true;
}

bool ChWindow::resizeWV() {
  if (control == nullptr)
    return false;

  RECT bounds;
  GetClientRect(&bounds);
  control->put_Bounds(bounds);

  return true;
}

bool ChWindow::monitorData(RECT &rect) {
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

bool ChWindow::monitorData(Vector2 &pos, Vector2 &size) {
  RECT r;

  if (!monitorData(r))
    return false;

  pos.x = r.left;
  pos.y = r.top;

  size.x = r.right - r.left;
  size.y = r.bottom - r.top;

  return true;
}

LRESULT ChWindow::on_resize(UINT uMsg, WPARAM wParam, LPARAM lParam,
                            BOOL &fHandled) {
  return resizeWV();
}

Vector2 movebuffer;

JSMessage msgFct(unsigned short event, std::vector<double> nums) {
  JSMessage msg(event);
  for (double num : nums)
    msg.args.PushBack(rapidjson::Value(num), msg.allocator);
  return msg;
}

LRESULT ChScriptedWindow::on_input(UINT uMsg, WPARAM wParam, LPARAM lParam,
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

LRESULT CALLBACK ChScriptedWindow::mouseMessage(int code, WPARAM wParam,
                                                LPARAM lParam) {
  return 1;
}

void ChScriptedWindow::hook_mouse() {
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
      SetWindowsHookEx(WH_MOUSE_LL, *mouseMessage, getHinstance(), NULL);
  mouse_hooked = true;
}

void ChScriptedWindow::unhook_mouse() {
  clog::debug << "Unhooked mouse" << clog::endl;

  raw_input.dwFlags = RIDEV_REMOVE;
  raw_input.hwndTarget = NULL;
  RegisterRawInputDevices(&raw_input, 1, sizeof(raw_input));

  UnhookWindowsHookEx(mouse_hook);
  mouse_hooked = false;
}

// https://peter.sh/experiments/chromium-command-line-switches/
std::wstring ChWindow::cmdLine() {
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

bool ChScriptedWindow::sendResource(
    ICoreWebView2WebResourceRequestedEventArgs *args, int resource,
    std::wstring mime) {
  IStream *stream = nullptr;
  std::string data;

  if (!loadResource(resource, data)) {
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

bool ChScriptedWindow::sendMessage(const JSMessage &message) {
  return SUCCEEDED(
      webview->PostWebMessageAsJson(ST::wstring(message.dump()).c_str()));
}

bool ChScriptedWindow::postMessage(
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

void ChWindow::dispatch() {
  if (!open)
    return;

  dispatchMtx.lock();

  for (std::wstring url : pendingNavigations) {
    webview->Stop();
    webview->Navigate(url.c_str());
  }

  pendingNavigations.clear();

  dispatchMtx.unlock();
}

void ChScriptedWindow::dispatch() {
  ChWindow::dispatch();

  if (!open)
    return;

  dispatchMtx.lock();

  for (JSMessage msg : pendingMessages)
    if (!sendMessage(msg))
      clog::error << "Unable to send " << msg.dump() << clog::endl;

  pendingMessages.clear();

  dispatchMtx.unlock();

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

void ChScriptedWindow::registerEvents() {
  ChWindow::registerEvents();

  EventRegistrationToken token;

  webview->add_WebMessageReceived(
      Microsoft::WRL::Callback<ICoreWebView2WebMessageReceivedEventHandler>(
          [this](ICoreWebView2 *sender,
                 ICoreWebView2WebMessageReceivedEventArgs *args) {
            wil::unique_cotaskmem_string mpt;
            args->TryGetWebMessageAsString(&mpt);

            if (!mpt)
              return S_OK;

            handleMessage(ST::string(mpt.get()));

            return S_OK;
          })
          .Get(),
      &token);

  webview->add_NavigationStarting(
      Microsoft::WRL::Callback<ICoreWebView2NavigationStartingEventHandler>(
          [this](ICoreWebView2 *sender,
                 ICoreWebView2NavigationStartingEventArgs *args) -> HRESULT {
            if (mouse_hooked)
              unhook_mouse();
            return S_OK;
          })
          .Get(),
      &token);

  if (wil::com_ptr<ICoreWebView2_2> webview_2 =
          webview.query<ICoreWebView2_2>()) {
    webview_2->add_ContentLoading(
        Microsoft::WRL::Callback<ICoreWebView2ContentLoadingEventHandler>(
            [this](ICoreWebView2 *sender,
                   ICoreWebView2ContentLoadingEventArgs *args) -> HRESULT {
              wil::unique_cotaskmem_string urir;
              webview->get_Source(&urir);
              UriW uri(urir.get());

              if (uri.host() == L"krunker.io")
                webview->ExecuteScript(
                    (mainJS + L"\nfunction getRuntimeData() { return " +
                     ST::wstring(runtimeData()) +
                     L"; }; delete window.getRuntimeData;")
                        .c_str(),
                    nullptr);

              return S_OK;
            })
            .Get(),
        &token);
  }

  webview->add_WebResourceRequested(
      Microsoft::WRL::Callback<ICoreWebView2WebResourceRequestedEventHandler>(
          [this](ICoreWebView2 *sender,
                 ICoreWebView2WebResourceRequestedEventArgs *args) -> HRESULT {
            wil::com_ptr<ICoreWebView2WebResourceRequest> request;
            args->get_Request(&request);
            wil::unique_cotaskmem_string urir;
            request->get_Uri(&urir);
            UriW uri(urir.get());

            if (uri.host() == L"chief.krunker.io") {
              if (uri.path() == L"/error.html")
                sendResource(args, HTML_ERROR, L"text/html");
              else if (uri.path() == L"/GameFont.ttf")
                sendResource(args, FONT_GAME, L"font/ttf");
            }

            return S_OK;
          })
          .Get(),
      &token);
}

Status ChWindow::show(UriW uri, ICoreWebView2 *sender,
                      std::function<void()> open, bool &shown) {
  if (webview && webview == sender) {
    if (&shown)
      shown = false;
    return Status::Ok;
  }

  if (&shown)
    shown = true;

  return get([this, uri, open](bool newly_created) {
    webview->Navigate(uri.toString().c_str());
    BringWindowToTop();
    if (open)
      open();
  });
}

void ChWindow::registerEvents() {
  EventRegistrationToken token;

  webview->add_NewWindowRequested(
      Microsoft::WRL::Callback<ICoreWebView2NewWindowRequestedEventHandler>(
          [this](ICoreWebView2 *sender,
                 ICoreWebView2NewWindowRequestedEventArgs *args) -> HRESULT {
            wil::unique_cotaskmem_string uri;
            args->get_Uri(&uri);

            ICoreWebView2 *newWindow = nullptr;

            wil::com_ptr<ICoreWebView2Deferral> defer;

            args->AddRef();
            if (SUCCEEDED(args->GetDeferral(&defer))) {
              bool shown = false;

              windows.navigate(
                  UriW(uri.get()), sender,
                  [defer, args](ChWindow *newWindow) -> void {
                    if (newWindow)
                      args->put_NewWindow(newWindow->webview.get());
                    args->put_Handled(true);
                    defer->Complete();
                  },
                  shown);

              if (!shown)
                defer->Complete();
            }

            return S_OK;
          })
          .Get(),
      &token);

  webview->add_NavigationStarting(
      Microsoft::WRL::Callback<ICoreWebView2NavigationStartingEventHandler>(
          [this](ICoreWebView2 *sender,
                 ICoreWebView2NavigationStartingEventArgs *args) -> HRESULT {
            wil::unique_cotaskmem_string uri;
            args->get_Uri(&uri);
            bool shown = false;
            windows.navigate(UriW(uri.get()), sender, nullptr, shown);
            if (shown)
              args->put_Cancel(true);

            return S_OK;
          })
          .Get(),
      &token);

  webview->add_PermissionRequested(
      Microsoft::WRL::Callback<ICoreWebView2PermissionRequestedEventHandler>(
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

  webview->add_NavigationCompleted(
      Microsoft::WRL::Callback<ICoreWebView2NavigationCompletedEventHandler>(
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
              callCreateWebView([this, status, uri]() {
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

                webview->Navigate(
                    (L"https://chief.krunker.io/error.html?data=" +
                     encodeURIComponent(
                         ST::wstring({buffer.GetString(), buffer.GetSize()})))
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
      Microsoft::WRL::Callback<ICoreWebView2WebResourceRequestedEventHandler>(
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
            } else if (uri.host() == L"krunker.io") {
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

Status ChScriptedWindow::create(std::function<void()> callback) {
  Status status = ChWindow::create(callback);

  if (status == Status::Ok && folder.config["render"]["fullscreen"].GetBool())
    enterFullscreen();

  return status;
}

Status ChWindow::create(std::function<void()> callback) {
  bool was_open = open;

  if (folder.config["window"]["meta"]["replace"].GetBool())
    title = JT::wstring(folder.config["window"]["meta"]["title"]);

  createWindow();

  SetClassLongPtr(m_hWnd, GCLP_HBRBACKGROUND,
                  (LONG_PTR)CreateSolidBrush(background));

  // hInstance is optional !
  if (folder.config["window"]["meta"]["replace"].GetBool())
    SetIcon((HICON)LoadImage(
        NULL,
        folder
            .resolve_path(JT::wstring(folder.config["window"]["meta"]["icon"]))
            .c_str(),
        IMAGE_ICON, 32, 32, LR_LOADFROMFILE));
  else
    SetIcon(getMainIcon());

  open = true;

  return callCreateWebView(callback);
  /*else {
          callback();
          return Status::Ok;
  }*/
}

Status ChWindow::callCreateWebView(std::function<void()> callback) {
  auto options = Microsoft::WRL::Make<CoreWebView2EnvironmentOptions>();

  options->put_AdditionalBrowserArguments(cmdLine().c_str());

  HRESULT create = CreateCoreWebView2EnvironmentWithOptions(
      nullptr, (folder.directory + folder.p_profile).c_str(), options.Get(),
      Microsoft::WRL::Callback<
          ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
          [this, callback](HRESULT result,
                           ICoreWebView2Environment *envp) -> HRESULT {
            if (envp == nullptr) {
              clog::error << "Env was nullptr" << clog::endl;

              return S_FALSE;
            }

            env = envp;
            env->CreateCoreWebView2Controller(
                m_hWnd,
                Microsoft::WRL::Callback<
                    ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                    [this,
                     callback](HRESULT result,
                               ICoreWebView2Controller *controller) -> HRESULT {
                      if (controller == nullptr) {
                        clog::error << "Controller was nullptr. Error code: 0x"
                                    << std::hex << result << clog::endl;
                        // callCreateWebView(callback);
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

                        // Insert io.krunker.steam useragent
                        // Will break navigator.userAgentData
                        if (wil::com_ptr<ICoreWebView2Settings2> settings2 =
                                settings.query<ICoreWebView2Settings2>()) {
                          wil::unique_cotaskmem_string _userAgent;
                          settings2->get_UserAgent(&_userAgent);

                          std::wstring userAgent = _userAgent.get();
                          std::wstring target = L"(KHTML, like Gecko) Chrome/";

                          settings2->put_UserAgent(
                              userAgent
                                  .replace(
                                      userAgent.find(target), target.length(),
                                      L"(KHTML, like Gecko) "
                                      L"io.krunker.steam/" +
                                          ST::wstring(CLIENT_VERSION_STRING) +
                                          L" Chrome/")
                                  .c_str());
                        }

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

                      resizeWV();
                      registerEvents();

                      clog::debug << "KrunkerWindow created" << clog::endl;

                      /*if (on_webview2_startup) on_webview2_startup();*/

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

Status ChWindow::get(std::function<void(bool)> callback) {
  if (open) {
    if (!webview)
      return Status::RuntimeFatal;

    callback(false);
    return Status::AlreadyOpen;
  } else {
    return create([this, callback]() { callback(true); });
  }
}

LRESULT ChWindow::on_destroy(UINT uMsg, WPARAM wParam, LPARAM lParam,
                             BOOL &fHandled) {
  open = false;
  // if (on_destroy_callback) on_destroy_callback();
  return true;
}
