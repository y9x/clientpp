#include "./Site.h"

namespace krunker {
const wchar_t *game = L"/";
const wchar_t *games = L"/games";
const wchar_t *editor = L"/editor.html";
const wchar_t *social = L"/social.html";
const wchar_t *viewer = L"/viewer.html";
const wchar_t *scripting = L"/scripting.html";

type identifyType(UriW uri) {
  if (uri.host() == L"krunker.io") {
    if (uri.path() == krunker::game || uri.path().starts_with(krunker::games))
      return type::Game;
    else if (uri.path() == krunker::social)
      return type::Social;
    else if (uri.path() == krunker::editor)
      return type::Editor;
    else if (uri.path() == krunker::viewer)
      return type::Viewer;
    else if (uri.path() == krunker::scripting)
      return type::Scripting;
  }

  return type::Invalid;
}
} // namespace krunker