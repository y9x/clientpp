'use strict';

// webview is chrome.webview but captured by bootstrap.js

var Events = require('./libs/Events'),	
	messages = require('../../Client/IPCMessages.h'),
	{ IM, LogType } = messages;

class IPCConsole {
	constructor(ipc, prefix){
		this.ipc = ipc;
	}
	log(...args){
		this.ipc.send(IM.log, LogType.info, args.join(' '));
	}
	info(...args){
		this.ipc.send(IM.log, LogType.info, args.join(' '));
	}
	warn(...args){
		this.ipc.send(IM.log, LogType.warn, args.join(' '));
	}
	error(...args){
		this.ipc.send(IM.log, LogType.error, args.join(' '));
	}
	debug(...args){
		this.ipc.send(IM.log, LogType.debug, args.join(' '));
	}
};

class IPC extends Events {
	constructor(){
		super();
	}
	console = new IPCConsole(this);
	send(event, ...data){
		if(typeof event != 'number')throw new TypeError(`Event must be a number. Recieved '${event}'`);
		webview.postMessage(JSON.stringify([ event, ...data ]));
		return true;
	}
};

var ipc = new IPC();

webview.addEventListener('message', ({ data }) => ipc.emit(...data));

Object.assign(exports, messages);
exports.ipc = ipc;
exports.IM = IM;