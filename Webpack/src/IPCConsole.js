'use strict';

class IPCConsole {
	constructor(ipc, prefix){
		this.ipc = ipc;
		this.prefix = [];
		if(typeof prefix == 'string')this.prefix.push(prefix);
	}
	log(...args){
		this.ipc.send('log', 'info', this.prefix.concat(args).join(' '));
	}
	info(...args){
		this.ipc.send('log', 'info', this.prefix.concat(args).join(' '));
	}
	warn(...args){
		this.ipc.send('log', 'warn', this.prefix.concat(args).join(' '));
	}
	error(...args){
		this.ipc.send('log', 'error', this.prefix.concat(args).join(' '));
	}
};

module.exports = IPCConsole;