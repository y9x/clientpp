#include "./Site.h"

namespace krunker {
const wchar_t *game = L"/";
const wchar_t *games = L"/games";
const wchar_t *editor = L"/editor.html";
const wchar_t *social = L"/social.html";
const wchar_t *viewer = L"/viewer.html";
const wchar_t *scripting = L"/scripting.html";

bool host_is_krunker(const std::wstring &host) {
  return host == L"krunker.io" || host.ends_with(L".krunker.io");
}
} // namespace krunker