#pragma once

enum class LogType {
	info,
	error,
	warn,
	debug,
};

// 0,1,2,3,4...
enum class IM {
	rpc_update,
	rpc_clear,
	rpc_init,
	save_config,
	shell_open,
	fullscreen,
	update_meta,
	revert_meta,
	reload_config,
	browse_file,
	mousedown,
	pointer,
	mouse_locked,
	open_devtools,
	log,
	relaunch_webview,
	close_window,
	reload_window,
	seek_game,
};