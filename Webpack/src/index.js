'use strict';

window.onbeforeunload = () => {};
Object.defineProperty(window, 'onbeforeunload', { writable: false, value(){} })

require('./FilePicker');

var ExtendMenu = require('./libs/ExtendMenu'),
	Category = require('./libs/MenuUI/Window/Category'),
	Events = require('./libs/Events'),
	Keybind = require('./libs/Keybind'),
	utils = require('./libs/Utils'),
	ipc = require('./IPC'),
	RPC = require('./RPC'),
	{ config: runtime_config, js } = require('./Runtime'),
	{ site_location, meta } = require('./Consts');

class Menu extends ExtendMenu {
	rpc = new RPC();
	save_config(){
		ipc.send('save config', this.config);
	}
	config = runtime_config;
	default_config = require('../../Client/Config.json');
	constructor(){
		super();
		
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
		
		Render.control('Adblock', {
			type: 'boolean',
			walk: 'client.adblock',
		}).on('change', (value, init) => !init && location.assign('/'));
		
		Render.control('Uncap FPS', {
			type: 'boolean',
			walk: 'client.uncap_fps',
		}).on('change', (value, init) => !init && this.relaunch());
		
		Render.control('Fullscreen', {
			type: 'boolean',
			walk: 'client.fullscreen',
		}).on('change', (value, init) => !init && ipc.send('fullscreen'));
		
		var Game = this.category('Game');
		
		// loads krunker from api
		Game.control('Fast Loading (Risky)', {
			type: 'boolean',
			walk: 'game.fast_load',
		});
		
		Game.control('Seek new Lobby [F4]', {
			type: 'boolean',
			walk: 'game.f4_seek',
		});
		
		var RPC = this.category('Discord RPC');
		
		RPC.control('Enabled', {
			type: 'boolean',
			walk: 'rpc.enabled',
		}).on('change', (value, init) => {
			if(init)return;
			if(!value)ipc.send('rpc_uninit');
			else this.rpc.update(true);
		});
		
		RPC.control('Show name', {
			type: 'boolean',
			walk: 'rpc.name',
		}).on('change', (value, init) => !init && this.rpc.update(true));
		
		var Window = this.category('Window');
		
		Window.control('DevTools [F10]', {
			type: 'boolean',
			walk: 'client.devtools',
		});
		
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
		
		this.keybinds();
		
		for(let category of this.categories)category.update(true);
		
		this.insert('Client');
	}
	keybinds(){
		new Keybind('F4', event => {
			if(event.altKey)ipc.send('close window');
			else if(this.config.game.f4_seek)ipc.send('seek game');
		});
		
		new Keybind('F10', event => {
			ipc.send('open devtools');
		});
		
		new Keybind('F11', () => {
			this.config.client.fullscreen = !this.config.client.fullscreen;
			this.save_config();
			ipc.send('fullscreen');
		});
	}
	relaunch(){
		ipc.send('relaunch');
	}
};

require('./Resources');

if(site_location == 'game'){
	require('./Fixes');
	require('./FastLoad');
	new Menu();
}

new Keybind('F5', event => ipc.send('reload'));