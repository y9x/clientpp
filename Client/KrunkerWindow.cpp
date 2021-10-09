#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "./httplib.h"
#include "./KrunkerWindow.h"
#include "../Utils/StringUtil.h"
#include "./LoadRes.h"
#include "./resource.h"
#include "../Utils/Uri.h"
#include "../Utils/Base64.h"
#include "./Log.h"

using namespace StringUtil;
using Microsoft::WRL::Make;
using Microsoft::WRL::Callback;

KrunkerWindow::KrunkerWindow(ClientFolder* f, Vector2 scale, std::wstring title, std::wstring p) : WebView2Window(scale, title), folder(f), og_title(title), pathname(p) {}

JSON KrunkerWindow::runtime_data() {
	JSON data = JSON::object();

	struct Search {
		std::wstring dir;
		std::wstring filter;
		JSON& obj;
	};

	for (Search search : std::vector<Search>{
		{folder->p_styles, L"*.css", data["css"] = JSON::object()},
		{folder->p_scripts, L"*.js", data["js"] = JSON::object()},
		})
		for (IOUtil::WDirectoryIterator it(folder->directory + search.dir, search.filter); ++it;) {
			std::string buffer;

			if (IOUtil::read_file(it.path().c_str(), buffer))
				search.obj[Convert::string(it.file()).c_str()] = buffer;
		}

	std::string css_client;
	if (load_resource(CSS_CLIENT, css_client)) data["css"]["Client/Client.css"] = css_client;

	data["config"] = folder->config;

	return data;
}

// https://peter.sh/experiments/chromium-command-line-switches/
std::wstring KrunkerWindow::cmdline() {
	std::vector<std::wstring> cmds = {
		L"--disable-features=msSmartScreenProtection",
		L"--force-dark-mode",
		L"--autoplay-policy=no-user-gesture-required",
		// L"--profile-directory=Profile",
		// L"disable-background-timer-throttling"
	};

	if (folder->config["client"]["uncap_fps"].get<bool>()) {
		cmds.push_back(L"--disable-frame-rate-limit");
		cmds.push_back(L"--disable-gpu-vsync");
	}

	std::wstring cmdline;
	bool first = false;

	for (std::wstring cmd : cmds) {
		if (first) first = false;
		else cmdline += L" ";

		cmdline += cmd;
	}

	return cmdline;
}

void KrunkerWindow::register_events() {
	EventRegistrationToken token;

	std::string bootstrap;
	if (load_resource(JS_BOOTSTRAP, bootstrap)) webview->AddScriptToExecuteOnDocumentCreated(Convert::wstring(bootstrap).c_str(), nullptr);
	else LOG_ERROR("Error loading bootstrapper");

	webview->AddWebResourceRequestedFilter(L"*", COREWEBVIEW2_WEB_RESOURCE_CONTEXT_ALL);
	webview->add_WebMessageReceived(Callback<ICoreWebView2WebMessageReceivedEventHandler>([this](ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args) {
		LPWSTR mpt;
		args->TryGetWebMessageAsString(&mpt);

		if (mpt) {
			JSON message = JSON::parse(Convert::string(mpt));
			std::string event = message[0].get<std::string>();

			if (event == "send webpack") {
				JSON response = JSON::array();

				response[0] = "eval webpack";

				std::string js_webpack = "throw Error('Failure loading Webpack.js');";
				load_resource(JS_WEBPACK, js_webpack);
				std::string js_webpack_map;
				load_resource(JS_WEBPACK_MAP, js_webpack_map);

				js_webpack += "\n//# sourceMappingURL=data:application/json;base64," + Base64::Encode(js_webpack_map);

				response[1] = js_webpack;
				response[2] = runtime_data();

				sender->PostWebMessageAsJson(Convert::wstring(response.dump()).c_str());
			}
			else if (event == "save config") {
				folder->config = message[1];
				folder->save_config();
			}
			else if (event == "open") {
				std::wstring open;

				if (message[1] == "root")open = folder->directory;
				else if (message[1] == "scripts") open = folder->directory + folder->p_scripts;
				else if (message[1] == "styles") open = folder->directory + folder->p_styles;
				else if (message[1] == "swapper") open = folder->directory + folder->p_swapper;
				else if (message[1] == "url") open = Convert::wstring(message[2].get<std::string>());

				ShellExecute(m_hWnd, L"open", open.c_str(), L"", L"", SW_SHOW);
			}
			else if (event == "relaunch") {
				// https://docs.microsoft.com/en-us/microsoft-edge/webview2/reference/win32/icorewebview2controller?view=webview2-1.0.992.28#close
				control->Close();

				call_create_webview([]() {});
			}
			else if (event == "close window") {
				if (::IsWindow(m_hWnd)) DestroyWindow();
			}
			else if (event == "fullscreen") {
				if (folder->config["client"]["fullscreen"].get<bool>()) enter_fullscreen();
				else exit_fullscreen();
			}
			else if (event == "update meta") {
				title = Convert::wstring(folder->config["window"]["meta"]["title"].get<std::string>());
				SetIcon((HICON)LoadImage(get_hinstance(), Convert::wstring(folder->config["window"]["meta"]["icon"].get<std::string>()).c_str(), IMAGE_ICON, 32, 32, LR_LOADFROMFILE));
				SetWindowText(title.c_str());
			}
			else if (event == "revert meta") {
				title = og_title;
				SetIcon(LoadIcon(get_hinstance(), MAKEINTRESOURCE(MAINICON)));
				SetWindowText(title.c_str());
			}
			else if (event == "reload config") {
				folder->load_config();
			}
			else if (event == "browse file") new std::thread([this](JSON message) {
				wchar_t filename[MAX_PATH];

				OPENFILENAME ofn;
				ZeroMemory(&filename, sizeof(filename));
				ZeroMemory(&ofn, sizeof(ofn));
				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = m_hWnd;
				std::wstring title = Convert::wstring(message[2]);
				std::wstring filters;
				for (JSON value : message[3]) {
					std::string label = value[0];
					std::string filter = value[1];

					label += " (" + filter + ")";

					filters += Convert::wstring(label);
					filters += L'\0';
					filters += Convert::wstring(filter);
					filters += L'\0';
				}

				// L"Icon Files\0*.ico\0Any File\0*.*\0"
				// filters is terminated by 2 null characters
				// each filter is terminated by 1 null character
				ofn.lpstrFilter = filters.c_str();
				ofn.lpstrFile = filename;
				ofn.nMaxFile = MAX_PATH;
				ofn.lpstrTitle = title.c_str();
				ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;

				JSON response = JSON::array();
				response[0] = message[1];

				if (GetOpenFileName(&ofn)) {
					response[1] = Convert::string(filename);
					response[2] = false;
				}
				else {
					response[2] = true;
				}

				mtx.lock();
				post.push_back(response);
				mtx.unlock();
			}, message);
		}
		else LOG_ERROR("Recieved invalid message");

		return S_OK;
	}).Get(), &token);

	webview->add_WebResourceRequested(Callback<ICoreWebView2WebResourceRequestedEventHandler>([this](ICoreWebView2* sender, ICoreWebView2WebResourceRequestedEventArgs* args) -> HRESULT {
		LPWSTR sender_uriptr;
		sender->get_Source(&sender_uriptr);

		ICoreWebView2WebResourceRequest* request = 0;
		args->get_Request(&request);
		LPWSTR uriptr;
		request->get_Uri(&uriptr);
		Uri uri(uriptr);
		request->Release();

		if (uri.host_owns(L"krunker.io")) {
			if (uri.pathname().starts_with(L"/pkg/")) {
				IStream* stream = nullptr;
				// request resource manually because webview2 fails to cache
				if (CreateStreamOnHGlobal(NULL, TRUE, (LPSTREAM*)&stream) == S_OK) {
					httplib::Client cli(Convert::string(uri.origin()));
					auto res = cli.Get(Convert::string(uri.pathname()).c_str());

					ULONG written = 0;

					if (stream->Write(res->body.data(), res->body.length(), &written) == S_OK) {
						wil::com_ptr<ICoreWebView2WebResourceResponse> response;
						env->CreateWebResourceResponse(stream, 200, L"OK", L"", &response);
						args->put_Response(response.get());
					}
					else LOG_ERROR("Error writing to IStream");

				}
				else LOG_ERROR("Error creating IStream on HGlobal");
			}
			else {
				std::wstring swap = folder->directory + folder->p_swapper + uri.pathname();

				if (IOUtil::file_exists(swap)) {
					LOG_INFO("Swapping " << Convert::string(uri.pathname()));
					// Create an empty IStream:
					IStream* stream;

					if (SHCreateStreamOnFileEx(swap.c_str(), STGM_READ | STGM_SHARE_DENY_WRITE, 0, false, 0, &stream) == S_OK) {
						wil::com_ptr<ICoreWebView2WebResourceResponse> response;
						env->CreateWebResourceResponse(stream, 200, L"OK", L"access-control-allow-origin: https://krunker.io\naccess-control-expose-headers: Content-Length, Content-Type, Date, Server, Transfer-Encoding, X-GUploader-UploadID, X-Google-Trace", &response);
						args->put_Response(response.get());
					}
					else LOG_ERROR("Error creating IStream on swap: " << Convert::string(swap));
				}
			}
		}
		else for (std::wstring test : blocked_hosts) if (uri.host_owns(test)) {
			wil::com_ptr<ICoreWebView2WebResourceResponse> response;
			env->CreateWebResourceResponse(nullptr, 403, L"Blocked", L"Content-Type: text/plain", &response);
			args->put_Response(response.get());
			break;
		}

		return S_OK;
		}).Get(), &token);
}

void KrunkerWindow::create(HINSTANCE inst, int cmdshow, std::function<void()> callback) {
	if (folder->config["window"]["meta"]["replace"].get<bool>())
		title = Convert::wstring(folder->config["window"]["meta"]["title"].get<std::string>());

	create_window(inst, cmdshow);

	SetClassLongPtr(m_hWnd, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(RGB(0, 0, 0)));

	if (folder->config["window"]["meta"]["replace"].get<bool>())
		SetIcon((HICON)LoadImage(inst, Convert::wstring(folder->config["window"]["meta"]["icon"].get<std::string>()).c_str(), IMAGE_ICON, 32, 32, LR_LOADFROMFILE));
	else SetIcon(LoadIcon(inst, MAKEINTRESOURCE(MAINICON)));

	call_create_webview(callback);
}

void KrunkerWindow::call_create_webview(std::function<void()> callback) {
	create_webview(cmdline(), folder->directory + folder->p_profile, [this, callback]() {
		wil::com_ptr<ICoreWebView2Controller2> control2;
		control2 = control.query<ICoreWebView2Controller2>();
		if (control2) control2->put_DefaultBackgroundColor(ColorRef(RGB(255, 255, 255)));

		ICoreWebView2Settings* settings;
		webview->get_Settings(&settings);

		settings->put_IsScriptEnabled(true);
		settings->put_AreDefaultScriptDialogsEnabled(true);
		settings->put_IsWebMessageEnabled(true);
		settings->put_IsZoomControlEnabled(false);
		settings->put_AreDefaultContextMenusEnabled(false);
#if _DEBUG != 1
		settings->put_AreDevToolsEnabled(false);
#else
		webview->OpenDevToolsWindow();
#endif

		resize_wv();

		register_events();

		webview->Navigate((L"https://krunker.io/" + pathname).c_str());

		LOG_INFO("KrunkerWindow created: " << Convert::string(pathname));

		callback();
		});
}

void KrunkerWindow::get(HINSTANCE inst, int cmdshow, std::function<void(bool)> callback) {
	if (!open) create(inst, cmdshow, [this, callback]() {
		callback(true);
		});
	else callback(false);
}

void KrunkerWindow::on_dispatch() {
	mtx.lock();
	for (JSON message : post)
		webview->PostWebMessageAsJson(Convert::wstring(message.dump()).c_str());

	post.clear();
	mtx.unlock();
}