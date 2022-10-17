#include "./ChScriptedWindow.h"
#include "../utils/StringUtil.h"
#include "./ChWindow.h"
#include "./JSMessage.h"
#include "./LoadRes.h"
#include "./Log.h"
#include "./resource.h"

ChScriptedWindow::ChScriptedWindow(ClientFolder &_folder, ChWindows &_windows,
                                   Vector2 _scale, std::wstring _title)
    : ChWindow(_folder, _windows, _scale, _title) {
  // convert to wstring immediately
  std::string mGenericJS;

  if (loadResource(JS_GENERIC, mGenericJS))
    genericJS = ST::wstring(mGenericJS);
  else
    clog::error << "Failure loading generic.js" << clog::endl;

  std::string mTampermonkeyJS;

  if (loadResource(JS_TAMPERMONKEY, mTampermonkeyJS))
    tampermonkeyJS = ST::wstring(mTampermonkeyJS);
  else
    clog::error << "Failure loading tampermonkey.js" << clog::endl;
}

ChWindow::Status ChScriptedWindow::create(std::function<void()> callback) {
  Status status = ChWindow::create(callback);

  if (status == Status::Ok && folder.config["render"]["fullscreen"].GetBool())
    enterFullscreen();

  return status;
}

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
}

void ChScriptedWindow::injectRuntimeScript(const std::wstring &script) {
  webview->ExecuteScript((script + L"\nfunction getRuntimeData() { return " +
                          ST::wstring(runtimeData()) +
                          L"; }; delete window.getRuntimeData;")
                             .c_str(),
                         nullptr);
}

void ChScriptedWindow::injectJS() {
  injectRuntimeScript(tampermonkeyJS);
  injectRuntimeScript(genericJS);
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
                injectJS();

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
