'use strict';

var utils = require('./libs/Utils');

class ChiefUserscript {
	constructor(name, metadata){
		this.name = name;
		this.metadata = metadata;
		
		// utils.assign_type(this.template, metadata);
		// done in host c++
		
		var { libs, gui } = this.metadata.features;
		
		if(libs.utils){
			libs.utils = utils;
		}
	}
	// returns false if the script failed to execute, otherwise true
	async run(script, site, menu){
		if(!this.metadata.locations.includes(site))return false;
		
		var exports = {},
			func;
		
		try{
			// cannot use import/export, fix soon
			func = new Function('_exports', '_metadata', script);
		}catch(err){
			console.error(`Error parsing userscript ${this.name}:\n`, err);
			return false;
		}
		
		if(menu){
			let { userscripts } = menu.config,
				{ author, features } = this.metadata;
			
			if(!userscripts[author])userscripts[author] = {};
			
			userscripts[author] = utils.assign_deep(utils.clone_obj(features.config), menu.config.userscripts[author]);
			
			Object.defineProperty(this.metadata.features, 'config', {
				get(){
					return userscripts[author];
				}
			});
		}
		
		try{
			func.call(window, exports, this.metadata);
		}catch(err){
			console.error(`Error executing userscript ${this.name}:\n`, err);
			return false;
		}
		
		var { gui } = this.metadata.features;
		
		if(menu)for(let [ labelct, controls ] of Object.entries(gui)){
			let category = menu.category(labelct);
			
			for(let [ labelco, data ] of Object.entries(controls)){
				let change_callback = data.change;
				
				delete data.change;
				
				if(typeof data.walk == 'string')data.walk = `userscripts.${this.metadata.author}.${data.walk}`;
				
				let control = category.control(labelco, data);
				
				if(change_callback)control.on('change', (value, init) => {
					var func = exports[change_callback];
					
					if(func)func(value, init);
				});
			}
		}
		
		return true;
	}
};

module.exports = ChiefUserscript;