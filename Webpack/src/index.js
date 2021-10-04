'use strict';

/*
use fast load for development
prevents the game loader from disabling console

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
	Control = require('./libs/MenuUI/Control'),
	IPC = require('./libs/IPC'),
	Events = require('./libs/Events'),
	Keybind = require('./libs/Keybind'),
	ipc = new IPC((...data) => chrome.webview.postMessage(JSON.stringify(data))),
	{ config: runtime_config, js } = require('./Runtime'),
	{ utils, meta } = require('./Consts');

class FilePicker extends Control.Types.TextBoxControl {
	static id = 'filepicker';
	create(...args){
		super.create(...args);
		this.browse = utils.add_ele('div', this.content, {
			className: 'settingsBtn',
			textContent: 'Browse',
			style: {
				width: '100px',
			},
			events: {
				click: () => {
					var id = Math.random();
					
					ipc.once(id, (data, error) => {
						if(error)return;
						this.value = this.input.value = data;
					});
					
					// send entries instead of an object, c++ json parser removes the order
					ipc.send('browse file', id, this.data.title, Object.entries(this.data.filters));
				},
			},
		});
	}
};

Control.Types.FilePicker = FilePicker;

chrome.webview.addEventListener('message', ({ data }) => ipc.emit(...data));

class Menu extends Events {
	html = new HTMLProxy();
	config = runtime_config;
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
		
		Client.control('Github', {
			type: 'linkfunction',
			value(){
				ipc.send('open', 'url', meta.github);
			},
		});
		
		Client.control('Discord', {
			type: 'linkfunction',
			value(){
				ipc.send('open', 'url', meta.discord);
			},
		});
		
		Client.control('Forum', {
			type: 'linkfunction',
			value(){
				ipc.send('open', 'url', meta.forum);
			},
		});
		
		var Folder = this.category('Folders');
		
		/*Folder.control('Root', {
			type: 'function',
			text: 'Open',
			value: () => ipc.send('open', 'root'),
		});*/
		
		Folder.control('Scripts', {
			type: 'function',
			text: 'Open',
			value: () => ipc.send('open', 'scripts'),
		});
		
		Folder.control('Styles', {
			type: 'function',
			text: 'Open',
			value: () => ipc.send('open', 'styles'),
		});
		
		Folder.control('Resource Swapper', {
			type: 'function',
			text: 'Open',
			value: () => ipc.send('open', 'swapper'),
		});
		
		var Render = this.category('Rendering');
		
		Render.control('Uncap FPS', {
			type: 'boolean',
			walk: 'client.uncap_fps',
		}).on('change', (value, init) => !init && this.relaunch());
		
		Render.control('Fullscreen', {
			type: 'boolean',
			walk: 'client.fullscreen',
		}).on('change', (value, init) => !init && ipc.send('fullscreen'));
		
		var Game = this.category('Game');
		
		// loads krunker from api.sys32.dev
		if(!js.length)Game.control('Fast Loading', {
			type: 'boolean',
			walk: 'game.fast_load',
		});
		
		Game.control('Seek new Lobby [F4]', {
			type: 'boolean',
			walk: 'game.f4_seek',
		});
		
		new Keybind('F4', () => {
			if(this.config.game.f4_seek)location.assign('/');
		});
		
		new Keybind('F11', () => {
			this.config.client.fullscreen = !this.config.client.fullscreen;
			this.save_config();
			ipc.send('fullscreen');
		});
		
		var Window = this.category('Window');
		
		Window.control('Replace Icon & Title', {
			type: 'boolean',
			walk: 'window.meta.replace',
		}).on('change', (value, init) => {
			if(init)return;
			
			if(value)ipc.send('update meta');
			else ipc.send('revert meta');
		});
		
		Window.control('New Title', {
			type: 'textbox',
			walk: 'window.meta.title',
		}).on('change', (value, init) => {
			if(!init && this.config.window.meta.replace)
				ipc.send('update meta');
		});
		
		Window.control('New Icon', {
			type: 'filepicker',
			walk: 'window.meta.icon',
			title: 'Select a new Icon',
			filters: {
				'Icon': '*.ico',
				'All types': '*.*',
			},
		}).on('change', (value, init) => {
			if(!init && this.config.window.meta.replace)
				ipc.send('update meta');
		});
		
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
	save_config(){
		ipc.send('save config', this.config);
	}
	async main(){
		var array = await utils.wait_for(() => typeof windows == 'object' && windows),
			settings = array[0],
			index = settings.tabs.length,
			get = settings.getSettings;
	
		settings.tabs.push({
			name: 'Client',
			categories: [],
		});
		
		settings.getSettings = () => settings.tabIndex == index ? this.html.get() : get.call(settings);
	}
};

new Menu();