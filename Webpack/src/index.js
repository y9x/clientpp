'use strict';

window.onbeforeunload = () => {};
Object.defineProperty(window, 'onbeforeunload', { writable: false, value(){} })

require('./FilePicker');

var ExtendMenu = require('./libs/ExtendMenu'),
	Keybind = require('./libs/Keybind'),
	utils = require('./libs/Utils'),
	RPC = require('./RPC'),
	{ ipc, IM } = require('./IPC'),
	{ config: runtime_config, js } = require('./Runtime'),
	site_location = require('./SiteLocation');

class Menu extends ExtendMenu {
	rpc = new RPC();
	save_config(){
		ipc.send(IM.save_config, this.config);
	}
	config = runtime_config;
	default_config = require('../../Client/Config.json');
	constructor(){
		super();
		
		var Client = this.category('Client');
		
		Client.control('Github', {
			type: 'linkfunction',
			value(){
				ipc.send(IM.shell_open, 'url', 'https://github.com/y9x/clientpp');
			},
		});
		
		Client.control('Discord', {
			type: 'linkfunction',
			value(){
				ipc.send(IM.shell_open, 'url', 'https://discord.gg/4r47ZwdSQj');
			},
		});
		
		var Folder = this.category('Folders');
		
		/*Folder.control('Root', {
			type: 'function',
			text: 'Open',
			value: () => ipc.send(IM.shell_open, 'root'),
		});*/
		
		Folder.control('Scripts', {
			type: 'function',
			text: 'Open',
			value: () => ipc.send(IM.shell_open, 'scripts'),
		});
		
		Folder.control('Styles', {
			type: 'function',
			text: 'Open',
			value: () => ipc.send(IM.shell_open, 'styles'),
		});
		
		Folder.control('Resource Swapper', {
			type: 'function',
			text: 'Open',
			value: () => ipc.send(IM.shell_open, 'swapper'),
		});
		
		var Render = this.category('Rendering');
		
		Render.control('Adblock', {
			type: 'boolean',
			walk: 'client.adblock',
		}).on('change', (value, init) => !init && location.assign('/'));
		
		Render.control('Uncap FPS', {
			type: 'boolean',
			walk: 'client.uncap_fps',
		}).on('change', (value, init) => !init && ipc.send(IM.relaunch_webview));
		
		Render.control('Fullscreen', {
			type: 'boolean',
			walk: 'client.fullscreen',
		}).on('change', (value, init) => !init && ipc.send(IM.fullscreen));
		
		var Game = this.category('Game');
		
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
			if(!value)ipc.send(IM.rpc_clear);
			else{
				ipc.send(IM.rpc_init);
				this.rpc.update(true);
			}
		});
		
		RPC.control('Show username', {
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
			
			if(value)ipc.send(IM.update_meta);
			else ipc.send(IM.revert_meta);
		});
		
		Window.control('New Title', {
			type: 'textbox',
			walk: 'window.meta.title',
		}).on('change', (value, init) => {
			if(!init && this.config.window.meta.replace)
				ipc.send(IM.update_meta);
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
				ipc.send(IM.update_meta);
		});
		
		this.keybinds();
		
		for(let category of this.categories)category.update(true);
		
		this.insert('Client');
	}
	keybinds(){
		new Keybind('F4', event => {
			if(event.altKey)ipc.send(IM.close_window);
			else if(this.config.game.f4_seek)ipc.send(IM.seek_game);
		});
		
		new Keybind('F10', event => {
			ipc.send(IM.open_devtools);
		});
		
		new Keybind('F11', () => {
			this.config.client.fullscreen = !this.config.client.fullscreen;
			this.save_config();
			ipc.send(IM.fullscreen);
		});
	}
};

require('./Resources');

if(site_location == 'game'){
	require('./Fixes');
	new Menu();
}

new Keybind('F5', event => ipc.send(IM.reload_window));