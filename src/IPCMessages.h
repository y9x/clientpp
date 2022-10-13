#pragma once

namespace LogType {
constexpr unsigned char info = 0, error = 1, warn = 2, debug = 3;
};

namespace IM {
// incoming
constexpr unsigned char rpc_update = 0, rpc_clear = 1, save_config = 3,
                        shell_open = 4, fullscreen = 5, update_meta = 6,
                        revert_meta = 7, reload_config = 8, browse_file = 9,
                        mousedown = 10, mouseup = 11, mousemove = 12,
                        mousewheel = 13, pointer = 14, open_devtools = 15,
                        log = 16, relaunch_webview = 17, close_window = 18,
                        reload_window = 19, seek_game = 20,
                        toggle_fullscreen = 21, update_menu = 22,
                        account_list = 23, account_password = 24,
                        account_set = 25, account_remove = 26,
                        account_set_password = 27, account_regen = 28;
}; // namespace IM