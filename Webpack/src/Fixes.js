'use strict';

var { utils } = require('./Consts'),
	ipc = require('./IPC'),
	listening = new WeakSet(),
	locked_node,
	listener = event => {
		if(event.button == 0)console.log('Recieved', event.ipc ? 'simulated' : 'real', 'left mousedown event');
	},
	is_game_canvas = node => node?.className == 'canvas' && !node.id;



document.addEventListener('pointerlockchange', () => {
	if(!document.pointerLockElement){
		locked_node?.removeEventListener('mousedown', listener);
		locked_node = null;
		return ipc.send('pointer', 'unhook');
	}
	
	locked_node = document.pointerLockElement;
	
	if(is_game_canvas(locked_node)){
		locked_node.addEventListener('mousedown', listener)
		ipc.send('pointer', 'hook');
	}
});

setInterval(() => {
	ipc.send('locked poll', is_game_canvas(document.pointerLockElement));
}, 1000);

ipc.on('mousedown', () => {
	var event = new MouseEvent('mousedown');
	event.ipc = true;
	locked_node?.dispatchEvent(event);
});

// document.addEventListener('click', event => event.target.nodeName == 'A' && event.target.requestPointerLock());

(async () => {
	if(localStorage.kro_setngss_scaleUI == void[])
		localStorage.kro_setngss_scaleUI = 1;
	
	var ui_base = await utils.wait_for(() => document.querySelector('#uiBase')),
		MIN_WIDTH = 1700,
		MIN_HEIGHT = 900;
		
	if(localStorage.kro_setngss_uiScaling === 'false')return;
	
	var ls = localStorage.kro_setngss_scaleUI,
		scale_ui = ls != void[] ? parseInt(ls) : 0.7;
	
	scale_ui = Math.min(1, Math.max(0.1, Number(scale_ui)));
	scale_ui = 1 + (1 - scale_ui);
	
	var height = window.innerHeight,
		width = window.innerWidth,
		min_width = MIN_WIDTH * scale_ui,
		min_height = MIN_HEIGHT * scale_ui,
		width_scale = width / min_width,
		height_scale = height / min_height,
		elm = document.getElementById('uiBase'),
		style = height_scale < width_scale ? {
			transform: height_scale,
			width: width / height_scale,
			height: min_height,
		} : {
			transform: width_scale,
			width: min_width,
			height: height / width_scale,
		};
	
	ui_base.style.transform = 'scale(' + style.transform.toFixed(3) + ')';
	ui_base.style.width = style.width.toFixed(3) + 'px';
	ui_base.style.height = style.height.toFixed(3) + 'px';
})();