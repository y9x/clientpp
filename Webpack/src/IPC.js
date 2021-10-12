'use strict';

var Events = require('./libs/Events'),
	{ webview } = chrome;

delete chrome.webview;

class IPCConsole {
	constructor(ipc, prefix){
		this.ipc = ipc;
		this.prefix = [];
		if(typeof prefix == 'string')this.prefix.push(prefix);
	}
	log(...args){
		this.ipc.send('log', 'info', args.join(' '));
	}
	info(...args){
		this.ipc.send('log', 'info', args.join(' '));
	}
	warn(...args){
		this.ipc.send('log', 'warn', args.join(' '));
	}
	error(...args){
		this.ipc.send('log', 'error', args.join(' '));
	}
	debug(...args){
		this.ipc.send('log', 'debug', args.join(' '));
	}
};

class IPC extends Events {
	constructor(){
		super();
	}
	console = new IPCConsole();
	send(event, ...data){
		webview.postMessage(JSON.stringify([ event, ...data ]));
		return true;
	}
};

var ipc = new IPC();

webview.addEventListener('message', ({ data }) => ipc.emit(...data));

module.exports = ipc;