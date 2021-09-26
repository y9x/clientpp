'use strict';

var Events = require('./Events');

class IPC extends Events {
	#send
	constructor(send){
		super();
		
		if(typeof send != 'function')throw new TypeError('Invalid send function');
		this.#send = send;
	}
	send(event, ...data){
		this.#send(event, ...data);
		return true;
	}
};

module.exports = IPC;