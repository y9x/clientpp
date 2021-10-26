/******/ (() => { // webpackBootstrap
/******/ 	var __webpack_modules__ = ({

/***/ "../Client/IPCMessages.h":
/*!*******************************!*\
  !*** ../Client/IPCMessages.h ***!
  \*******************************/
/***/ ((__unused_webpack_module, exports) => {

exports.LogType={info:0,error:1,warn:2,debug:3};exports.IM={rpc_update:0,rpc_clear:1,rpc_init:2,save_config:3,shell_open:4,fullscreen:5,update_meta:6,revert_meta:7,reload_config:8,browse_file:9,mousedown:10,mouseup:11,mousemove:12,mousewheel:13,pointer:14,open_devtools:15,log:16,relaunch_webview:17,close_window:18,reload_window:19,seek_game:20,toggle_fullscreen:21,update_menu:22}

/***/ }),

/***/ "./src/ChiefUserscript.js":
/*!********************************!*\
  !*** ./src/ChiefUserscript.js ***!
  \********************************/
/***/ ((module, __unused_webpack_exports, __webpack_require__) => {

"use strict";


var { ipc } = __webpack_require__(/*! ./IPC */ "./src/IPC.js"),
	console = __webpack_require__(/*! ./Console */ "./src/Console.js"),
	utils = __webpack_require__(/*! ./libs/Utils */ "./src/libs/Utils.js");

class ChiefUserscript {
	constructor(name, metadata){
		this.name = name;
		this.metadata = metadata;
		
		// utils.assign_type(this.template, metadata);
		// done in host c++
		
		var { libs, gui } = this.metadata.features;
		
		if(libs.utils)libs.utils = utils;
	}
	// returns false if the script failed to execute, otherwise true
	async run(script, site, menu){
		if(!this.metadata.locations.some(s => s == site || s == 'all'))return false;
		
		var exports = {},
			run,
			context = { _metadata: this.metadata, exports, console };
		
		try{
			// cannot use import/export, fix soon
			run = eval(`(function(${Object.keys(context)}){${script}\n//# sourceURL=https://krunker.io/userscripts:/${this.name}\n})`);
		}catch(err){
			console.warn(`Error parsing userscript: ${this.name}\n`, err);
			ipc.console.error(`Error parsing userscript ${this.name}:\n${err}`);
			return false;
		}
		
		if(menu){
			let { userscripts } = menu.config,
				{ author, features } = this.metadata;
			
			Object.defineProperty(features, 'config', {
				get(){
					return userscripts[author];
				}
			});
		}
		
		try{
			run(...Object.values(context));
		}catch(err){
			console.error(`Error executing userscript ${this.name}:\n`, err);
			ipc.console.error(`Error executing userscript ${name}:\n${err}`);
			return false;
		}
		
		var { gui } = this.metadata.features;
		
		if(menu)for(let [ labelct, controls ] of Object.entries(gui)){
			let category;
			
			// use existing category when possible
			for(let ct of menu.categories)if(ct.label == labelct)category = ct;
			if(!category)category = menu.category(labelct);
			
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

/***/ }),

/***/ "./src/Console.js":
/*!************************!*\
  !*** ./src/Console.js ***!
  \************************/
/***/ ((__unused_webpack_module, exports) => {

"use strict";


// script executes before devtools listens on logs

var methods = ['log','info','warn','error','debug','trace'],
	initial = {},
	buffer = {},
	tempcall = (type, ...data) => {
		
	};

for(let method of methods){
	initial[method] = console[method].bind(console);
	exports[method] = (...data) => {
		if(!buffer[method])buffer[method] = [];
		buffer[method].push(data);
	};
}

// devtools hooks after

setTimeout(() => {
	for(let method of methods){
		exports[method] = initial[method];
		if(buffer[method])for(let data of buffer[method])exports[method](...data);
	}
	
	buffer = null;
	initial = null;
});

/***/ }),

/***/ "./src/FilePicker.js":
/*!***************************!*\
  !*** ./src/FilePicker.js ***!
  \***************************/
/***/ ((module, __unused_webpack_exports, __webpack_require__) => {

"use strict";


var Control = __webpack_require__(/*! ./libs/MenuUI/Control */ "./src/libs/MenuUI/Control.js"),
	utils = __webpack_require__(/*! ./libs/Utils */ "./src/libs/Utils.js"),
	{ IM, ipc } = __webpack_require__(/*! ./IPC */ "./src/IPC.js");

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
					var id = ~~(Math.random() * 2147483647);
					
					ipc.once(id, (data, error) => {
						if(error)return;
						this.value = this.input.value = data;
					});
					
					// send entries instead of an object, c++ json parser removes the order
					ipc.send(IM.browse_file, id, this.data.title, Object.entries(this.data.filters));
				},
			},
		});
	}
};

module.exports = Control.Types.FilePicker = FilePicker;

/***/ }),

/***/ "./src/Fixes.js":
/*!**********************!*\
  !*** ./src/Fixes.js ***!
  \**********************/
/***/ ((__unused_webpack_module, __unused_webpack_exports, __webpack_require__) => {

"use strict";


var utils = __webpack_require__(/*! ./libs/Utils */ "./src/libs/Utils.js"),
	{ ipc, IM }  = __webpack_require__(/*! ./IPC */ "./src/IPC.js"),
	console = __webpack_require__(/*! ./Console */ "./src/Console.js"),
	listening = new WeakSet(),
	locked_node;

ipc.send(IM.pointer, false);

window.addEventListener('beforeunload', () => {
	document.exitPointerLock();
	ipc.send(IM.pointer, false);
});

document.addEventListener('pointerlockchange', () => {
	if(!document.pointerLockElement){
		locked_node = null;
		return ipc.send(IM.pointer, false);
	}
	
	locked_node = document.pointerLockElement;
	ipc.send(IM.pointer, true);
});

setInterval(() => {
	ipc.send(IM.pointer, document.pointerLockElement != void[]);
}, 250);

ipc.on(IM.mousewheel, deltaY => {
	locked_node?.dispatchEvent(new WheelEvent('wheel', { deltaY }));
});

ipc.on(IM.mousedown, button => {
	locked_node?.dispatchEvent(new MouseEvent('mousedown', { button }));
});

ipc.on(IM.mouseup, button => {
	locked_node?.dispatchEvent(new MouseEvent('mouseup', { button }));
});

ipc.on(IM.mousemove, (movementX, movementY) => {
	locked_node?.dispatchEvent(new MouseEvent('mousemove', { movementX, movementY }));
});

if(localStorage.kro_setngss_scaleUI == void[])localStorage.kro_setngss_scaleUI = 0.7;

var MIN_WIDTH = 1700,
	MIN_HEIGHT = 900;

if(localStorage.kro_setngss_uiScaling !== 'false'){
	var ls = localStorage.kro_setngss_scaleUI,
		scale_ui = ls != void[] ? parseInt(ls) : 0.7;

	scale_ui = Math.min(1, Math.max(0.1, Number(scale_ui)));
	scale_ui = 1 + (1 - scale_ui);

	var height = window.innerHeight,
		width = window.innerWidth,
		min_width = MIN_WIDTH * scale_ui,
		min_height = MIN_HEIGHT * scale_ui,
		width_scale = width / min_width,
		height_scale = height / min_height,
		elm = document.getElementById('uiBase'),
		style = height_scale < width_scale ? {
			transform: height_scale,
			width: width / height_scale,
			height: min_height,
		} : {
			transform: width_scale,
			width: min_width,
			height: height / width_scale,
		};


	utils.wait_for(() => document.querySelector('#uiBase')).then(ui_base => setTimeout(() => {
		ui_base.style.transform = 'scale(' + style.transform.toFixed(3) + ')';
		ui_base.style.width = style.width.toFixed(3) + 'px';
		ui_base.style.height = style.height.toFixed(3) + 'px';
	}, 10));
}

/***/ }),

/***/ "./src/IPC.js":
/*!********************!*\
  !*** ./src/IPC.js ***!
  \********************/
/***/ ((__unused_webpack_module, exports, __webpack_require__) => {

"use strict";


// webview is chrome.webview but captured by bootstrap.js

var Events = __webpack_require__(/*! ./libs/Events */ "./src/libs/Events.js"),	
	messages = __webpack_require__(/*! ../../Client/IPCMessages.h */ "../Client/IPCMessages.h"),
	{ IM, LogType } = messages;

class IPCConsole {
	constructor(ipc, prefix){
		this.ipc = ipc;
	}
	log(...args){
		this.ipc.send(IM.log, LogType.info, args.join(' ') + '\n');
	}
	info(...args){
		this.ipc.send(IM.log, LogType.info, args.join(' ') + '\n');
	}
	warn(...args){
		this.ipc.send(IM.log, LogType.warn, args.join(' ') + '\n');
	}
	error(...args){
		this.ipc.send(IM.log, LogType.error, args.join(' ') + '\n');
	}
	debug(...args){
		this.ipc.send(IM.log, LogType.debug, args.join(' ') + '\n');
	}
};

class IPC extends Events {
	constructor(){
		super();
	}
	console = new IPCConsole(this);
	send(event, ...data){
		if(typeof event != 'number')throw new TypeError(`Event must be a number. Recieved '${event}'`);
		webview.postMessage(JSON.stringify([ event, ...data ]));
		return true;
	}
};

var ipc = new IPC();

webview.addEventListener('message', ({ data }) => ipc.emit(...data));

Object.assign(exports, messages);
exports.ipc = ipc;
exports.IM = IM;

/***/ }),

/***/ "./src/LegacyUserscript.js":
/*!*********************************!*\
  !*** ./src/LegacyUserscript.js ***!
  \*********************************/
/***/ ((module, __unused_webpack_exports, __webpack_require__) => {

"use strict";


// Legacy IDKR userscript

var site = __webpack_require__(/*! ./Site */ "./src/Site.js");

class Userscript {
	#field(compare, value){
		if(Array.isArray(compare)){
			if(!Array.isArray(value))value = [].concat(value);
			
			if(!value.length || value.some(data => typeof data != 'string'))return compare;
		}else if(typeof compare != typeof value)return compare;
		
		return value;
	}
	#run = () => {}
	name = 'Unnamed userscript';
	version = 'Unknown version';
	author = 'Unknown author';
	description = 'No description provided';
	locations = ['game'];
	platforms = ['all'];
	settings = { used: false };
	constructor(data){
		this.#run = this.#field(this.#run, data.run);
		this.name = this.#field(this.name, data.name);
		this.version = this.#field(this.version, data.version);
		this.author = this.#field(this.author, data.author);
		this.description = this.#field(this.description, data.description);
		this.locations = this.#field(this.locations, data.locations);
		this.platforms = this.#field(this.platforms, data.platforms);
		this.settings = this.#field(this.settings, data.settings);
	}
	async run(){
		if(!this.locations.includes('all') && !this.locations.includes(site))return false;
		
		try{
			await this.#run();
			return true;
		}catch(err){
			console.error(err);
			return false;
		}
	}
};

module.exports = Userscript;

/***/ }),

/***/ "./src/RPC.js":
/*!********************!*\
  !*** ./src/RPC.js ***!
  \********************/
/***/ ((module, __unused_webpack_exports, __webpack_require__) => {

"use strict";


var { IM, ipc } = __webpack_require__(/*! ./IPC */ "./src/IPC.js");

class RPC {
	start = Date.now();
	listener(){
		this.update();
	}
	constructor(){
		this.interval = setInterval(this.update.bind(this), 1000);
	}
	delete(){
		clearInterval(this.interval);
	}
	update(force = false){
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
		
		if(!force && jargs != this.last){
			ipc.send(IM.rpc_update, this.start, ...args);
			this.last = jargs;
		}
	}
};

module.exports = RPC;

/***/ }),

/***/ "./src/Resources.js":
/*!**************************!*\
  !*** ./src/Resources.js ***!
  \**************************/
/***/ ((module, __unused_webpack_exports, __webpack_require__) => {

"use strict";


var { css, js } = __webpack_require__(/*! ./Runtime */ "./src/Runtime.js"),
	site = __webpack_require__(/*! ./Site */ "./src/Site.js"),
	utils = __webpack_require__(/*! ./libs/Utils */ "./src/libs/Utils.js"),
	{ ipc } = __webpack_require__(/*! ./IPC */ "./src/IPC.js"),
	console = __webpack_require__(/*! ./Console */ "./src/Console.js"),
	LegacyUserscript = __webpack_require__(/*! ./LegacyUserscript */ "./src/LegacyUserscript.js"),
	ChiefUserscript = __webpack_require__(/*! ./ChiefUserscript */ "./src/ChiefUserscript.js"),
	add_css = () => {
		for(let [ name, data ] of Object.entries(css)){
			let url = URL.createObjectURL(new Blob([ data ], { type: 'text/css' }));
			
			let link = document.head.appendChild(Object.assign(document.createElement('link'), {
				rel: 'stylesheet',
				href: url,
			}));

			link.addEventListener('load', () => {
				URL.revokeObjectURL(url);
			});
			
			document.head.appendChild(link);
		}
	};

module.exports = menu => {
	for(let [ name, [ data, metadata, errors ] ] of Object.entries(js)){
		if(metadata){
			if(errors)for(let error of errors)console.error(error);
			else new ChiefUserscript(name, metadata).run(data, site, menu);
		}else{ // legacy idkr, unknown
			// quick fix
			if(data.includes('// ==UserScript==') && site != 'game')continue;
			
			let module = { exports: {} },
				func,
				context = {
					module,
					exports: module.exports,
					console,
				};
			
			try{
				func = eval(`(function(${Object.keys(context)}){${data}//# sourceURL=${name}\n})`);
			}catch(err){
				console.warn(`Error parsing userscript: ${name}\n`, err);
				ipc.console.error(`Error parsing userscript ${name}:\n${err}`);
				break;
			}
			
			// try{...}catch(err){...} doesnt provide: line, column
			
			try{
				func(...Object.values(context));
				
				let userscript = new LegacyUserscript(module.exports);
				
				userscript.run();
			}catch(err){
				console.warn(`Error executing userscript: ${name}\n`, err);
				ipc.console.error(`Error executing userscript ${name}:\n${err}`);
				break;
			}
		}
	}
};

new MutationObserver((mutations, observer) => {
	for(let mutation of mutations){
		for(let node of mutation.addedNodes){
			if(node.nodeName == 'LINK' && new URL(node.href || '/', location).pathname == '/css/main_custom.css'){
				add_css();
				observer.disconnect();
			}
		}
	}
}).observe(document, {
	childList: true,
	subtree: true,
});

/***/ }),

/***/ "./src/Runtime.js":
/*!************************!*\
  !*** ./src/Runtime.js ***!
  \************************/
/***/ ((module) => {

"use strict";

module.exports = _RUNTIME_DATA_;

/***/ }),

/***/ "./src/Site.js":
/*!*********************!*\
  !*** ./src/Site.js ***!
  \*********************/
/***/ ((module) => {

"use strict";


module.exports = {
	'/': 'game',
	'/social.html': 'social',
	'/editor.html': 'editor',
}[location.pathname];

/***/ }),

/***/ "./src/console.js":
/*!************************!*\
  !*** ./src/console.js ***!
  \************************/
/***/ ((__unused_webpack_module, exports) => {

"use strict";


// script executes before devtools listens on logs

var methods = ['log','info','warn','error','debug','trace'],
	initial = {},
	buffer = {},
	tempcall = (type, ...data) => {
		
	};

for(let method of methods){
	initial[method] = console[method].bind(console);
	exports[method] = (...data) => {
		if(!buffer[method])buffer[method] = [];
		buffer[method].push(data);
	};
}

// devtools hooks after

setTimeout(() => {
	for(let method of methods){
		exports[method] = initial[method];
		if(buffer[method])for(let data of buffer[method])exports[method](...data);
	}
	
	buffer = null;
	initial = null;
});

/***/ }),

/***/ "./src/libs/Events.js":
/*!****************************!*\
  !*** ./src/libs/Events.js ***!
  \****************************/
/***/ ((module) => {

"use strict";


class Events {
	static original = Symbol();
	#events = new Map();
	#resolve(event){
		var callbacks = this.#events.get(event);
		
		if(!callbacks){
			callbacks = new Set();
			this.#events.set(event, callbacks);
		}
		
		return callbacks;
	};
	on(event, callback){
		if(typeof callback != 'function')throw new TypeError('Callback is not a function.');
		
		this.#resolve(event).add(callback);
		
		return this;
	}
	once(event, callback){
		var cb = function(...data){
			this.off(event, callback);
			callback.call(this, ...data);
		};
		
		callback[Events.original] = cb;
		
		return this.on(event, cb);
	}
	off(event, callback){
		if(typeof callback != 'function')throw new TypeError('Callback is not a function.');
		
		if(callback[Events.original])callback = callback[Events.original];
		
		var list = this.#resolve(event);
		
		return list.delete(callback);
	}
	emit(event, ...data){
		var set = this.#resolve(event);
		
		if(!set.size){
			if(event == 'error')throw data[0];
			return false;
		}else for(let item of set)try{
			item.call(this, ...data);
		}catch(err){
			this.emit('error', err);
		}
		
		return true;
	}
};

module.exports = Events;

/***/ }),

/***/ "./src/libs/ExtendMenu.js":
/*!********************************!*\
  !*** ./src/libs/ExtendMenu.js ***!
  \********************************/
/***/ ((module, __unused_webpack_exports, __webpack_require__) => {

"use strict";


var utils = __webpack_require__(/*! ../libs/utils */ "./src/libs/utils.js"),
	Events = __webpack_require__(/*! ./Events */ "./src/libs/Events.js"),
	HTMLProxy = __webpack_require__(/*! ./HTMLProxy */ "./src/libs/HTMLProxy.js"),
	Category = __webpack_require__(/*! ./MenuUI/Window/Category */ "./src/libs/MenuUI/Window/Category.js");

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
	async insert(label){
		var array = await utils.wait_for(() => typeof windows == 'object' && windows),
			settings = array[0],
			index = settings.tabs.length,
			get = settings.getSettings;
	
		settings.tabs.push({
			name: label,
			categories: [],
		});
		
		settings.getSettings = () => settings.tabIndex == index ? this.html.get() : get.call(settings);
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
	constructor(){
		super();
	}
};

module.exports = ExtendMenu;

/***/ }),

/***/ "./src/libs/HTMLProxy.js":
/*!*******************************!*\
  !*** ./src/libs/HTMLProxy.js ***!
  \*******************************/
/***/ ((module) => {

"use strict";


class HTMLProxy {
	children = [];
	appendChild(node){
		this.append(node);
		return node;
	}
	append(node){
		this.children.push(node);
	}
	constructor(){
		this.id = 'a-' + Math.random().toString().slice(2);
		
		var children = this.children;
		
		customElements.define(this.id, class extends HTMLElement {
			connectedCallback(){
				for(let node of children)this.parentNode.insertBefore(node, this);
				this.remove();
			}
		});
	}
	get(){
		var html = `<${this.id}></${this.id}>`;
		
		// html += '<!-- ';
		// for(let node of this.children)html += node.outerHTML.replace(/-->/g, '->');
		// html += '-->';
		
		return html;
	}
};

module.exports = HTMLProxy;

/***/ }),

/***/ "./src/libs/Keybind.js":
/*!*****************************!*\
  !*** ./src/libs/Keybind.js ***!
  \*****************************/
/***/ ((module) => {

"use strict";


class Keybind {
	static keybinds = new Set();
	constructor(key, callback){
		this.keys = new Set();
		this.callbacks = new Set();
		Keybind.keybinds.add(this);
		
		if(typeof key == 'string'){
			this.key(key);
			key = callback;
		}
		
		if(typeof key == 'function')this.callback(callback);
	}
	delete(){
		Keybind.keybinds.delete(this);
	}
	set_key(...args){
		return this.keys = new Set(), this.key(...args);
	}
	set_callback(...args){
		return this.callbacks = new Set(), this.key(...args);
	}
	key(...keys){
		for(let key of keys)this.keys.add(key);
		return this;
	}
	callback(...funcs){
		for(let func of funcs)this.callbacks.add(func);
		return this;
	}
};

window.addEventListener('keydown', event => {
	if(event.repeat)return;
	for(let node of [...event.composedPath()])if(node.tagName)for(let part of ['INPUT', 'TEXTAREA'])if(node.tagName.includes(part))return;
	
	//  || keybind.repeat
	for(let keybind of Keybind.keybinds)if((!event.repeat) && keybind.keys.has(event.code)){
		event.preventDefault();
		for(let callback of keybind.callbacks)callback(event);
	}
});

module.exports = Keybind;

/***/ }),

/***/ "./src/libs/MenuUI/Control.js":
/*!************************************!*\
  !*** ./src/libs/MenuUI/Control.js ***!
  \************************************/
/***/ ((module, __unused_webpack_exports, __webpack_require__) => {

"use strict";


var utils = __webpack_require__(/*! ../Utils */ "./src/libs/Utils.js"),
	Events = __webpack_require__(/*! ../Events */ "./src/libs/Events.js");

class Control extends Events {
	constructor(name, data, category){
		super();
		
		this.data = data;
		this.name = name;
		this.category = category;
		this.menu = this.category.tab.window.menu;
		
		this.content = utils.add_ele('div', this.category.content, { className: 'settName' });
		this.label = utils.add_ele('text', this.content);
		
		this.create();
		
		this.menu.emit('control', this);
	}
	label_text(text){
		this.label.nodeValue = text;
	}
	remove(){
		this.content.remove();
	}
	walk(data){
		var state = this.menu.config,
			last_state,
			last_key;
		
		for(let key of data.split('.'))state = (last_state = state)[last_key = key] || {};
		
		return [ last_state, last_key ];
	}
	get value(){
		if(typeof this.data.value == 'function')return this.data.value;
		
		var walked = this.walk(this.data.walk);
		
		return walked[0][walked[1]];
	}
	set value(value){
		var walked = this.walk(this.data.walk);
		
		walked[0][walked[1]] = value;
		
		this.menu.save_config();
		
		this.emit('change', value);
		
		return value;
	}
	create(){}
	interact(){
		console.warn('No defined interaction for', this);
	}
	update(init){
		// MAKE CHANGE EMIT CALLED FROM THE CATEGORY
		if(init)this.emit('change', this.value, true);
		this.label_text(this.name);
	}
	show_content(){
		this.content.style.display = 'block';
	}
	hide_content(){
		this.content.style.display = 'none';
	}
};

class BooleanControl extends Control {
	static id = 'boolean';
	create(){
		this.switch = utils.add_ele('label', this.content, {
			className: 'switch',
			textContent: 'Run',
			style: {
				'margin-left': '10px',
			},
		});
		
		this.input = utils.add_ele('input', this.switch, { type: 'checkbox' });
		
		this.input.addEventListener('change', () => this.value = this.input.checked);
		
		utils.add_ele('span', this.switch, { className: 'slider' });
	}
	update(init){
		super.update(init);
		if(init)this.input.checked = this.value;
		this.label_text(this.name);
	}
}

class SelectControl extends Control {
	static id = 'select';
	create(){
		this.select = utils.add_ele('select', this.content, { className: 'inputGrey2' });
		
		this.select.addEventListener('change', () => this.value = this.select.value);
		
		for(let value in this.data.value)utils.add_ele('option', this.select, {
			value: value,
			textContent: this.data.value[value],
		});
	}
	update(init){
		super.update(init);
		
		if(init)this.select.value = this.value;
	}
};

class DropdownControl extends Control {
	static id = 'dropdown';
	create(){
		this.select = utils.add_ele('select', this.content, { className: 'inputGrey2' });
		
		this.select.addEventListener('change', () => {
			this.key = this.select.value;
			this.value = this.data.value[this.select.value];
		});
		
		for(let key in this.data.value)utils.add_ele('option', this.select, {
			textContent: key,
			value: key,
		});
	}
	update(init){
		super.update(init);
		
		if(init)for(let [ key, value ] of Object.entries(this.data.value)){
			if(value == this.value){
				this.select.value = key;
				this.key = key;
			}
		}
	}
};

class LinkControl extends Control {
	static id = 'link';
	create(){
		this.link = utils.add_ele('a', this.content, {
			href: this.value,
		});
		this.link.append(this.label);
	}
	interact(){
		this.link.click();
	}
};

class LinkFunctionControl extends Control {
	static id = 'linkfunction';
	create(){
		this.link = utils.add_ele('a', this.content, {
			href: '#',
			events: {
				click: () => this.interact(),
			},
		});
		this.link.append(this.label);
	}
	interact(){
		this.value();
	}
};


class FunctionControl extends Control {
	static id = 'function';
	create(){
		utils.add_ele('div', this.content, {
			className: 'settingsBtn',
			textContent: this.data.text || 'Run',
			events: {
				click: () => this.interact(),
			},
		});
	}
	interact(){
		this.value();
	}
};

class KeybindControl extends Control {
	static id = 'keybind';
	create(){
		this.input = utils.add_ele('input', this.content, {
			className: 'inputGrey2',
			placeholder: 'Press a key',
			style: {
				display: 'inline-block',
				width: '220px',
			},
		});
		
		this.input.addEventListener('focus', () => {
			this.input.value = '';
		});
		
		this.input.addEventListener('keydown', event => {
			event.preventDefault();
			this.value = event.code == 'Escape' ? null : event.code;
			this.input.blur();
		});

		this.input.addEventListener('blur', () => {
			this.category.update();
			this.update();
		});
	}
	update(init){
		super.update(init);
		
		this.input.value = utils.string_key(this.value);
	}
};

class TextBoxControl extends Control {
	static id = 'textbox';
	create(){
		this.input = utils.add_ele('input', this.content, {
			className: 'inputGrey2',
			placeholder: this.data.placeholder || '',
			style: {
				display: 'inline-block',
				width: '220px',
			},
		});
		
		this.input.addEventListener('change', () => this.value = this.input.value);
	}
	update(init){
		super.update(init);
		
		if(init)this.input.value = this.value;
	}
};

class SliderControl extends Control {
	static id = 'slider';
	create(){
		var slider = {
			min: this.data.min,
			max: this.data.max,
			step: this.data.step,
		};
		
		this.input = utils.add_ele('input', this.content, {
			className: 'sliderVal',
			type: 'number',
			...slider,
		});
		
		this.slider = utils.add_ele('input', utils.add_ele('div', this.content, {
			className: 'slidecontainer',
			style: { 'margin-top': '-8px' },
		}), {
			className: 'sliderM',
			type: 'range',
			...slider,
		});
		
		this.input.addEventListener('focus', () => (this.input_focused = true, this.interact()));
		this.input.addEventListener('blur', () => (this.input_focused = false, this.interact()));
		
		this.slider.addEventListener('input', () => this.interact(this.value = this.slider.value));
		this.input.addEventListener('input', () => this.interact(this.value = +this.input.value));
	}
	interact(){
		var label = !this.input_focused && this.data.labels && this.data.labels[this.value] || this.value;
		
		this.input.type = typeof label == 'string' ? 'text' : 'number';
		
		this.input.value = label;
		
		this.slider.value = this.value;
	}
	update(init){
		super.update(init);
		
		this.interact();
	}
};

class ColorControl extends Control {
	static id = 'color';
	create(){
		this.input = utils.add_ele('input', this.content, {
			name: 'color',
			type: 'color',
			style: {
				float: 'right',
			},
		});
		
		this.input.addEventListener('change', () => this.value = this.input.value);
	}
	update(init){
		super.update(init);
		
		if(init)this.input.value = this.value;
	}
};

Control.Types = {
	KeybindControl,
	SelectControl,
	DropdownControl,
	BooleanControl,
	FunctionControl,
	LinkControl,
	TextBoxControl,
	SliderControl,
	ColorControl,
	LinkControl,
	LinkFunctionControl,
};

module.exports = Control;

/***/ }),

/***/ "./src/libs/MenuUI/Window/Category.js":
/*!********************************************!*\
  !*** ./src/libs/MenuUI/Window/Category.js ***!
  \********************************************/
/***/ ((module, __unused_webpack_exports, __webpack_require__) => {

"use strict";


var utils = __webpack_require__(/*! ../../Utils */ "./src/libs/Utils.js"),
	Control = __webpack_require__(/*! ../Control */ "./src/libs/MenuUI/Control.js");

class Category {
	constructor(tab, label){
		this.tab = tab;
		
		this.controls = new Set();
		
		if(label){
			this.label = label;
			
			this.header = utils.add_ele('div', this.tab.content, {
				className: 'setHed',
			});
			
			this.header_status = utils.add_ele('span', this.header, { className: 'material-icons plusOrMinus' });
			
			utils.add_ele('text', this.header, { nodeValue: label });
			
			this.header.addEventListener('click', () => this.toggle());
		}
		
		this.content = utils.add_ele('div', this.tab.content, {
			className: 'setBodH',
		});
		
		if(label)this.expand();
	}
	toggle(){
		if(this.collapsed)this.expand();
		else this.collapse();
	}
	collapse(){
		this.collapsed = true;
		this.update();
	}
	expand(){
		this.collapsed = false;
		this.update();
	}
	update(init){
		this.content.style.display = this.collapsed ? 'none' : 'block';
		
		if(this.header){
			this.header.style.display = 'block';
			this.header_status.textContent = 'keyboard_arrow_' + (this.collapsed ? 'right' : 'down');
		}
		
		for(let control of this.controls)control.update(init);
	}
	show(){
		this.expand();
		if(this.header)this.header.style.display = 'block';
	}
	hide(){
		this.content.style.display = 'none';
		if(this.header)this.header.style.display = 'none';
	}
	fix(){
		this.update();
		for(let control of this.controls)control.show_content();
	}
	control(name, data){
		for(let [ cls, type ] of Object.entries(Control.Types))if(type.id == data.type){
			let control = new type(name, data, this);
			
			this.controls.add(control);
			
			return control;
		}
		
		throw new TypeError('Unknown type: ' + data.type);
	}
};

module.exports = Category;

/***/ }),

/***/ "./src/libs/Utils.js":
/*!***************************!*\
  !*** ./src/libs/Utils.js ***!
  \***************************/
/***/ ((module) => {

"use strict";


class Utils {
	static is_host(url, ...hosts){
		return hosts.some(host => url.hostname == host || url.hostname.endsWith('.' + host));
	}
	static round(n, r){
		return Math.round(n * Math.pow(10, r)) / Math.pow(10, r);
	}
	static add_ele(node_name, parent, attributes = {}){
		var crt = this.crt_ele(node_name, attributes);
		
		if(typeof parent == 'function')this.wait_for(parent).then(data => data.append(crt));
		else if(typeof parent == 'object' && parent != null && parent.append)parent.append(crt);
		else throw new Error('Parent is not resolvable to a DOM element');
		
		return crt;
	}
	static crt_ele(node_name, attributes = {}){
		var after = {};
		
		for(let prop in attributes)if(typeof attributes[prop] == 'object' && attributes[prop] != null)after[prop] = attributes[prop], delete attributes[prop];
	
		var node;
		
		if(node_name == 'raw')node = this.crt_ele('div', { innerHTML: attributes.html }).firstChild;
		else if(node_name == 'text')node = document.createTextNode('');
		else node = document.createElement(node_name);
		
		var cls = attributes.className;
		
		if(cls){
			delete attributes.className;
			node.setAttribute('class', cls);
		}
		
		var events = after.events;
		
		if(events){
			delete after.events;
			
			for(let event in events)node.addEventListener(event, events[event]);
		}
		
		Object.assign(node, attributes);
		
		for(let prop in after)Object.assign(node[prop], after[prop]);
		
		return node;
	}
	static wait_for(check, time){
		return new Promise(resolve => {
			var interval,
				run = () => {
					try{
						var result = check();
						
						if(result){
							if(interval)clearInterval(interval);
							resolve(result);
							
							return true;
						}
					}catch(err){console.log(err)}
				};
			
			interval = run() || setInterval(run, time || 50);
		});
	}
	static sanitize(string){
		var node = document.createElement('div');
		
		node.textContent = string;
		
		return node.innerHTML;
	}
	static unsanitize(string){
		var node = document.createElement('div');
		
		node.innerHTML = string;
		
		return node.textContent;
	}
	static node_tree(nodes, parent = document){
		var output = {
				parent: parent,
			},
			match_container = /^\$\s+>?/g,
			match_parent = /^\^\s+>?/g;
		
		for(var label in nodes){
			var value = nodes[label];
			
			if(value instanceof Node)output[label] = value;
			else if(typeof value == 'object')output[label] = this.node_tree(value, output.container);
			else if(match_container.test(nodes[label])){
				if(!output.container){
					console.warn('No container is available, could not access', value);
					continue;
				}
				
				output[label] = output.container.querySelector(nodes[label].replace(match_container, ''));
			}else if(match_parent.test(nodes[label])){
				if(!output.parent){
					console.warn('No parent is available, could not access', value);
					continue;
				}
				
				output[label] = output.parent.querySelector(nodes[label].replace(match_parent, ''));
			}else output[label] = parent.querySelector(nodes[label]);
			
			if(!output[label])console.warn('No node found, could not access', value);
		}
		
		return output;
	}
	static string_key(key){
		return key.replace(/^([A-Z][a-z]+?)([A-Z0-9][a-z]*?)/, (match, type, key) => ['Digit', 'Key'].includes(type) ? key : `${key} ${type}`);
	}
	static clone_obj(obj){
		return JSON.parse(JSON.stringify(obj));
	}
	static assign_deep(target, ...objects){
		for(let ind in objects)for(let key in objects[ind]){
			if(typeof objects[ind][key] == 'object' && objects[ind][key] != null && key in target)this.assign_deep(target[key], objects[ind][key]);
			else if(typeof target == 'object' && target != null)Object.defineProperty(target, key, Object.getOwnPropertyDescriptor(objects[ind], key))
		}
		
		return target;
	}
	static filter_deep(target, match){
		for(let key in target){
			if(!(key in match))delete target[key];
			
			if(typeof match[key] == 'object' && match[key] != null)this.filter_deep(target[key], match[key]);
		}
		
		return target;
	}
	static redirect(name, from, to){
		var proxy = Symbol();
		
		to.addEventListener(name, event => {
			if(event[proxy])return;
		});
		
		from.addEventListener(name, event => to.dispatchEvent(Object.assign(new(event.constructor)(name, event), {
			[proxy]: true,
			stopImmediatePropagation: event.stopImmediatePropagation.bind(event),
			preventDefault: event.preventDefault.bind(event),
		})));
	}
	static promise(){
		var temp,
			promise = new Promise((resolve, reject) => temp = { resolve, reject });
		
		Object.assign(promise, temp);
		
		promise.resolve_in = (time = 0, data) => setTimeout(() => promise.resolve(data), time);
		
		return promise;
	}
	static rtn(number, unit){
		return (number / unit).toFixed() * unit;
	}
};

module.exports = Utils;

/***/ }),

/***/ "./src/libs/utils.js":
/*!***************************!*\
  !*** ./src/libs/utils.js ***!
  \***************************/
/***/ ((module) => {

"use strict";


class Utils {
	static is_host(url, ...hosts){
		return hosts.some(host => url.hostname == host || url.hostname.endsWith('.' + host));
	}
	static round(n, r){
		return Math.round(n * Math.pow(10, r)) / Math.pow(10, r);
	}
	static add_ele(node_name, parent, attributes = {}){
		var crt = this.crt_ele(node_name, attributes);
		
		if(typeof parent == 'function')this.wait_for(parent).then(data => data.append(crt));
		else if(typeof parent == 'object' && parent != null && parent.append)parent.append(crt);
		else throw new Error('Parent is not resolvable to a DOM element');
		
		return crt;
	}
	static crt_ele(node_name, attributes = {}){
		var after = {};
		
		for(let prop in attributes)if(typeof attributes[prop] == 'object' && attributes[prop] != null)after[prop] = attributes[prop], delete attributes[prop];
	
		var node;
		
		if(node_name == 'raw')node = this.crt_ele('div', { innerHTML: attributes.html }).firstChild;
		else if(node_name == 'text')node = document.createTextNode('');
		else node = document.createElement(node_name);
		
		var cls = attributes.className;
		
		if(cls){
			delete attributes.className;
			node.setAttribute('class', cls);
		}
		
		var events = after.events;
		
		if(events){
			delete after.events;
			
			for(let event in events)node.addEventListener(event, events[event]);
		}
		
		Object.assign(node, attributes);
		
		for(let prop in after)Object.assign(node[prop], after[prop]);
		
		return node;
	}
	static wait_for(check, time){
		return new Promise(resolve => {
			var interval,
				run = () => {
					try{
						var result = check();
						
						if(result){
							if(interval)clearInterval(interval);
							resolve(result);
							
							return true;
						}
					}catch(err){console.log(err)}
				};
			
			interval = run() || setInterval(run, time || 50);
		});
	}
	static sanitize(string){
		var node = document.createElement('div');
		
		node.textContent = string;
		
		return node.innerHTML;
	}
	static unsanitize(string){
		var node = document.createElement('div');
		
		node.innerHTML = string;
		
		return node.textContent;
	}
	static node_tree(nodes, parent = document){
		var output = {
				parent: parent,
			},
			match_container = /^\$\s+>?/g,
			match_parent = /^\^\s+>?/g;
		
		for(var label in nodes){
			var value = nodes[label];
			
			if(value instanceof Node)output[label] = value;
			else if(typeof value == 'object')output[label] = this.node_tree(value, output.container);
			else if(match_container.test(nodes[label])){
				if(!output.container){
					console.warn('No container is available, could not access', value);
					continue;
				}
				
				output[label] = output.container.querySelector(nodes[label].replace(match_container, ''));
			}else if(match_parent.test(nodes[label])){
				if(!output.parent){
					console.warn('No parent is available, could not access', value);
					continue;
				}
				
				output[label] = output.parent.querySelector(nodes[label].replace(match_parent, ''));
			}else output[label] = parent.querySelector(nodes[label]);
			
			if(!output[label])console.warn('No node found, could not access', value);
		}
		
		return output;
	}
	static string_key(key){
		return key.replace(/^([A-Z][a-z]+?)([A-Z0-9][a-z]*?)/, (match, type, key) => ['Digit', 'Key'].includes(type) ? key : `${key} ${type}`);
	}
	static clone_obj(obj){
		return JSON.parse(JSON.stringify(obj));
	}
	static assign_deep(target, ...objects){
		for(let ind in objects)for(let key in objects[ind]){
			if(typeof objects[ind][key] == 'object' && objects[ind][key] != null && key in target)this.assign_deep(target[key], objects[ind][key]);
			else if(typeof target == 'object' && target != null)Object.defineProperty(target, key, Object.getOwnPropertyDescriptor(objects[ind], key))
		}
		
		return target;
	}
	static filter_deep(target, match){
		for(let key in target){
			if(!(key in match))delete target[key];
			
			if(typeof match[key] == 'object' && match[key] != null)this.filter_deep(target[key], match[key]);
		}
		
		return target;
	}
	static redirect(name, from, to){
		var proxy = Symbol();
		
		to.addEventListener(name, event => {
			if(event[proxy])return;
		});
		
		from.addEventListener(name, event => to.dispatchEvent(Object.assign(new(event.constructor)(name, event), {
			[proxy]: true,
			stopImmediatePropagation: event.stopImmediatePropagation.bind(event),
			preventDefault: event.preventDefault.bind(event),
		})));
	}
	static promise(){
		var temp,
			promise = new Promise((resolve, reject) => temp = { resolve, reject });
		
		Object.assign(promise, temp);
		
		promise.resolve_in = (time = 0, data) => setTimeout(() => promise.resolve(data), time);
		
		return promise;
	}
	static rtn(number, unit){
		return (number / unit).toFixed() * unit;
	}
};

module.exports = Utils;

/***/ }),

/***/ "../Resources/Config.json":
/*!********************************!*\
  !*** ../Resources/Config.json ***!
  \********************************/
/***/ ((module) => {

"use strict";
module.exports = JSON.parse('{"game":{"seek":{"F4":true,"mode":"any"}},"client":{"devtools":false},"render":{"uncap_fps":false,"fullscreen":false,"angle":"default","color":"default"},"rpc":{"enabled":true,"name":false},"window":{"meta":{"replace":false,"title":"Krunker","icon":"Krunker.ico"}},"userscripts":{}}');

/***/ })

/******/ 	});
/************************************************************************/
/******/ 	// The module cache
/******/ 	var __webpack_module_cache__ = {};
/******/ 	
/******/ 	// The require function
/******/ 	function __webpack_require__(moduleId) {
/******/ 		// Check if module is in cache
/******/ 		var cachedModule = __webpack_module_cache__[moduleId];
/******/ 		if (cachedModule !== undefined) {
/******/ 			return cachedModule.exports;
/******/ 		}
/******/ 		// Create a new module (and put it into the cache)
/******/ 		var module = __webpack_module_cache__[moduleId] = {
/******/ 			// no module.id needed
/******/ 			// no module.loaded needed
/******/ 			exports: {}
/******/ 		};
/******/ 	
/******/ 		// Execute the module function
/******/ 		__webpack_modules__[moduleId](module, module.exports, __webpack_require__);
/******/ 	
/******/ 		// Return the exports of the module
/******/ 		return module.exports;
/******/ 	}
/******/ 	
/************************************************************************/
var __webpack_exports__ = {};
// This entry need to be wrapped in an IIFE because it need to be in strict mode.
(() => {
"use strict";
/*!**********************!*\
  !*** ./src/index.js ***!
  \**********************/


try{
	window.onbeforeunload = () => {};
	Object.defineProperty(window, 'onbeforeunload', { writable: false, value(){} })
}catch(err){
	
}

__webpack_require__(/*! ./FilePicker */ "./src/FilePicker.js");

var ExtendMenu = __webpack_require__(/*! ./libs/ExtendMenu */ "./src/libs/ExtendMenu.js"),
	Keybind = __webpack_require__(/*! ./libs/Keybind */ "./src/libs/Keybind.js"),
	utils = __webpack_require__(/*! ./libs/Utils */ "./src/libs/Utils.js"),
	RPC = __webpack_require__(/*! ./RPC */ "./src/RPC.js"),
	console = __webpack_require__(/*! ./console */ "./src/console.js"),
	{ ipc, IM } = __webpack_require__(/*! ./IPC */ "./src/IPC.js"),
	{ config: runtime_config, js } = __webpack_require__(/*! ./Runtime */ "./src/Runtime.js"),
	site = __webpack_require__(/*! ./Site */ "./src/Site.js"),
	run_resources = __webpack_require__(/*! ./Resources */ "./src/Resources.js");

class Menu extends ExtendMenu {
	rpc = new RPC();
	save_config(){
		ipc.send(IM.save_config, this.config);
	}
	config = runtime_config;
	default_config = __webpack_require__(/*! ../../Resources/Config.json */ "../Resources/Config.json");
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
		
		Client.control('DevTools [F10]', {
			type: 'boolean',
			walk: 'client.devtools',
		});
		
		var Folder = this.category('Folders');
		
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
		
		Render.control('Fullscreen', {
			type: 'boolean',
			walk: 'render.fullscreen',
		}).on('change', (value, init) => !init && ipc.send(IM.fullscreen));
		
		Render.control('Uncap FPS', {
			type: 'boolean',
			walk: 'render.uncap_fps',
		}).on('change', (value, init) => !init && ipc.send(IM.relaunch_webview));
		
		/*Render.control('VSync', {
			type: 'boolean',
			walk: 'render.vsync',
		}).on('change', (value, init) => !init && ipc.send(IM.relaunch_webview));*/
		
		Render.control('Angle backend', {
			type: 'dropdown',
			walk: 'render.angle',
			value: {
				Default: 'default',
				'Direct3D 11 on 12': 'd3d11on12',
				'Direct3D 11': 'd3d11',
				'Direct3D 9': 'd3d9',
				'OpenGL': 'gl',
			},
		}).on('change', (value, init) => !init && ipc.send(IM.relaunch_webview));
		
		Render.control('Color profile', {
			type: 'dropdown',
			walk: 'render.color',
			value: {
				Default: 'default',
				'SRGB': 'srgb',
				'RGB': 'generic-rgb',
			},
		}).on('change', (value, init) => !init && ipc.send(IM.relaunch_webview));
		
		var Game = this.category('Game');
		
		Game.control('Seek new Lobby [F4]', {
			type: 'boolean',
			walk: 'game.seek.F4',
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
		
		this.insert('Client');
	}
	update(){
		for(let category of this.categories)category.update(true);
	}
};

new Keybind('F4', event => {
	if(event.altKey)ipc.send(IM.close_window);
	else ipc.send(IM.seek_game);
});

new Keybind('F11', () => {
	ipc.send(IM.toggle_fullscreen);
});

new Keybind('F10', event => {
	ipc.send(IM.open_devtools);
});


if(site == 'game'){
	__webpack_require__(/*! ./Fixes */ "./src/Fixes.js");
	let menu = new Menu();
	run_resources(menu);
	menu.update();
	
	ipc.on(IM.update_menu, config => {
		menu.config = config;
		menu.update();
	});
}else run_resources();

new Keybind('F5', event => ipc.send(IM.reload_window));
})();

/******/ })()
;
//# sourceMappingURL=Webpack.js.map