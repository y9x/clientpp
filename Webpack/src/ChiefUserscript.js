'use strict';

var console = require('./Console'),
	utils = require('./libs/Utils');

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
		if(!this.metadata.locations.some(s => s == site || s == 'all'))return false;
		
		var exports = {},
			func,
			context = { _metadata: this.metadata, exports, console };
			
		
		try{
			// cannot use import/export, fix soon
			func = eval(`(function(${Object.keys(context)}){${script}\n//# sourceURL=https://krunker.io/userscripts:/${this.name}\n})`);
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
			func(...Object.values(context));
		}catch(err){
			console.error(`Error executing userscript ${this.name}:\n`, err);
			return false;
		}
		
		var { gui } = this.metadata.features;
		
		console.log('shees', gui);
		
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