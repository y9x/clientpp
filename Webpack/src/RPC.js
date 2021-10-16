'use strict';

var ipc = require('./IPC'),
	{ utils, site_location } = require('./Consts'),
	start = Date.now(),
	last_args,
	update_rpc = (force = false) => {
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
		
		if(!force && jargs != last_args){
			ipc.send('rpc', start, ...args);
			last_args = jargs;
		}
	};

if(site_location == 'game'){
	for(let method of ['pushState','replaceState']){
		let original = history[method];
		
		history[method] = function(data, title, url){
			var ret = original.call(this, data, title, url);
			update_rpc();
			return ret;
		};
	}
	
	setInterval(() => update_rpc(), 100);
}

module.exports = update_rpc;