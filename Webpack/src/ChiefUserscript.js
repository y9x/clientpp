'use strict';

var utils = require('./libs/Utils');

class ChiefUserscript {
	constructor(name, metadata, menu){
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
		
		var func;
		
		try{
			// cannot use import/export, fix soon
			func = new Function('_metadata', script);
		}catch(err){
			console.error(`Error parsing userscript ${this.name}:\n`, err);
			return false;
		}
		
		try{
			func.call(window, this.metadata);
		}catch(err){
			console.error(`Error executing userscript ${this.name}:\n`, err);
			return false;
		}
		
		var { gui } = this.metadata.features;
		
		if(menu){
			// this.metadata.features.config
			// menu.config.userscripts[this.metadata.author]
			for(let [ labelct, controls ] of Object.entries(gui)){
				let category = menu.category(labelct);
				
				for(let [ labelco, data ] of Object.entries(controls)){
					let control = category.control(labelco, data);
					
					control.on('change', (value, init) => {
						// send to module export
						// data.
					});
				}
			}
		}
		
		return true;
	}
};

module.exports = ChiefUserscript;