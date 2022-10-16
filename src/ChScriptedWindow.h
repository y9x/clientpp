#pragma once
#include "./ChWindow.h"
#include <rapidjson/fwd.h>

class ChScriptedWindow : public ChWindow {
private:
  // Window-Frontend messaging:

  std::map<short, std::pair<std::function<void(const rapidjson::Value &)>,
                            std::function<void(const rapidjson::Value &)>>>
      postedMessages;

  // Fullscreen:
  RECT saved_size;
  DWORD saved_style = 0;
  DWORD saved_ex_style = 0;
  bool fullscreen = false;
  bool enterFullscreen();
  bool exitFullscreen();
  std::vector<JSMessage> pendingMessages;

  std::wstring genericJS;

  // Messaging:

  /// @brief Used by runtimeData
  rapidjson::Value getUserScripts(
      rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> allocator);

  // Window:

  /// @brief Send a resource defined in Resource.rc in response to a
  /// WebResourceRequestedEvent
  /// @param args WebResourceRequestedEvent args
  /// @param resource Resource ID
  /// @param mime Mime type
  /// @return If the operation was successful
  bool sendResource(ICoreWebView2WebResourceRequestedEventArgs *args,
                    int resource, std::wstring mime);
  virtual Status create(std::function<void()> callback = nullptr) override;

protected:
  /// @brief Handle a message sent from the frontend
  /// @param msg
  virtual void handleMessage(JSMessage msg);

  virtual void registerEvents() override;

  /// @brief Ran as soon as the DOM is available and the domain is krunker.io
  virtual void injectJS();

  /// @brief Produce runtime data
  /// format.
  /// @return UserScripts, UserStyles, and config in JSON format
  std::string runtimeData();

  /// @brief Used by runtimeData
  virtual rapidjson::Value getUserStyles(
      rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> allocator);

  /// @brief Asynchronously sends a message and expects a result.
  /// @param msg
  /// @param then
  /// @param catchError
  /// @return
  bool postMessage(const JSMessage &msg,
                   std::function<void(const rapidjson::Value &)> then,
                   std::function<void(const rapidjson::Value &)> catchError);
  /// @brief Asynchronously sends a message.
  /// @param message
  /// @return
  bool sendMessage(const JSMessage &message);

public:
  ChScriptedWindow(ClientFolder &folder, ChWindows &windows, Vector2 scale,
                   std::wstring title);
  /// @brief Execute any pending operations from the main thread
  virtual void dispatch() override;
};
