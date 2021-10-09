'use strict';

var Events = require('./libs/Events');

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
		chrome.webview.postMessage(JSON.stringify([ event, ...data ]));
		return true;
	}
};

module.exports = new IPC();