'use strict';

var { IM, RPCM, ipc } = require('./IPC'),
	utils = require('./libs/Utils'),
	site_location = require('./SiteLocation');

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
		
		if(!force && jargs != this.last){
			ipc.send(RPCM.update, this.start, ...args);
			this.last = jargs;
		}
	}
};

module.exports = RPC;