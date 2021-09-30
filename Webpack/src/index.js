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
require('./FastLoad');

var HTMLProxy = require('./libs/HTMLProxy'),
	Category = require('./libs/MenuUI/Window/Category'),
	IPC = require('./libs/IPC'),
	Utils = require('./libs/Utils'),
	Events = require('./libs/Events'),
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
		
		var Inst = this.category('Installation');
		
		Inst.control('Folder', {
			type: 'function',
			button: 'Open',
			value(){
				ipc.send('open folder');
			},
		});
		
		var Render = this.category('Rendering');
		
		Render.control('Uncap FPS', {
			type: 'boolean',
			walk: 'client.uncap_fps',
		}).on('change', (value, init) => !init && this.relaunch());
		
		Render.control('Fullscreen', {
			type: 'boolean',
			walk: 'client.fullscreen',
		}).on('change', (value, init) => !init && ipc.send('fullscreen', value));
		
		var Game = this.category('Game');
		
		// loads krunker from api.sys32.dev
		Game.control('Fast Loading', {
			type: 'boolean',
			walk: 'game.fast_load',
		});
		
		Game.control('Seek new Lobby [F4]', {
			type: 'boolean',
			walk: 'game.f4_seek',
		}).on('change', (value, init) => !init && setTimeout(() => ipc.send('reload config')));
		
		for(let category of this.categories)category.update(true);
	}
	relaunch(){
		ipc.send('relaunch');
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