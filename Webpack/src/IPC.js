'use strict';

var Events = require('./libs/Events');

class IPC extends Events {
	constructor(){
		super();
	}
	send(event, ...data){
		chrome.webview.postMessage(JSON.stringify([ event, ...data ]));
		return true;
	}
};

module.exports = new IPC();