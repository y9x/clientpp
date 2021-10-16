'use strict';

var ipc = require('./IPC'),
	utils = require('./libs/Utils'),
	{ site_location } = require('./Consts');

class RPC {
	start = Date.now();
	listener(){
		this.update();
	}
	constructor(){
		this.interval = setInterval(this.update.bind(this), 1000);
	}
	delete(){
		clearInterval(this.interval);
	}
	update(force = false){
		if(!window.getGameActivity)return;
		
		var activity;
		
		try{
			activity = window.getGameActivity();
		}catch(err){
			return;
		}

		var { user, map, mode } = activity,
			args = [ user, map, mode ],
			jargs = JSON.stringify(args);
		
		if(!force && jargs != this.last_args){
			ipc.send('rpc', this.start, ...args);
			this.last_args = jargs;
		}
	}
};

module.exports = RPC;