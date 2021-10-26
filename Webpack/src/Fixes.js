'use strict';

var utils = require('./libs/Utils'),
	{ ipc, IM }  = require('./IPC'),
	console = require('./Console'),
	listening = new WeakSet(),
	locked_node;

ipc.send(IM.pointer, false);

window.addEventListener('beforeunload', () => {
	document.exitPointerLock();
	ipc.send(IM.pointer, false);
});

document.addEventListener('pointerlockchange', () => {
	if(!document.pointerLockElement){
		locked_node = null;
		return ipc.send(IM.pointer, false);
	}
	
	locked_node = document.pointerLockElement;
	ipc.send(IM.pointer, true);
});

setInterval(() => {
	ipc.send(IM.pointer, document.pointerLockElement != void[]);
}, 250);

ipc.on(IM.mousewheel, deltaY => {
	locked_node?.dispatchEvent(new WheelEvent('wheel', { deltaY }));
});

ipc.on(IM.mousedown, button => {
	locked_node?.dispatchEvent(new MouseEvent('mousedown', { button }));
});

ipc.on(IM.mouseup, button => {
	locked_node?.dispatchEvent(new MouseEvent('mouseup', { button }));
});

ipc.on(IM.mousemove, (movementX, movementY) => {
	locked_node?.dispatchEvent(new MouseEvent('mousemove', { movementX, movementY }));
});

if(localStorage.kro_setngss_scaleUI == void[])localStorage.kro_setngss_scaleUI = 0.7;

var MIN_WIDTH = 1700,
	MIN_HEIGHT = 900;

if(localStorage.kro_setngss_uiScaling !== 'false'){
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


	utils.wait_for(() => document.querySelector('#uiBase')).then(ui_base => setTimeout(() => {
		ui_base.style.transform = 'scale(' + style.transform.toFixed(3) + ')';
		ui_base.style.width = style.width.toFixed(3) + 'px';
		ui_base.style.height = style.height.toFixed(3) + 'px';
	}, 10));
}