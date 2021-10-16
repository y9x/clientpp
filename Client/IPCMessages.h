#pragma once

enum class LogType {
	info,
	error,
	warn,
	debug,
};

// 0,1,2,3,4...
enum class IM {
	send_webpack,
	eval_webpack,
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
	rpc_init,
	rpc_uninit,
	rpc_update,
	log,
	relaunch_webview,
	close_window,
	reload_window,
	seek_game,
};