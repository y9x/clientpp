/******/ (() => { // webpackBootstrap
/******/ 	"use strict";
/******/ 	var __webpack_modules__ = ({

/***/ "./src/Events.js":
/*!***********************!*\
  !*** ./src/Events.js ***!
  \***********************/
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

/***/ "./src/FixLoad.js":
/*!************************!*\
  !*** ./src/FixLoad.js ***!
  \************************/
/***/ ((__unused_webpack_module, __unused_webpack_exports, __webpack_require__) => {



var Utils = __webpack_require__(/*! ./Utils */ "./src/Utils.js"),
	utils = new Utils();

(async () => {	
	var ui_base = await utils.wait_for(() => document.querySelector('#uiBase')),
		MIN_WIDTH = 1700,
		MIN_HEIGHT = 900;
		
	if(localStorage?.kro_setngss_uiScaling === 'false')return;
	
	var ls = localStorage.kro_setngss_scaleUI,
		scale_ui = ls != void[] ? ls : 0.7;
	
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

/***/ "./src/HTMLProxy.js":
/*!**************************!*\
  !*** ./src/HTMLProxy.js ***!
  \**************************/
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

/***/ "./src/IPC.js":
/*!********************!*\
  !*** ./src/IPC.js ***!
  \********************/
/***/ ((module, __unused_webpack_exports, __webpack_require__) => {



var Events = __webpack_require__(/*! ./Events */ "./src/Events.js");

class IPC extends Events {
	#send
	constructor(send){
		super();
		
		if(typeof send != 'function')throw new TypeError('Invalid send function');
		this.#send = send;
	}
	send(event, ...data){
		this.#send(event, ...data);
		return true;
	}
};

module.exports = IPC;

/***/ }),

/***/ "./src/MenuUI/Control.js":
/*!*******************************!*\
  !*** ./src/MenuUI/Control.js ***!
  \*******************************/
/***/ ((module, __unused_webpack_exports, __webpack_require__) => {



var { utils } = __webpack_require__(/*! ./consts */ "./src/MenuUI/consts.js"),
	Events = __webpack_require__(/*! ../Events */ "./src/Events.js");

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
			className: 'settingsBtn',
			textContent: 'Run',
		});
	}
	update(init){
		this.link.textContent = this.value;
	}
};

class FunctionControl extends Control {
	static id = 'function';
	create(){
		utils.add_ele('div', this.content, {
			className: 'settingsBtn',
			textContent: this.data.button || 'Run',
		}).addEventListener('click', () => this.interact());
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

Control.Types = [
	KeybindControl,
	SelectControl,
	BooleanControl,
	FunctionControl,
	LinkControl,
	TextBoxControl,
	SliderControl,
	ColorControl,
	LinkControl,
];

module.exports = Control;

/***/ }),

/***/ "./src/MenuUI/Window/Category.js":
/*!***************************************!*\
  !*** ./src/MenuUI/Window/Category.js ***!
  \***************************************/
/***/ ((module, __unused_webpack_exports, __webpack_require__) => {



var { utils } = __webpack_require__(/*! ../consts */ "./src/MenuUI/consts.js"),
	Control = __webpack_require__(/*! ../Control */ "./src/MenuUI/Control.js");

class Category {
	constructor(tab, label){
		this.tab = tab;
		
		this.controls = new Set();
		
		if(label){
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
		for(let type of Control.Types)if(type.id == data.type){
			let control = new type(name, data, this);
			
			this.controls.add(control);
			
			return control;
		}
		
		throw new TypeError('Unknown type: ' + data.type);
	}
};

module.exports = Category;

/***/ }),

/***/ "./src/MenuUI/consts.js":
/*!******************************!*\
  !*** ./src/MenuUI/consts.js ***!
  \******************************/
/***/ ((__unused_webpack_module, exports, __webpack_require__) => {



var Utils = __webpack_require__(/*! ../utils */ "./src/utils.js"),
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

/***/ "./src/Resources.js":
/*!**************************!*\
  !*** ./src/Resources.js ***!
  \**************************/
/***/ ((__unused_webpack_module, __unused_webpack_exports, __webpack_require__) => {



var Utils = __webpack_require__(/*! ./Utils */ "./src/Utils.js"),
	utils = new Utils();

// wait for krunker css
// utils.wait_for(() => document.querySelector(`link[href*='/css/main_custom.css']`))

var { css, js } = __webpack_require__(/*! ./Runtime */ "./src/Runtime.js");

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
	let module = { exports: {} },
		ev = `(function(module,exports){${data}//# sourceURL=${name}\n})`;
	
	// console.log(ev);

	try{
		eval(ev)(module, module.exports);
	}catch(err){
		console.error(`Error loading script ${name}:\n`, err);
		// todo: postMessage write to logs.txt in client folder
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

/***/ "./src/Utils.js":
/*!**********************!*\
  !*** ./src/Utils.js ***!
  \**********************/
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

/***/ "./src/utils.js":
/*!**********************!*\
  !*** ./src/utils.js ***!
  \**********************/
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

module.exports = JSON.parse('{"game":{"fast_load":true},"client":{"uncap_fps":true}}');

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

__webpack_require__(/*! ./FixLoad */ "./src/FixLoad.js");
__webpack_require__(/*! ./Resources */ "./src/Resources.js");

var HTMLProxy = __webpack_require__(/*! ./HTMLProxy */ "./src/HTMLProxy.js"),
	Category = __webpack_require__(/*! ./MenuUI/Window/Category */ "./src/MenuUI/Window/Category.js"),
	IPC = __webpack_require__(/*! ./IPC */ "./src/IPC.js"),
	Utils = __webpack_require__(/*! ./Utils */ "./src/Utils.js"),
	Events = __webpack_require__(/*! ./Events */ "./src/Events.js"),
	utils = new Utils(),
	ipc = new IPC((...data) => chrome.webview.postMessage(JSON.stringify(data)));

chrome.webview.addEventListener('message', ({ data }) => ipc.emit(...JSON.parse(data)));

class Menu extends Events {
	html = new HTMLProxy();
	config = __webpack_require__(/*! ./Runtime */ "./src/Runtime.js").config;
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
		
		var Inst = this.category('Installation');
		
		Inst.control('Folder', {
			type: 'function',
			button: 'Open',
			value(){
				ipc.send('open folder');
			},
		});
		
		Inst.control('Uncap FPS', {
			type: 'boolean',
			walk: 'client.uncap_fps',
		}).on('change', (value, init) => !init && this.relaunch());
		
		var Game = this.category('Game');
		
		// loads krunker from api.sys32.dev
		Game.control('Fast Loading', {
			type: 'boolean',
			walk: 'game.fast_load',
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
})();

/******/ })()
;
//# sourceMappingURL=Webpack.js.map