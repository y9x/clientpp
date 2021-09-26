'use strict';

/*
console.log('Running');

var log = console.log;
Object.defineProperty(console, 'log', {
	get(){
		return log;
	},
	set(value){
		return value;
	},
	configurable: false,
});
*/

require('./FixLoad');
require('./Resources');

var HTMLProxy = require('./HTMLProxy'),
	Category = require('./MenuUI/Window/Category'),
	IPC = require('./IPC'),
	Utils = require('./Utils'),
	Events = require('./Events'),
	utils = new Utils(),
	ipc = new IPC((...data) => chrome.webview.postMessage(JSON.stringify(data)));

chrome.webview.addEventListener('message', ({ data }) => ipc.emit(...JSON.parse(data)));

class Menu extends Events {
	html = new HTMLProxy();
	config = require('./Runtime').config;
	default_config = require('../../Client/Config.json');
	tab = {
		content: this.html,
		window: {
			menu: this,
		},
	};
	constructor(){
		super();
		
		this.main();
		
		var Client = this.category('Client');
		
		Client.control('Open Folder', {
			type: 'function',
			value(){
				ipc.send('open folder');
			},
		});
		
		Client.control('Uncap FPS', {
			type: 'boolean',
			walk: 'client.uncap_fps',
		});
		
		for(let category of this.categories)category.update(true);
	}
	categories = new Set();
	category(label){
		var cat = new Category(this.tab, label);
		this.categories.add(cat);
		return cat;
	}
	async save_config(){
		ipc.send('save config', this.config);
	}
	async main(){
		var array = await utils.wait_for(() => typeof windows == 'object' && windows),
			settings = array[0],
			index = settings.tabs.length,
			get = settings.getSettings;
	
		settings.tabs.push({ name: 'Client', categories: [] });
		
		settings.getSettings = () => settings.tabIndex == index ? this.html.get() : get.call(settings);
	}
};

new Menu();