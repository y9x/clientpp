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
		this.ipc.send(IM.log, LogType.info, args.join(' ') + '\n');
	}
	info(...args){
		this.ipc.send(IM.log, LogType.info, args.join(' ') + '\n');
	}
	warn(...args){
		this.ipc.send(IM.log, LogType.warn, args.join(' ') + '\n');
	}
	error(...args){
		this.ipc.send(IM.log, LogType.error, args.join(' ') + '\n');
	}
	debug(...args){
		this.ipc.send(IM.log, LogType.debug, args.join(' ') + '\n');
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
	post(event, ...data){
		var id = ~~(Math.random() * 2147483647);
		
		return new Promise((resolve, reject) => {
			this.once(id, (data, error) => {
				if(error)reject(error);
				else resolve(data);
			});
			this.send(event, id, ...data);
		});
	}
};

var ipc = new IPC();

webview.addEventListener('message', ({ data }) => ipc.emit(...data));

Object.assign(exports, messages);
exports.ipc = ipc;
exports.IM = IM;