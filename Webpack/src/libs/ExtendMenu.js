'use strict';

var utils = require('../libs/utils'),
	Events = require('./Events'),
	HTMLProxy = require('./HTMLProxy'),
	Category = require('./MenuUI/Window/Category');

class ExtendMenu extends Events {
	html = new HTMLProxy();
	async save_config(){
		console.error('save_config() not implemented');
	}
	async load_config(){
		console.error('load_config() not implemented');
	}
	tab = {
		content: this.html,
		window: {
			menu: this,
		},
	};
	async attach(){
		if(this.window instanceof Object)throw new Error('Already attached');
		
		var array = await utils.wait_for(() => typeof windows == 'object' && windows);
		
		this.window = array.find(window => window.header == this.header);
		
		if(!this.window)throw new Error(`Unable to find header '${this.header}'`);
		
		this.index = this.window.tabs.push({
			name: this.label,
			categories: [],
		}) - 1;
		
		this.getSettings = this.window.getSettings;
		
		this.window.getSettings = () => this.window.tabIndex == this.index ? this.html.get() : this.getSettings.call(this.window);
	}
	detach(){
		if(!(this.window instanceof Object))throw new Error('Not attached');
		
		this.window.tabs.splice(this.index, 1);
		this.window.getSettings = this.getSettings;
		this.window = null;
	}
	categories = new Set();
	category(label){
		var cat = new Category(this.tab, label);
		this.categories.add(cat);
		return cat;
	}
	update(init = false){
		for(let category of this.categories)category.update(init);	
	}
	constructor(header, label){
		super();
		this.header = header;
		this.label = label;
	}
};

module.exports = ExtendMenu;