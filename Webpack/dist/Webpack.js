/******/ (() => { // webpackBootstrap
/******/ 	"use strict";
/******/ 	var __webpack_modules__ = ({

/***/ "./src/Consts.js":
/*!***********************!*\
  !*** ./src/Consts.js ***!
  \***********************/
/***/ ((__unused_webpack_module, exports, __webpack_require__) => {



var Utils = __webpack_require__(/*! ./libs/Utils */ "./src/libs/Utils.js"),
	utils = new Utils();

exports.utils = utils;

exports.meta = {
	github: 'https://github.com/y9x/',
	discord: 'https://y9x.github.io/discord/',
	forum: 'https://forum.sys32.dev/',
};

exports.site_location = {
	'/': 'game',
	'/social.html': 'social',
	'/editor.html': 'editor',
}[location.pathname];

/***/ }),

/***/ "./src/FastLoad.js":
/*!*************************!*\
  !*** ./src/FastLoad.js ***!
  \*************************/
/***/ ((__unused_webpack_module, __unused_webpack_exports, __webpack_require__) => {



var { config, js } = __webpack_require__(/*! ./Runtime */ "./src/Runtime.js"),
	{ meta } = __webpack_require__(/*! ./consts */ "./src/consts.js"),
	Loader = __webpack_require__(/*! ./libs/Loader */ "./src/libs/Loader.js");

if(config.game.fast_load && !Object.keys(js).length){
	let loader = new Loader();
	
	loader.observe();
	
	loader.license(meta);
	
	loader.load({}, {});
}

/***/ }),

/***/ "./src/Fixes.js":
/*!**********************!*\
  !*** ./src/Fixes.js ***!
  \**********************/
/***/ ((__unused_webpack_module, __unused_webpack_exports, __webpack_require__) => {



var	{ utils } = __webpack_require__(/*! ./Consts */ "./src/Consts.js");

(async () => {
	if(localStorage.kro_setngss_scaleUI == void[])
		localStorage.kro_setngss_scaleUI = 1;
	
	var ui_base = await utils.wait_for(() => document.querySelector('#uiBase')),
		MIN_WIDTH = 1700,
		MIN_HEIGHT = 900;
		
	if(localStorage.kro_setngss_uiScaling === 'false')return;
	
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
	
	ui_base.style.transform = 'scale(' + style.transform.toFixed(3) + ')';
	ui_base.style.width = style.width.toFixed(3) + 'px';
	ui_base.style.height = style.height.toFixed(3) + 'px';
})();

/***/ }),

/***/ "./src/IPC.js":
/*!********************!*\
  !*** ./src/IPC.js ***!
  \********************/
/***/ ((module, __unused_webpack_exports, __webpack_require__) => {



var Events = __webpack_require__(/*! ./libs/Events */ "./src/libs/Events.js");

class IPC extends Events {
	constructor(){
		super();
	}
	send(event, ...data){
		chrome.webview.postMessage(JSON.stringify([ event, ...data ]));
		return true;
	}
};

module.exports = new IPC();

/***/ }),

/***/ "./src/IPCConsole.js":
/*!***************************!*\
  !*** ./src/IPCConsole.js ***!
  \***************************/
/***/ ((module) => {



class IPCConsole {
	constructor(ipc, prefix){
		this.ipc = ipc;
		this.prefix = [];
		if(typeof prefix == 'string')this.prefix.push(prefix);
	}
	log(...args){
		this.ipc.send('log', 'info', this.prefix.concat(args).join(' '));
	}
	info(...args){
		this.ipc.send('log', 'info', this.prefix.concat(args).join(' '));
	}
	warn(...args){
		this.ipc.send('log', 'warn', this.prefix.concat(args).join(' '));
	}
	error(...args){
		this.ipc.send('log', 'error', this.prefix.concat(args).join(' '));
	}
};

module.exports = IPCConsole;

/***/ }),

/***/ "./src/Resources.js":
/*!**************************!*\
  !*** ./src/Resources.js ***!
  \**************************/
/***/ ((__unused_webpack_module, __unused_webpack_exports, __webpack_require__) => {



var { site_location, utils } = __webpack_require__(/*! ./Consts */ "./src/Consts.js"),
	{ css, js } = __webpack_require__(/*! ./Runtime */ "./src/Runtime.js"),
	ipc = __webpack_require__(/*! ./IPC */ "./src/IPC.js"),
	IPCConsole = __webpack_require__(/*! ./IPCConsole */ "./src/IPCConsole.js"),
	Userscript = __webpack_require__(/*! ./Userscript */ "./src/Userscript.js");

// wait for krunker css
// utils.wait_for(() => document.querySelector(`link[href*='/css/main_custom.css']`))

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

for(let [ name, data ] of Object.entries(js)){
	// quick fix
	if(data.includes('// ==UserScript==') && site_location != 'game')continue;
	
	let module = { exports: {} },
		func,
		context = {
			module,
			exports: module.exports,
			console: new IPCConsole(ipc, name + ':'),
		};
	
	try{
		func = eval(`(function(${Object.keys(context)}){${data}//# sourceURL=${name}\n})`);
	}catch(err){
		ipc.send('log', 'error', `Error parsing UserScript ${name}:\n${err}`);
	}
	
	
	try{
		func(...Object.values(context));
		
		let userscript = new Userscript(module.exports);
		
		userscript.run();
	}catch(err){
		ipc.send('log', 'warn', `Error executing UserScript ${name}:\n${err}`);
	}
}

/***/ }),

/***/ "./src/Runtime.js":
/*!************************!*\
  !*** ./src/Runtime.js ***!
  \************************/
/***/ ((module) => {


module.exports = _RUNTIME_DATA_;

/***/ }),

/***/ "./src/Userscript.js":
/*!***************************!*\
  !*** ./src/Userscript.js ***!
  \***************************/
/***/ ((module, __unused_webpack_exports, __webpack_require__) => {



// Legacy IDKR userscript

var { site_location } = __webpack_require__(/*! ./Consts */ "./src/Consts.js");

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
		if(!this.locations.includes('all') && !this.locations.includes(site_location))return false;
		
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

/***/ "./src/consts.js":
/*!***********************!*\
  !*** ./src/consts.js ***!
  \***********************/
/***/ ((__unused_webpack_module, exports, __webpack_require__) => {



var Utils = __webpack_require__(/*! ./libs/Utils */ "./src/libs/Utils.js"),
	utils = new Utils();

exports.utils = utils;

exports.meta = {
	github: 'https://github.com/y9x/',
	discord: 'https://y9x.github.io/discord/',
	forum: 'https://forum.sys32.dev/',
};

exports.site_location = {
	'/': 'game',
	'/social.html': 'social',
	'/editor.html': 'editor',
}[location.pathname];

/***/ }),

/***/ "./src/libs/Events.js":
/*!****************************!*\
  !*** ./src/libs/Events.js ***!
  \****************************/
/***/ ((module) => {



class Events {
	static original = Symbol();
	#events = new Map();
	#resolve(target, event){
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

/***/ "./src/libs/HTMLProxy.js":
/*!*******************************!*\
  !*** ./src/libs/HTMLProxy.js ***!
  \*******************************/
/***/ ((module) => {



var log = console.log;

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
				
				// log(node.children[this.id], this.id, node.children, node);
				// this.replaceWith(node.children[this.id]);
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

/***/ "./src/libs/Loader.js":
/*!****************************!*\
  !*** ./src/libs/Loader.js ***!
  \****************************/
/***/ ((module, __unused_webpack_exports, __webpack_require__) => {



var Utils = __webpack_require__(/*! ./Utils */ "./src/libs/Utils.js"),
	Request = __webpack_require__(/*! ./Request */ "./src/libs/Request.js"),
	Events = __webpack_require__(/*! ./Events */ "./src/libs/Events.js"),
	utils = new Utils();

class Loader extends Events {
	gconsts = {
		playerHeight: 11,
		cameraHeight: 1.5,
		headScale: 2,
		armScale: 1.3,
		armInset: 0.1,
		chestWidth: 2.6,
		hitBoxPad: 1,
		crouchDst: 3,
		recoilMlt: 0.3,
		nameOffset: 0.6,
		nameOffsetHat: 0.8,
	};
	api =  false ? 0 : 'https://api.sys32.dev/';
	matchmaker = 'https://matchmaker.krunker.io/';
	badge = '[GameLoader]';
		// outcome of above maps
	vars = {};
	context = {
		key: '_' + Math.random().toString().substr(2),
	};
	has_instruct = this.has_instruct.bind(this);
	stacks = new Set();
	api_v2 = new URL('v2/', this.api);
	
	meta = utils.promise();
	patches = new Map();
	variables = new Map();
	log(...text){
		console.log(this.badge, ...text);
	}
	var(label, regex, index){
		return this.variables.set(label, [ regex, index ]), this;
	}
	patch(label, regex, replacement){
		return this.patches.set(label, [ regex, replacement ]), this;
	}
	observe(){
		this.loadp = new Promise(resolve => new MutationObserver((muts, observer) => muts.forEach(mut => [...mut.addedNodes].forEach(node => {
			if(node.tagName == 'DIV' && node.id == 'instructionHolder'){
				this.instruction_holder = node;
				
				new MutationObserver(() => setTimeout(() => this.emit('instruct', this.has_instruct), 200)).observe(this.instruction_holder, {
					attributes: true,
					attributeFilter: [ 'style' ],
				});
			}
			
			if(node.tagName == 'SCRIPT' && node.textContent.includes('Yendis Entertainment')){
				node.textContent = '';
				resolve();
			}
		}))).observe(document, { childList: true, subtree: true }));
	}
	has_instruct(...test){
		if(!this.instruction_holder)return false
		var instruction = this.instruction_holder.textContent.trim().toLowerCase();
		for(let string of test)if(instruction.includes(test))return true;
		return false;
	}
	async report_error(where, err){
		if(typeof err != 'object')return;
		
		var body = {
			name: err.name,
			message: err.message,
			stack: err.stack,
			where: where,
		};
		
		if(this.stacks.has(err.stack))return;
		
		console.error('Where:', where, '\nUncaught', err);
		
		this.stacks.add(err.stack);
		
		await Request({
			target: this.api_v2,
			endpoint: 'error',
			data: body,
		});
	}
	async show_error(title, message){
		await this.load;
		
		var holder = document.querySelector('#instructionHolder'),
			instructions = document.querySelector('#instructions');
		
		holder.style.display = 'block';
		holder.style.pointerEvents = 'all';
		
		instructions.innerHTML = `<div style='color:#FFF9'>${title}</div><div style='margin-top:10px;font-size:20px;color:#FFF6'>${message}</div>`;
	}
	async token(){
		await this.meta;
		
		return await Request({
			target: this.api_v2,
			endpoint: 'token',
			data: await Request({
				target: this.matchmaker,
				endpoint: 'generate-token',
				headers: {
					'client-key': this.meta.key,
				},
				result: 'json',
			}),
			result: 'json',
		});
	}
	apply_patches(source){
		var missing;
		
		for(var [ label, [ regex, index ] ] of this.variables){
			var value = (source.match(regex) || 0)[index];
			
			if(value)this.vars[label] = value;
			else (missing || (missing = {}))[label] = [ regex, index ];
		}
		
		console.log('Game Variables:');
		console.table(this.vars);
		
		if(missing){
			console.log('Missing:');
			console.table(missing);
		}
		
		for(var [ label, [ input, replacement ] ] of this.patches){
			if(!source.match(input))console.error('Could not patch', label);
			
			source = source.replace(input, replacement);
		}
		
		return source;
	}
	async license(input_meta){
		var meta = await Request({
			target: this.api_v2,
			endpoint: 'meta',
			data: {
				...input_meta,
				needs_key: true,
			},
			method: 'POST',
			result: 'json',
		});
		
		if(meta.error){
			utils.add_ele('style', document.documentElement, {
				textContent: '#initLoader,#instructionsUpdate{display:none!IMPORTANT}',
			});
			
			this.show_error(meta.error.title, meta.error.message);
			this.meta.reject();
		}else this.meta.resolve(this.meta = meta);
	}
	async source(){
		await this.meta;
		
		return await Request({
			target: this.api_v2,
			endpoint: 'source',
			query: {
				build: this.meta.build,
			},
			result: 'text',
			cache: true,
		});
	}
	async load(add_args = {}, add_context = {}){
		var args = {
				...add_args,
				[this.context.key]: this.context,
				WP_fetchMMToken: this.token(),
			},
			source = this.apply_patches(await this.source());
		
		Object.assign(this.context, add_context);
		
		try{
			await this.loadp;
			new Function(...Object.keys(args), source)(...Object.values(args));
		}catch(err){
			this.report_error('loading', err);
			this.show_error(err.message, `Post a screenshot of this error on <a href='https://forum.sys32.dev/'>the forums</a> or <a href='/'>click here</a> to try again.`);
		}
	}
};

module.exports = Loader;

/***/ }),

/***/ "./src/libs/MenuUI/Control.js":
/*!************************************!*\
  !*** ./src/libs/MenuUI/Control.js ***!
  \************************************/
/***/ ((module, __unused_webpack_exports, __webpack_require__) => {



var { utils } = __webpack_require__(/*! ./consts */ "./src/libs/MenuUI/consts.js"),
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
		
		data.split('.').forEach(key => state = ((last_state = state)[last_key = key] || {}));
		
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



var { utils } = __webpack_require__(/*! ../consts */ "./src/libs/MenuUI/consts.js"),
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

/***/ "./src/libs/MenuUI/consts.js":
/*!***********************************!*\
  !*** ./src/libs/MenuUI/consts.js ***!
  \***********************************/
/***/ ((__unused_webpack_module, exports, __webpack_require__) => {



var Utils = __webpack_require__(/*! ../utils */ "./src/libs/utils.js"),
	utils = new Utils();

exports.utils = utils;

exports.tick = node => node.addEventListener('mouseenter', () => {
	try{
		playTick();
	}catch(err){}
});

exports.select = node => node.addEventListener('click', () => {
	try{
		SOUND.play('select_0', 0.1);
	}catch(err){}
});

/***/ }),

/***/ "./src/libs/Request.js":
/*!*****************************!*\
  !*** ./src/libs/Request.js ***!
  \*****************************/
/***/ ((module) => {



var is_obj = data => typeof data == 'object' && data != null,
	is_url = data => typeof data == 'string' || data instanceof Location || data instanceof URL,
	headers_obj = headers => {
		if(!is_obj(headers))return {};
		else if(headers instanceof Headers){
			let out = {};
			
			for(let [ key, value ] of headers)out[key] = value;
			
			return out;
		}else return headers;
	};

var request = input => {
	if(!is_obj(input))throw new TypeError('Input must be an object');
	
	var opts = {
			cache: 'no-cache',
			headers: headers_obj(input.headers),
		},
		url = request.resolve(input);
	
	switch(input.cache){
		case true:
			opts.cache = 'force-cache';
			break;
		case'query':	
			url.search += '?' + Date.now();
			break;
	}
	if(input.cache == true)opts.cache = 'force-cache';
	
	if(is_obj(input.data)){
		opts.method = 'POST';
		opts.body = JSON.stringify(input.data);
		opts.headers['content-type'] = 'application/json';
	}
	
	if(typeof input.method == 'string')opts.method = input.method;
	
	if(input.sync){
		opts.xhr = true;
		opts.synchronous = true;
	}
	
	var result = ['text', 'json', 'arrayBuffer'].includes(input.result) ? input.result : 'text';
	
	return (opts.xhr ? request.fetch_xhr : window.fetch.bind(window))(url, opts).then(res => res[result]());
};

// request.fetch = window.fetch.bind(window);

request.fetch_xhr = (url, opts = {}) => {
	if(!is_url(url))throw new TypeError('url param is not resolvable');
	
	var url = new URL(url, location).href,
		method = typeof opts.method == 'string' ? opts.method : 'GET';
	
	// if(opts.cache == 'no-cache')url += '?' + Date.now();
	
	var req = new XMLHttpRequest();
	
	req.open(method, url, !opts.synchronous);
	
	return new Promise((resolve, reject) => {
		req.addEventListener('load', () => resolve({
			async text(){
				return req.responseText;
			},
			async json(){
				return JSON.parse(req.responseText);
			},
			headers: new Headers(),
		}));
		
		req.addEventListener('error', event => reject(event.error));
		
		req.send(opts.body);
	});
};

request.resolve = input => {
	if(!is_url(input.target))throw new TypeError('Target must be specified');
	
	var url = new URL(input.target);
	
	if(is_url(input.endpoint))url = new URL(input.endpoint, url);
	
	if(typeof input.query == 'object' && input.query != null)url.search = '?' + new URLSearchParams(Object.entries(input.query));
	
	return url;
};

module.exports = request;

/***/ }),

/***/ "./src/libs/Utils.js":
/*!***************************!*\
  !*** ./src/libs/Utils.js ***!
  \***************************/
/***/ ((module) => {



class Utils {
	is_host(url, ...hosts){
		return hosts.some(host => url.hostname == host || url.hostname.endsWith('.' + host));
	}
	round(n, r){
		return Math.round(n * Math.pow(10, r)) / Math.pow(10, r);
	}
	add_ele(node_name, parent, attributes = {}){
		var crt = this.crt_ele(node_name, attributes);
		
		if(typeof parent == 'function')this.wait_for(parent).then(data => data.append(crt));
		else if(typeof parent == 'object' && parent != null && parent.append)parent.append(crt);
		else throw new Error('Parent is not resolvable to a DOM element');
		
		return crt;
	}
	crt_ele(node_name, attributes = {}){
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
	wait_for(check, time){
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
	sanitize(string){
		var node = document.createElement('div');
		
		node.textContent = string;
		
		return node.innerHTML;
	}
	unsanitize(string){
		var node = document.createElement('div');
		
		node.innerHTML = string;
		
		return node.textContent;
	}
	node_tree(nodes, parent = document){
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
	string_key(key){
		return key.replace(/^([A-Z][a-z]+?)([A-Z0-9][a-z]*?)/, (match, type, key) => ['Digit', 'Key'].includes(type) ? key : `${key} ${type}`);
	}
	clone_obj(obj){
		return JSON.parse(JSON.stringify(obj));
	}
	assign_deep(target, ...objects){
		for(let ind in objects)for(let key in objects[ind]){
			if(typeof objects[ind][key] == 'object' && objects[ind][key] != null && key in target)this.assign_deep(target[key], objects[ind][key]);
			else if(typeof target == 'object' && target != null)Object.defineProperty(target, key, Object.getOwnPropertyDescriptor(objects[ind], key))
		}
		
		return target;
	}
	filter_deep(target, match){
		for(let key in target){
			if(!(key in match))delete target[key];
			
			if(typeof match[key] == 'object' && match[key] != null)this.filter_deep(target[key], match[key]);
		}
		
		return target;
	}
	redirect(name, from, to){
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
	promise(){
		var temp,
			promise = new Promise((resolve, reject) => temp = { resolve, reject });
		
		Object.assign(promise, temp);
		
		promise.resolve_in = (time = 0, data) => setTimeout(() => promise.resolve(data), time);
		
		return promise;
	}
	rtn(number, unit){
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



class Utils {
	is_host(url, ...hosts){
		return hosts.some(host => url.hostname == host || url.hostname.endsWith('.' + host));
	}
	round(n, r){
		return Math.round(n * Math.pow(10, r)) / Math.pow(10, r);
	}
	add_ele(node_name, parent, attributes = {}){
		var crt = this.crt_ele(node_name, attributes);
		
		if(typeof parent == 'function')this.wait_for(parent).then(data => data.append(crt));
		else if(typeof parent == 'object' && parent != null && parent.append)parent.append(crt);
		else throw new Error('Parent is not resolvable to a DOM element');
		
		return crt;
	}
	crt_ele(node_name, attributes = {}){
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
	wait_for(check, time){
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
	sanitize(string){
		var node = document.createElement('div');
		
		node.textContent = string;
		
		return node.innerHTML;
	}
	unsanitize(string){
		var node = document.createElement('div');
		
		node.innerHTML = string;
		
		return node.textContent;
	}
	node_tree(nodes, parent = document){
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
	string_key(key){
		return key.replace(/^([A-Z][a-z]+?)([A-Z0-9][a-z]*?)/, (match, type, key) => ['Digit', 'Key'].includes(type) ? key : `${key} ${type}`);
	}
	clone_obj(obj){
		return JSON.parse(JSON.stringify(obj));
	}
	assign_deep(target, ...objects){
		for(let ind in objects)for(let key in objects[ind]){
			if(typeof objects[ind][key] == 'object' && objects[ind][key] != null && key in target)this.assign_deep(target[key], objects[ind][key]);
			else if(typeof target == 'object' && target != null)Object.defineProperty(target, key, Object.getOwnPropertyDescriptor(objects[ind], key))
		}
		
		return target;
	}
	filter_deep(target, match){
		for(let key in target){
			if(!(key in match))delete target[key];
			
			if(typeof match[key] == 'object' && match[key] != null)this.filter_deep(target[key], match[key]);
		}
		
		return target;
	}
	redirect(name, from, to){
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
	promise(){
		var temp,
			promise = new Promise((resolve, reject) => temp = { resolve, reject });
		
		Object.assign(promise, temp);
		
		promise.resolve_in = (time = 0, data) => setTimeout(() => promise.resolve(data), time);
		
		return promise;
	}
	rtn(number, unit){
		return (number / unit).toFixed() * unit;
	}
};

module.exports = Utils;


/***/ }),

/***/ "../Client/Config.json":
/*!*****************************!*\
  !*** ../Client/Config.json ***!
  \*****************************/
/***/ ((module) => {

module.exports = JSON.parse('{"game":{"fast_load":true,"f4_seek":true},"client":{"uncap_fps":false,"fullscreen":false},"window":{"meta":{"replace":false,"title":"Client++","icon":""}}}');

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
// This entry need to be wrapped in an IIFE because it need to be isolated against other modules in the chunk.
(() => {
/*!**********************!*\
  !*** ./src/index.js ***!
  \**********************/


window.onbeforeunload = () => {};
Object.defineProperty(window, 'onbeforeunload', { writable: false, value(){} })

__webpack_require__(/*! ./Fixes */ "./src/Fixes.js");
__webpack_require__(/*! ./Resources */ "./src/Resources.js");
__webpack_require__(/*! ./FastLoad */ "./src/FastLoad.js");

var HTMLProxy = __webpack_require__(/*! ./libs/HTMLProxy */ "./src/libs/HTMLProxy.js"),
	Category = __webpack_require__(/*! ./libs/MenuUI/Window/Category */ "./src/libs/MenuUI/Window/Category.js"),
	Control = __webpack_require__(/*! ./libs/MenuUI/Control */ "./src/libs/MenuUI/Control.js"),
	Events = __webpack_require__(/*! ./libs/Events */ "./src/libs/Events.js"),
	Keybind = __webpack_require__(/*! ./libs/Keybind */ "./src/libs/Keybind.js"),
	ipc = __webpack_require__(/*! ./IPC */ "./src/IPC.js"),
	{ config: runtime_config, js } = __webpack_require__(/*! ./Runtime */ "./src/Runtime.js"),
	{ site_location, utils, meta } = __webpack_require__(/*! ./Consts */ "./src/Consts.js");

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
	default_config = __webpack_require__(/*! ../../Client/Config.json */ "../Client/Config.json");
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
		if(!Object.keys(js).length)Game.control('Fast Loading', {
			type: 'boolean',
			walk: 'game.fast_load',
		});
		
		Game.control('Seek new Lobby [F4]', {
			type: 'boolean',
			walk: 'game.f4_seek',
		});
		
		new Keybind('F4', event => {
			if(event.altKey)ipc.send('close window');
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
		if(site_location == 'game'){
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
	}
};

new Menu();
})();

/******/ })()
;
//# sourceMappingURL=Webpack.js.map