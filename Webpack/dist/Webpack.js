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
			textContent: this.data.button || 'Run',
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
			textContent: 'Run',
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
//# sourceMappingURL=data:application/json;charset=utf-8;base64,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V2VicGFja2VyLy4vc3JjL0V2ZW50cy5qcyIsIndlYnBhY2s6Ly9HdXJ1IENsaWVudCsrIFdlYnBhY2tlci8uL3NyYy9GaXhMb2FkLmpzIiwid2VicGFjazovL0d1cnUgQ2xpZW50KysgV2VicGFja2VyLy4vc3JjL0hUTUxQcm94eS5qcyIsIndlYnBhY2s6Ly9HdXJ1IENsaWVudCsrIFdlYnBhY2tlci8uL3NyYy9JUEMuanMiLCJ3ZWJwYWNrOi8vR3VydSBDbGllbnQrKyBXZWJwYWNrZXIvLi9zcmMvTWVudVVJL0NvbnRyb2wuanMiLCJ3ZWJwYWNrOi8vR3VydSBDbGllbnQrKyBXZWJwYWNrZXIvLi9zcmMvTWVudVVJL1dpbmRvdy9DYXRlZ29yeS5qcyIsIndlYnBhY2s6Ly9HdXJ1IENsaWVudCsrIFdlYnBhY2tlci8uL3NyYy9NZW51VUkvY29uc3RzLmpzIiwid2VicGFjazovL0d1cnUgQ2xpZW50KysgV2VicGFja2VyLy4vc3JjL1Jlc291cmNlcy5qcyIsIndlYnBhY2s6Ly9HdXJ1IENsaWVudCsrIFdlYnBhY2tlci8uL3NyYy9SdW50aW1lLmpzIiwid2VicGFjazovL0d1cnUgQ2xpZW50KysgV2VicGFja2VyLy4vc3JjL1V0aWxzLmpzIiwid2VicGFjazovL0d1cnUgQ2xpZW50KysgV2VicGFja2VyLy4vc3JjL3V0aWxzLmpzIiwid2VicGFjazovL0d1cnUgQ2xpZW50KysgV2VicGFja2VyL3dlYnBhY2svYm9vdHN0cmFwIiwid2VicGFjazovL0d1cnUgQ2xpZW50KysgV2VicGFja2VyLy4vc3JjL2luZGV4LmpzIl0sInNvdXJjZXNDb250ZW50IjpbIid1c2Ugc3RyaWN0JztcclxuXHJcbmNsYXNzIEV2ZW50cyB7XHJcblx0c3RhdGljIG9yaWdpbmFsID0gU3ltYm9sKCk7XHJcblx0I2V2ZW50cyA9IG5ldyBNYXAoKTtcclxuXHQjcmVzb2x2ZSh0YXJnZXQsIGV2ZW50KXtcclxuXHRcdHZhciBjYWxsYmFja3MgPSB0aGlzLiNldmVudHMuZ2V0KGV2ZW50KTtcclxuXHRcdFxyXG5cdFx0aWYoIWNhbGxiYWNrcyl7XHJcblx0XHRcdGNhbGxiYWNrcyA9IG5ldyBTZXQoKTtcclxuXHRcdFx0dGhpcy4jZXZlbnRzLnNldChldmVudCwgY2FsbGJhY2tzKTtcclxuXHRcdH1cclxuXHRcdFxyXG5cdFx0cmV0dXJuIGNhbGxiYWNrcztcclxuXHR9O1xyXG5cdG9uKGV2ZW50LCBjYWxsYmFjayl7XHJcblx0XHRpZih0eXBlb2YgY2FsbGJhY2sgIT0gJ2Z1bmN0aW9uJyl0aHJvdyBuZXcgVHlwZUVycm9yKCdDYWxsYmFjayBpcyBub3QgYSBmdW5jdGlvbi4nKTtcclxuXHRcdFxyXG5cdFx0dGhpcy4jcmVzb2x2ZShldmVudCkuYWRkKGNhbGxiYWNrKTtcclxuXHR9XHJcblx0b25jZShldmVudCwgY2FsbGJhY2spe1xyXG5cdFx0dmFyIGNiID0gZnVuY3Rpb24oLi4uZGF0YSl7XHJcblx0XHRcdHRoaXMub2ZmKGV2ZW50LCBjYWxsYmFjayk7XHJcblx0XHRcdGNhbGxiYWNrLmNhbGwodGhpcywgLi4uZGF0YSk7XHJcblx0XHR9O1xyXG5cdFx0XHJcblx0XHRjYWxsYmFja1tFdmVudHMub3JpZ2luYWxdID0gY2I7XHJcblx0XHRcclxuXHRcdHJldHVybiB0aGlzLm9uKGV2ZW50LCBjYik7XHJcblx0fVxyXG5cdG9mZihldmVudCwgY2FsbGJhY2spe1xyXG5cdFx0aWYodHlwZW9mIGNhbGxiYWNrICE9ICdmdW5jdGlvbicpdGhyb3cgbmV3IFR5cGVFcnJvcignQ2FsbGJhY2sgaXMgbm90IGEgZnVuY3Rpb24uJyk7XHJcblx0XHRcclxuXHRcdGlmKGNhbGxiYWNrW0V2ZW50cy5vcmlnaW5hbF0pY2FsbGJhY2sgPSBjYWxsYmFja1tFdmVudHMub3JpZ2luYWxdO1xyXG5cdFx0XHJcblx0XHR2YXIgbGlzdCA9IHRoaXMuI3Jlc29sdmUoZXZlbnQpO1xyXG5cdFx0XHJcblx0XHRyZXR1cm4gbGlzdC5kZWxldGUoY2FsbGJhY2spO1xyXG5cdH1cclxuXHRlbWl0KGV2ZW50LCAuLi5kYXRhKXtcclxuXHRcdHZhciBzZXQgPSB0aGlzLiNyZXNvbHZlKGV2ZW50KTtcclxuXHRcdFxyXG5cdFx0aWYoIXNldC5zaXplKXtcclxuXHRcdFx0aWYoZXZlbnQgPT0gJ2Vycm9yJyl0aHJvdyBkYXRhWzBdO1xyXG5cdFx0XHRyZXR1cm4gZmFsc2U7XHJcblx0XHR9ZWxzZSBmb3IobGV0IGl0ZW0gb2Ygc2V0KXRyeXtcclxuXHRcdFx0aXRlbS5jYWxsKHRoaXMsIC4uLmRhdGEpO1xyXG5cdFx0fWNhdGNoKGVycil7XHJcblx0XHRcdHRoaXMuZW1pdCgnZXJyb3InLCBlcnIpO1xyXG5cdFx0fVxyXG5cdFx0XHJcblx0XHRyZXR1cm4gdHJ1ZTtcclxuXHR9XHJcbn07XHJcblxyXG5tb2R1bGUuZXhwb3J0cyA9IEV2ZW50czsiLCIndXNlIHN0cmljdCc7XHJcblxyXG52YXIgVXRpbHMgPSByZXF1aXJlKCcuL1V0aWxzJyksXHJcblx0dXRpbHMgPSBuZXcgVXRpbHMoKTtcclxuXHJcbihhc3luYyAoKSA9PiB7XHRcclxuXHR2YXIgdWlfYmFzZSA9IGF3YWl0IHV0aWxzLndhaXRfZm9yKCgpID0+IGRvY3VtZW50LnF1ZXJ5U2VsZWN0b3IoJyN1aUJhc2UnKSksXHJcblx0XHRNSU5fV0lEVEggPSAxNzAwLFxyXG5cdFx0TUlOX0hFSUdIVCA9IDkwMDtcclxuXHRcdFxyXG5cdGlmKGxvY2FsU3RvcmFnZT8ua3JvX3NldG5nc3NfdWlTY2FsaW5nID09PSAnZmFsc2UnKXJldHVybjtcclxuXHRcclxuXHR2YXIgbHMgPSBsb2NhbFN0b3JhZ2Uua3JvX3NldG5nc3Nfc2NhbGVVSSxcclxuXHRcdHNjYWxlX3VpID0gbHMgIT0gdm9pZFtdID8gbHMgOiAwLjc7XHJcblx0XHJcblx0c2NhbGVfdWkgPSBNYXRoLm1pbigxLCBNYXRoLm1heCgwLjEsIE51bWJlcihzY2FsZV91aSkpKTtcclxuXHRzY2FsZV91aSA9IDEgKyAoMSAtIHNjYWxlX3VpKTtcclxuXHRcclxuXHR2YXIgaGVpZ2h0ID0gd2luZG93LmlubmVySGVpZ2h0LFxyXG5cdFx0d2lkdGggPSB3aW5kb3cuaW5uZXJXaWR0aCxcclxuXHRcdG1pbl93aWR0aCA9IE1JTl9XSURUSCAqIHNjYWxlX3VpLFxyXG5cdFx0bWluX2hlaWdodCA9IE1JTl9IRUlHSFQgKiBzY2FsZV91aSxcclxuXHRcdHdpZHRoX3NjYWxlID0gd2lkdGggLyBtaW5fd2lkdGgsXHJcblx0XHRoZWlnaHRfc2NhbGUgPSBoZWlnaHQgLyBtaW5faGVpZ2h0LFxyXG5cdFx0ZWxtID0gZG9jdW1lbnQuZ2V0RWxlbWVudEJ5SWQoJ3VpQmFzZScpLFxyXG5cdFx0c3R5bGUgPSBoZWlnaHRfc2NhbGUgPCB3aWR0aF9zY2FsZSA/IHtcclxuXHRcdFx0dHJhbnNmb3JtOiBoZWlnaHRfc2NhbGUsXHJcblx0XHRcdHdpZHRoOiB3aWR0aCAvIGhlaWdodF9zY2FsZSxcclxuXHRcdFx0aGVpZ2h0OiBtaW5faGVpZ2h0LFxyXG5cdFx0fSA6IHtcclxuXHRcdFx0dHJhbnNmb3JtOiB3aWR0aF9zY2FsZSxcclxuXHRcdFx0d2lkdGg6IG1pbl93aWR0aCxcclxuXHRcdFx0aGVpZ2h0OiBoZWlnaHQgLyB3aWR0aF9zY2FsZSxcclxuXHRcdH07XHJcblx0XHJcblx0dWlfYmFzZS5zdHlsZS50cmFuc2Zvcm0gPSAnc2NhbGUoJyArIHN0eWxlLnRyYW5zZm9ybS50b0ZpeGVkKDMpICsgJyknO1xyXG5cdHVpX2Jhc2Uuc3R5bGUud2lkdGggPSBzdHlsZS53aWR0aC50b0ZpeGVkKDMpICsgJ3B4JztcclxuXHR1aV9iYXNlLnN0eWxlLmhlaWdodCA9IHN0eWxlLmhlaWdodC50b0ZpeGVkKDMpICsgJ3B4JztcclxufSkoKTsiLCIndXNlIHN0cmljdCc7XHJcblxyXG52YXIgbG9nID0gY29uc29sZS5sb2c7XHJcblxyXG5jbGFzcyBIVE1MUHJveHkge1xyXG5cdGNoaWxkcmVuID0gW107XHJcblx0YXBwZW5kQ2hpbGQobm9kZSl7XHJcblx0XHR0aGlzLmFwcGVuZChub2RlKTtcclxuXHRcdHJldHVybiBub2RlO1xyXG5cdH1cclxuXHRhcHBlbmQobm9kZSl7XHJcblx0XHR0aGlzLmNoaWxkcmVuLnB1c2gobm9kZSk7XHJcblx0fVxyXG5cdGNvbnN0cnVjdG9yKCl7XHJcblx0XHR0aGlzLmlkID0gJ2EtJyArIE1hdGgucmFuZG9tKCkudG9TdHJpbmcoKS5zbGljZSgyKTtcclxuXHRcdFxyXG5cdFx0dmFyIGNoaWxkcmVuID0gdGhpcy5jaGlsZHJlbjtcclxuXHRcdFxyXG5cdFx0Y3VzdG9tRWxlbWVudHMuZGVmaW5lKHRoaXMuaWQsIGNsYXNzIGV4dGVuZHMgSFRNTEVsZW1lbnQge1xyXG5cdFx0XHRjb25uZWN0ZWRDYWxsYmFjaygpe1xyXG5cdFx0XHRcdGZvcihsZXQgbm9kZSBvZiBjaGlsZHJlbil0aGlzLnBhcmVudE5vZGUuaW5zZXJ0QmVmb3JlKG5vZGUsIHRoaXMpO1xyXG5cdFx0XHRcdHRoaXMucmVtb3ZlKCk7XHJcblx0XHRcdFx0XHJcblx0XHRcdFx0Ly8gbG9nKG5vZGUuY2hpbGRyZW5bdGhpcy5pZF0sIHRoaXMuaWQsIG5vZGUuY2hpbGRyZW4sIG5vZGUpO1xyXG5cdFx0XHRcdC8vIHRoaXMucmVwbGFjZVdpdGgobm9kZS5jaGlsZHJlblt0aGlzLmlkXSk7XHJcblx0XHRcdH1cclxuXHRcdH0pO1xyXG5cdH1cclxuXHRnZXQoKXtcclxuXHRcdHZhciBodG1sID0gYDwke3RoaXMuaWR9PjwvJHt0aGlzLmlkfT5gO1xyXG5cdFx0XHJcblx0XHQvLyBodG1sICs9ICc8IS0tICc7XHJcblx0XHQvLyBmb3IobGV0IG5vZGUgb2YgdGhpcy5jaGlsZHJlbilodG1sICs9IG5vZGUub3V0ZXJIVE1MLnJlcGxhY2UoLy0tPi9nLCAnLT4nKTtcclxuXHRcdC8vIGh0bWwgKz0gJy0tPic7XHJcblx0XHRcclxuXHRcdHJldHVybiBodG1sO1xyXG5cdH1cclxufTtcclxuXHJcbm1vZHVsZS5leHBvcnRzID0gSFRNTFByb3h5OyIsIid1c2Ugc3RyaWN0JztcclxuXHJcbnZhciBFdmVudHMgPSByZXF1aXJlKCcuL0V2ZW50cycpO1xyXG5cclxuY2xhc3MgSVBDIGV4dGVuZHMgRXZlbnRzIHtcclxuXHQjc2VuZFxyXG5cdGNvbnN0cnVjdG9yKHNlbmQpe1xyXG5cdFx0c3VwZXIoKTtcclxuXHRcdFxyXG5cdFx0aWYodHlwZW9mIHNlbmQgIT0gJ2Z1bmN0aW9uJyl0aHJvdyBuZXcgVHlwZUVycm9yKCdJbnZhbGlkIHNlbmQgZnVuY3Rpb24nKTtcclxuXHRcdHRoaXMuI3NlbmQgPSBzZW5kO1xyXG5cdH1cclxuXHRzZW5kKGV2ZW50LCAuLi5kYXRhKXtcclxuXHRcdHRoaXMuI3NlbmQoZXZlbnQsIC4uLmRhdGEpO1xyXG5cdFx0cmV0dXJuIHRydWU7XHJcblx0fVxyXG59O1xyXG5cclxubW9kdWxlLmV4cG9ydHMgPSBJUEM7IiwiJ3VzZSBzdHJpY3QnO1xuXG52YXIgeyB1dGlscyB9ID0gcmVxdWlyZSgnLi9jb25zdHMnKSxcblx0RXZlbnRzID0gcmVxdWlyZSgnLi4vRXZlbnRzJyk7XG5cbmNsYXNzIENvbnRyb2wgZXh0ZW5kcyBFdmVudHMge1xuXHRjb25zdHJ1Y3RvcihuYW1lLCBkYXRhLCBjYXRlZ29yeSl7XG5cdFx0c3VwZXIoKTtcblx0XHRcblx0XHR0aGlzLmRhdGEgPSBkYXRhO1xuXHRcdHRoaXMubmFtZSA9IG5hbWU7XG5cdFx0dGhpcy5jYXRlZ29yeSA9IGNhdGVnb3J5O1xuXHRcdHRoaXMubWVudSA9IHRoaXMuY2F0ZWdvcnkudGFiLndpbmRvdy5tZW51O1xuXHRcdFxuXHRcdHRoaXMuY29udGVudCA9IHV0aWxzLmFkZF9lbGUoJ2RpdicsIHRoaXMuY2F0ZWdvcnkuY29udGVudCwgeyBjbGFzc05hbWU6ICdzZXR0TmFtZScgfSk7XG5cdFx0dGhpcy5sYWJlbCA9IHV0aWxzLmFkZF9lbGUoJ3RleHQnLCB0aGlzLmNvbnRlbnQpO1xuXHRcdFxuXHRcdHRoaXMuY3JlYXRlKCk7XG5cdFx0XG5cdFx0dGhpcy5tZW51LmVtaXQoJ2NvbnRyb2wnLCB0aGlzKTtcblx0fVxuXHRsYWJlbF90ZXh0KHRleHQpe1xuXHRcdHRoaXMubGFiZWwubm9kZVZhbHVlID0gdGV4dDtcblx0fVxuXHRyZW1vdmUoKXtcblx0XHR0aGlzLmNvbnRlbnQucmVtb3ZlKCk7XG5cdH1cblx0d2FsayhkYXRhKXtcblx0XHR2YXIgc3RhdGUgPSB0aGlzLm1lbnUuY29uZmlnLFxuXHRcdFx0bGFzdF9zdGF0ZSxcblx0XHRcdGxhc3Rfa2V5O1xuXHRcdFxuXHRcdGRhdGEuc3BsaXQoJy4nKS5mb3JFYWNoKGtleSA9PiBzdGF0ZSA9ICgobGFzdF9zdGF0ZSA9IHN0YXRlKVtsYXN0X2tleSA9IGtleV0gfHwge30pKTtcblx0XHRcblx0XHRyZXR1cm4gWyBsYXN0X3N0YXRlLCBsYXN0X2tleSBdO1xuXHR9XG5cdGdldCB2YWx1ZSgpe1xuXHRcdGlmKHR5cGVvZiB0aGlzLmRhdGEudmFsdWUgPT0gJ2Z1bmN0aW9uJylyZXR1cm4gdGhpcy5kYXRhLnZhbHVlO1xuXHRcdFxuXHRcdHZhciB3YWxrZWQgPSB0aGlzLndhbGsodGhpcy5kYXRhLndhbGspO1xuXHRcdFxuXHRcdHJldHVybiB3YWxrZWRbMF1bd2Fsa2VkWzFdXTtcblx0fVxuXHRzZXQgdmFsdWUodmFsdWUpe1xuXHRcdHZhciB3YWxrZWQgPSB0aGlzLndhbGsodGhpcy5kYXRhLndhbGspO1xuXHRcdFxuXHRcdHdhbGtlZFswXVt3YWxrZWRbMV1dID0gdmFsdWU7XG5cdFx0XG5cdFx0dGhpcy5tZW51LnNhdmVfY29uZmlnKCk7XG5cdFx0XG5cdFx0dGhpcy5lbWl0KCdjaGFuZ2UnLCB2YWx1ZSk7XG5cdFx0XG5cdFx0cmV0dXJuIHZhbHVlO1xuXHR9XG5cdGNyZWF0ZSgpe31cblx0aW50ZXJhY3QoKXtcblx0XHRjb25zb2xlLndhcm4oJ05vIGRlZmluZWQgaW50ZXJhY3Rpb24gZm9yJywgdGhpcyk7XG5cdH1cblx0dXBkYXRlKGluaXQpe1xuXHRcdC8vIE1BS0UgQ0hBTkdFIEVNSVQgQ0FMTEVEIEZST00gVEhFIENBVEVHT1JZXG5cdFx0aWYoaW5pdCl0aGlzLmVtaXQoJ2NoYW5nZScsIHRoaXMudmFsdWUsIHRydWUpO1xuXHRcdHRoaXMubGFiZWxfdGV4dCh0aGlzLm5hbWUpO1xuXHR9XG5cdHNob3dfY29udGVudCgpe1xuXHRcdHRoaXMuY29udGVudC5zdHlsZS5kaXNwbGF5ID0gJ2Jsb2NrJztcblx0fVxuXHRoaWRlX2NvbnRlbnQoKXtcblx0XHR0aGlzLmNvbnRlbnQuc3R5bGUuZGlzcGxheSA9ICdub25lJztcblx0fVxufTtcblxuY2xhc3MgQm9vbGVhbkNvbnRyb2wgZXh0ZW5kcyBDb250cm9sIHtcblx0c3RhdGljIGlkID0gJ2Jvb2xlYW4nO1xuXHRjcmVhdGUoKXtcblx0XHR0aGlzLnN3aXRjaCA9IHV0aWxzLmFkZF9lbGUoJ2xhYmVsJywgdGhpcy5jb250ZW50LCB7XG5cdFx0XHRjbGFzc05hbWU6ICdzd2l0Y2gnLFxuXHRcdFx0dGV4dENvbnRlbnQ6IHRoaXMuZGF0YS5idXR0b24gfHwgJ1J1bicsXG5cdFx0XHRzdHlsZToge1xuXHRcdFx0XHQnbWFyZ2luLWxlZnQnOiAnMTBweCcsXG5cdFx0XHR9LFxuXHRcdH0pO1xuXHRcdFxuXHRcdHRoaXMuaW5wdXQgPSB1dGlscy5hZGRfZWxlKCdpbnB1dCcsIHRoaXMuc3dpdGNoLCB7IHR5cGU6ICdjaGVja2JveCcgfSk7XG5cdFx0XG5cdFx0dGhpcy5pbnB1dC5hZGRFdmVudExpc3RlbmVyKCdjaGFuZ2UnLCAoKSA9PiB0aGlzLnZhbHVlID0gdGhpcy5pbnB1dC5jaGVja2VkKTtcblx0XHRcblx0XHR1dGlscy5hZGRfZWxlKCdzcGFuJywgdGhpcy5zd2l0Y2gsIHsgY2xhc3NOYW1lOiAnc2xpZGVyJyB9KTtcblx0fVxuXHR1cGRhdGUoaW5pdCl7XG5cdFx0c3VwZXIudXBkYXRlKGluaXQpO1xuXHRcdGlmKGluaXQpdGhpcy5pbnB1dC5jaGVja2VkID0gdGhpcy52YWx1ZTtcblx0XHR0aGlzLmxhYmVsX3RleHQodGhpcy5uYW1lKTtcblx0fVxufVxuXG5jbGFzcyBTZWxlY3RDb250cm9sIGV4dGVuZHMgQ29udHJvbCB7XG5cdHN0YXRpYyBpZCA9ICdzZWxlY3QnO1xuXHRjcmVhdGUoKXtcblx0XHR0aGlzLnNlbGVjdCA9IHV0aWxzLmFkZF9lbGUoJ3NlbGVjdCcsIHRoaXMuY29udGVudCwgeyBjbGFzc05hbWU6ICdpbnB1dEdyZXkyJyB9KTtcblx0XHRcblx0XHR0aGlzLnNlbGVjdC5hZGRFdmVudExpc3RlbmVyKCdjaGFuZ2UnLCAoKSA9PiB0aGlzLnZhbHVlID0gdGhpcy5zZWxlY3QudmFsdWUpO1xuXHRcdFxuXHRcdGZvcihsZXQgdmFsdWUgaW4gdGhpcy5kYXRhLnZhbHVlKXV0aWxzLmFkZF9lbGUoJ29wdGlvbicsIHRoaXMuc2VsZWN0LCB7XG5cdFx0XHR2YWx1ZTogdmFsdWUsXG5cdFx0XHR0ZXh0Q29udGVudDogdGhpcy5kYXRhLnZhbHVlW3ZhbHVlXSxcblx0XHR9KTtcblx0fVxuXHR1cGRhdGUoaW5pdCl7XG5cdFx0c3VwZXIudXBkYXRlKGluaXQpO1xuXHRcdFxuXHRcdGlmKGluaXQpdGhpcy5zZWxlY3QudmFsdWUgPSB0aGlzLnZhbHVlO1xuXHR9XG59O1xuXG5jbGFzcyBMaW5rQ29udHJvbCBleHRlbmRzIENvbnRyb2wge1xuXHRzdGF0aWMgaWQgPSAnbGluayc7XG5cdGNyZWF0ZSgpe1xuXHRcdHRoaXMubGluayA9IHV0aWxzLmFkZF9lbGUoJ2EnLCB0aGlzLmNvbnRlbnQsIHtcblx0XHRcdGNsYXNzTmFtZTogJ3NldHRpbmdzQnRuJyxcblx0XHRcdHRleHRDb250ZW50OiAnUnVuJyxcblx0XHR9KTtcblx0fVxuXHR1cGRhdGUoaW5pdCl7XG5cdFx0dGhpcy5saW5rLnRleHRDb250ZW50ID0gdGhpcy52YWx1ZTtcblx0fVxufTtcblxuY2xhc3MgRnVuY3Rpb25Db250cm9sIGV4dGVuZHMgQ29udHJvbCB7XG5cdHN0YXRpYyBpZCA9ICdmdW5jdGlvbic7XG5cdGNyZWF0ZSgpe1xuXHRcdHV0aWxzLmFkZF9lbGUoJ2RpdicsIHRoaXMuY29udGVudCwge1xuXHRcdFx0Y2xhc3NOYW1lOiAnc2V0dGluZ3NCdG4nLFxuXHRcdFx0dGV4dENvbnRlbnQ6ICdSdW4nLFxuXHRcdH0pLmFkZEV2ZW50TGlzdGVuZXIoJ2NsaWNrJywgKCkgPT4gdGhpcy5pbnRlcmFjdCgpKTtcblx0fVxuXHRpbnRlcmFjdCgpe1xuXHRcdHRoaXMudmFsdWUoKTtcblx0fVxufTtcblxuY2xhc3MgS2V5YmluZENvbnRyb2wgZXh0ZW5kcyBDb250cm9sIHtcblx0c3RhdGljIGlkID0gJ2tleWJpbmQnO1xuXHRjcmVhdGUoKXtcblx0XHR0aGlzLmlucHV0ID0gdXRpbHMuYWRkX2VsZSgnaW5wdXQnLCB0aGlzLmNvbnRlbnQsIHtcblx0XHRcdGNsYXNzTmFtZTogJ2lucHV0R3JleTInLFxuXHRcdFx0cGxhY2Vob2xkZXI6ICdQcmVzcyBhIGtleScsXG5cdFx0XHRzdHlsZToge1xuXHRcdFx0XHRkaXNwbGF5OiAnaW5saW5lLWJsb2NrJyxcblx0XHRcdFx0d2lkdGg6ICcyMjBweCcsXG5cdFx0XHR9LFxuXHRcdH0pO1xuXHRcdFxuXHRcdHRoaXMuaW5wdXQuYWRkRXZlbnRMaXN0ZW5lcignZm9jdXMnLCAoKSA9PiB7XG5cdFx0XHR0aGlzLmlucHV0LnZhbHVlID0gJyc7XG5cdFx0fSk7XG5cdFx0XG5cdFx0dGhpcy5pbnB1dC5hZGRFdmVudExpc3RlbmVyKCdrZXlkb3duJywgZXZlbnQgPT4ge1xuXHRcdFx0ZXZlbnQucHJldmVudERlZmF1bHQoKTtcblx0XHRcdHRoaXMudmFsdWUgPSBldmVudC5jb2RlID09ICdFc2NhcGUnID8gbnVsbCA6IGV2ZW50LmNvZGU7XG5cdFx0XHR0aGlzLmlucHV0LmJsdXIoKTtcblx0XHR9KTtcblxuXHRcdHRoaXMuaW5wdXQuYWRkRXZlbnRMaXN0ZW5lcignYmx1cicsICgpID0+IHtcblx0XHRcdHRoaXMuY2F0ZWdvcnkudXBkYXRlKCk7XG5cdFx0XHR0aGlzLnVwZGF0ZSgpO1xuXHRcdH0pO1xuXHR9XG5cdHVwZGF0ZShpbml0KXtcblx0XHRzdXBlci51cGRhdGUoaW5pdCk7XG5cdFx0XG5cdFx0dGhpcy5pbnB1dC52YWx1ZSA9IHV0aWxzLnN0cmluZ19rZXkodGhpcy52YWx1ZSk7XG5cdH1cbn07XG5cbmNsYXNzIFRleHRCb3hDb250cm9sIGV4dGVuZHMgQ29udHJvbCB7XG5cdHN0YXRpYyBpZCA9ICd0ZXh0Ym94Jztcblx0Y3JlYXRlKCl7XG5cdFx0dGhpcy5pbnB1dCA9IHV0aWxzLmFkZF9lbGUoJ2lucHV0JywgdGhpcy5jb250ZW50LCB7XG5cdFx0XHRjbGFzc05hbWU6ICdpbnB1dEdyZXkyJyxcblx0XHRcdHBsYWNlaG9sZGVyOiB0aGlzLmRhdGEucGxhY2Vob2xkZXIgfHwgJycsXG5cdFx0XHRzdHlsZToge1xuXHRcdFx0XHRkaXNwbGF5OiAnaW5saW5lLWJsb2NrJyxcblx0XHRcdFx0d2lkdGg6ICcyMjBweCcsXG5cdFx0XHR9LFxuXHRcdH0pO1xuXHRcdFxuXHRcdHRoaXMuaW5wdXQuYWRkRXZlbnRMaXN0ZW5lcignY2hhbmdlJywgKCkgPT4gdGhpcy52YWx1ZSA9IHRoaXMuaW5wdXQudmFsdWUpO1xuXHR9XG5cdHVwZGF0ZShpbml0KXtcblx0XHRzdXBlci51cGRhdGUoaW5pdCk7XG5cdFx0XG5cdFx0aWYoaW5pdCl0aGlzLmlucHV0LnZhbHVlID0gdGhpcy52YWx1ZTtcblx0fVxufTtcblxuY2xhc3MgU2xpZGVyQ29udHJvbCBleHRlbmRzIENvbnRyb2wge1xuXHRzdGF0aWMgaWQgPSAnc2xpZGVyJztcblx0Y3JlYXRlKCl7XG5cdFx0dmFyIHNsaWRlciA9IHtcblx0XHRcdG1pbjogdGhpcy5kYXRhLm1pbixcblx0XHRcdG1heDogdGhpcy5kYXRhLm1heCxcblx0XHRcdHN0ZXA6IHRoaXMuZGF0YS5zdGVwLFxuXHRcdH07XG5cdFx0XG5cdFx0dGhpcy5pbnB1dCA9IHV0aWxzLmFkZF9lbGUoJ2lucHV0JywgdGhpcy5jb250ZW50LCB7XG5cdFx0XHRjbGFzc05hbWU6ICdzbGlkZXJWYWwnLFxuXHRcdFx0dHlwZTogJ251bWJlcicsXG5cdFx0XHQuLi5zbGlkZXIsXG5cdFx0fSk7XG5cdFx0XG5cdFx0dGhpcy5zbGlkZXIgPSB1dGlscy5hZGRfZWxlKCdpbnB1dCcsIHV0aWxzLmFkZF9lbGUoJ2RpdicsIHRoaXMuY29udGVudCwge1xuXHRcdFx0Y2xhc3NOYW1lOiAnc2xpZGVjb250YWluZXInLFxuXHRcdFx0c3R5bGU6IHsgJ21hcmdpbi10b3AnOiAnLThweCcgfSxcblx0XHR9KSwge1xuXHRcdFx0Y2xhc3NOYW1lOiAnc2xpZGVyTScsXG5cdFx0XHR0eXBlOiAncmFuZ2UnLFxuXHRcdFx0Li4uc2xpZGVyLFxuXHRcdH0pO1xuXHRcdFxuXHRcdHRoaXMuaW5wdXQuYWRkRXZlbnRMaXN0ZW5lcignZm9jdXMnLCAoKSA9PiAodGhpcy5pbnB1dF9mb2N1c2VkID0gdHJ1ZSwgdGhpcy5pbnRlcmFjdCgpKSk7XG5cdFx0dGhpcy5pbnB1dC5hZGRFdmVudExpc3RlbmVyKCdibHVyJywgKCkgPT4gKHRoaXMuaW5wdXRfZm9jdXNlZCA9IGZhbHNlLCB0aGlzLmludGVyYWN0KCkpKTtcblx0XHRcblx0XHR0aGlzLnNsaWRlci5hZGRFdmVudExpc3RlbmVyKCdpbnB1dCcsICgpID0+IHRoaXMuaW50ZXJhY3QodGhpcy52YWx1ZSA9IHRoaXMuc2xpZGVyLnZhbHVlKSk7XG5cdFx0dGhpcy5pbnB1dC5hZGRFdmVudExpc3RlbmVyKCdpbnB1dCcsICgpID0+IHRoaXMuaW50ZXJhY3QodGhpcy52YWx1ZSA9ICt0aGlzLmlucHV0LnZhbHVlKSk7XG5cdH1cblx0aW50ZXJhY3QoKXtcblx0XHR2YXIgbGFiZWwgPSAhdGhpcy5pbnB1dF9mb2N1c2VkICYmIHRoaXMuZGF0YS5sYWJlbHMgJiYgdGhpcy5kYXRhLmxhYmVsc1t0aGlzLnZhbHVlXSB8fCB0aGlzLnZhbHVlO1xuXHRcdFxuXHRcdHRoaXMuaW5wdXQudHlwZSA9IHR5cGVvZiBsYWJlbCA9PSAnc3RyaW5nJyA/ICd0ZXh0JyA6ICdudW1iZXInO1xuXHRcdFxuXHRcdHRoaXMuaW5wdXQudmFsdWUgPSBsYWJlbDtcblx0XHRcblx0XHR0aGlzLnNsaWRlci52YWx1ZSA9IHRoaXMudmFsdWU7XG5cdH1cblx0dXBkYXRlKGluaXQpe1xuXHRcdHN1cGVyLnVwZGF0ZShpbml0KTtcblx0XHRcblx0XHR0aGlzLmludGVyYWN0KCk7XG5cdH1cbn07XG5cbmNsYXNzIENvbG9yQ29udHJvbCBleHRlbmRzIENvbnRyb2wge1xuXHRzdGF0aWMgaWQgPSAnY29sb3InO1xuXHRjcmVhdGUoKXtcblx0XHR0aGlzLmlucHV0ID0gdXRpbHMuYWRkX2VsZSgnaW5wdXQnLCB0aGlzLmNvbnRlbnQsIHtcblx0XHRcdG5hbWU6ICdjb2xvcicsXG5cdFx0XHR0eXBlOiAnY29sb3InLFxuXHRcdFx0c3R5bGU6IHtcblx0XHRcdFx0ZmxvYXQ6ICdyaWdodCcsXG5cdFx0XHR9LFxuXHRcdH0pO1xuXHRcdFxuXHRcdHRoaXMuaW5wdXQuYWRkRXZlbnRMaXN0ZW5lcignY2hhbmdlJywgKCkgPT4gdGhpcy52YWx1ZSA9IHRoaXMuaW5wdXQudmFsdWUpO1xuXHR9XG5cdHVwZGF0ZShpbml0KXtcblx0XHRzdXBlci51cGRhdGUoaW5pdCk7XG5cdFx0XG5cdFx0aWYoaW5pdCl0aGlzLmlucHV0LnZhbHVlID0gdGhpcy52YWx1ZTtcblx0fVxufTtcblxuQ29udHJvbC5UeXBlcyA9IFtcblx0S2V5YmluZENvbnRyb2wsXG5cdFNlbGVjdENvbnRyb2wsXG5cdEJvb2xlYW5Db250cm9sLFxuXHRGdW5jdGlvbkNvbnRyb2wsXG5cdExpbmtDb250cm9sLFxuXHRUZXh0Qm94Q29udHJvbCxcblx0U2xpZGVyQ29udHJvbCxcblx0Q29sb3JDb250cm9sLFxuXHRMaW5rQ29udHJvbCxcbl07XG5cbm1vZHVsZS5leHBvcnRzID0gQ29udHJvbDsiLCIndXNlIHN0cmljdCc7XG5cbnZhciB7IHV0aWxzIH0gPSByZXF1aXJlKCcuLi9jb25zdHMnKSxcblx0Q29udHJvbCA9IHJlcXVpcmUoJy4uL0NvbnRyb2wnKTtcblxuY2xhc3MgQ2F0ZWdvcnkge1xuXHRjb25zdHJ1Y3Rvcih0YWIsIGxhYmVsKXtcblx0XHR0aGlzLnRhYiA9IHRhYjtcblx0XHRcblx0XHR0aGlzLmNvbnRyb2xzID0gbmV3IFNldCgpO1xuXHRcdFxuXHRcdGlmKGxhYmVsKXtcblx0XHRcdHRoaXMuaGVhZGVyID0gdXRpbHMuYWRkX2VsZSgnZGl2JywgdGhpcy50YWIuY29udGVudCwge1xuXHRcdFx0XHRjbGFzc05hbWU6ICdzZXRIZWQnLFxuXHRcdFx0fSk7XG5cdFx0XHRcblx0XHRcdHRoaXMuaGVhZGVyX3N0YXR1cyA9IHV0aWxzLmFkZF9lbGUoJ3NwYW4nLCB0aGlzLmhlYWRlciwgeyBjbGFzc05hbWU6ICdtYXRlcmlhbC1pY29ucyBwbHVzT3JNaW51cycgfSk7XG5cdFx0XHRcblx0XHRcdHV0aWxzLmFkZF9lbGUoJ3RleHQnLCB0aGlzLmhlYWRlciwgeyBub2RlVmFsdWU6IGxhYmVsIH0pO1xuXHRcdFx0XG5cdFx0XHR0aGlzLmhlYWRlci5hZGRFdmVudExpc3RlbmVyKCdjbGljaycsICgpID0+IHRoaXMudG9nZ2xlKCkpO1xuXHRcdH1cblx0XHRcblx0XHR0aGlzLmNvbnRlbnQgPSB1dGlscy5hZGRfZWxlKCdkaXYnLCB0aGlzLnRhYi5jb250ZW50LCB7XG5cdFx0XHRjbGFzc05hbWU6ICdzZXRCb2RIJyxcblx0XHR9KTtcblx0XHRcblx0XHRpZihsYWJlbCl0aGlzLmV4cGFuZCgpO1xuXHR9XG5cdHRvZ2dsZSgpe1xuXHRcdGlmKHRoaXMuY29sbGFwc2VkKXRoaXMuZXhwYW5kKCk7XG5cdFx0ZWxzZSB0aGlzLmNvbGxhcHNlKCk7XG5cdH1cblx0Y29sbGFwc2UoKXtcblx0XHR0aGlzLmNvbGxhcHNlZCA9IHRydWU7XG5cdFx0dGhpcy51cGRhdGUoKTtcblx0fVxuXHRleHBhbmQoKXtcblx0XHR0aGlzLmNvbGxhcHNlZCA9IGZhbHNlO1xuXHRcdHRoaXMudXBkYXRlKCk7XG5cdH1cblx0dXBkYXRlKGluaXQpe1xuXHRcdHRoaXMuY29udGVudC5zdHlsZS5kaXNwbGF5ID0gdGhpcy5jb2xsYXBzZWQgPyAnbm9uZScgOiAnYmxvY2snO1xuXHRcdFxuXHRcdGlmKHRoaXMuaGVhZGVyKXtcblx0XHRcdHRoaXMuaGVhZGVyLnN0eWxlLmRpc3BsYXkgPSAnYmxvY2snO1xuXHRcdFx0dGhpcy5oZWFkZXJfc3RhdHVzLnRleHRDb250ZW50ID0gJ2tleWJvYXJkX2Fycm93XycgKyAodGhpcy5jb2xsYXBzZWQgPyAncmlnaHQnIDogJ2Rvd24nKTtcblx0XHR9XG5cdFx0XG5cdFx0Zm9yKGxldCBjb250cm9sIG9mIHRoaXMuY29udHJvbHMpY29udHJvbC51cGRhdGUoaW5pdCk7XG5cdH1cblx0c2hvdygpe1xuXHRcdHRoaXMuZXhwYW5kKCk7XG5cdFx0aWYodGhpcy5oZWFkZXIpdGhpcy5oZWFkZXIuc3R5bGUuZGlzcGxheSA9ICdibG9jayc7XG5cdH1cblx0aGlkZSgpe1xuXHRcdHRoaXMuY29udGVudC5zdHlsZS5kaXNwbGF5ID0gJ25vbmUnO1xuXHRcdGlmKHRoaXMuaGVhZGVyKXRoaXMuaGVhZGVyLnN0eWxlLmRpc3BsYXkgPSAnbm9uZSc7XG5cdH1cblx0Zml4KCl7XG5cdFx0dGhpcy51cGRhdGUoKTtcblx0XHRmb3IobGV0IGNvbnRyb2wgb2YgdGhpcy5jb250cm9scyljb250cm9sLnNob3dfY29udGVudCgpO1xuXHR9XG5cdGNvbnRyb2wobmFtZSwgZGF0YSl7XG5cdFx0Zm9yKGxldCB0eXBlIG9mIENvbnRyb2wuVHlwZXMpaWYodHlwZS5pZCA9PSBkYXRhLnR5cGUpe1xuXHRcdFx0bGV0IGNvbnRyb2wgPSBuZXcgdHlwZShuYW1lLCBkYXRhLCB0aGlzKTtcblx0XHRcdFxuXHRcdFx0dGhpcy5jb250cm9scy5hZGQoY29udHJvbCk7XG5cdFx0XHRcblx0XHRcdHJldHVybiBjb250cm9sO1xuXHRcdH1cblx0XHRcblx0XHR0aHJvdyBuZXcgVHlwZUVycm9yKCdVbmtub3duIHR5cGU6ICcgKyBkYXRhLnR5cGUpO1xuXHR9XG59O1xuXG5tb2R1bGUuZXhwb3J0cyA9IENhdGVnb3J5OyIsIid1c2Ugc3RyaWN0JztcblxudmFyIFV0aWxzID0gcmVxdWlyZSgnLi4vdXRpbHMnKSxcblx0dXRpbHMgPSBuZXcgVXRpbHMoKTtcblxuZXhwb3J0cy51dGlscyA9IHV0aWxzO1xuXG5leHBvcnRzLnRpY2sgPSBub2RlID0+IG5vZGUuYWRkRXZlbnRMaXN0ZW5lcignbW91c2VlbnRlcicsICgpID0+IHtcblx0dHJ5e1xuXHRcdHBsYXlUaWNrKCk7XG5cdH1jYXRjaChlcnIpe31cbn0pO1xuXG5leHBvcnRzLnNlbGVjdCA9IG5vZGUgPT4gbm9kZS5hZGRFdmVudExpc3RlbmVyKCdjbGljaycsICgpID0+IHtcblx0dHJ5e1xuXHRcdFNPVU5ELnBsYXkoJ3NlbGVjdF8wJywgMC4xKTtcblx0fWNhdGNoKGVycil7fVxufSk7IiwiJ3VzZSBzdHJpY3QnO1xyXG5cclxudmFyIFV0aWxzID0gcmVxdWlyZSgnLi9VdGlscycpLFxyXG5cdHV0aWxzID0gbmV3IFV0aWxzKCk7XHJcblxyXG4vLyB3YWl0IGZvciBrcnVua2VyIGNzc1xyXG4vLyB1dGlscy53YWl0X2ZvcigoKSA9PiBkb2N1bWVudC5xdWVyeVNlbGVjdG9yKGBsaW5rW2hyZWYqPScvY3NzL21haW5fY3VzdG9tLmNzcyddYCkpXHJcblxyXG52YXIgeyBjc3MsIGpzIH0gPSByZXF1aXJlKCcuL1J1bnRpbWUnKTtcclxuXHJcbmZvcihsZXQgWyBuYW1lLCBkYXRhIF0gb2YgT2JqZWN0LmVudHJpZXMoY3NzKSl7XHJcblx0bGV0IHVybCA9IFVSTC5jcmVhdGVPYmplY3RVUkwobmV3IEJsb2IoWyBkYXRhIF0sIHsgdHlwZTogJ3RleHQvY3NzJyB9KSk7XHJcblx0XHJcblx0bGV0IGxpbmsgPSBkb2N1bWVudC5oZWFkLmFwcGVuZENoaWxkKE9iamVjdC5hc3NpZ24oZG9jdW1lbnQuY3JlYXRlRWxlbWVudCgnbGluaycpLCB7XHJcblx0XHRyZWw6ICdzdHlsZXNoZWV0JyxcclxuXHRcdGhyZWY6IHVybCxcclxuXHR9KSk7XHJcblxyXG5cdGxpbmsuYWRkRXZlbnRMaXN0ZW5lcignbG9hZCcsICgpID0+IHtcclxuXHRcdFVSTC5yZXZva2VPYmplY3RVUkwodXJsKTtcclxuXHR9KTtcclxuXHRcclxuXHRkb2N1bWVudC5oZWFkLmFwcGVuZENoaWxkKGxpbmspO1xyXG59XHJcblxyXG5mb3IobGV0IFsgbmFtZSwgZGF0YSBdIG9mIE9iamVjdC5lbnRyaWVzKGpzKSl7XHJcblx0bGV0IG1vZHVsZSA9IHsgZXhwb3J0czoge30gfSxcclxuXHRcdGV2ID0gYChmdW5jdGlvbihtb2R1bGUsZXhwb3J0cyl7JHtkYXRhfS8vIyBzb3VyY2VVUkw9JHtuYW1lfVxcbn0pYDtcclxuXHRcclxuXHQvLyBjb25zb2xlLmxvZyhldik7XHJcblxyXG5cdHRyeXtcclxuXHRcdGV2YWwoZXYpKG1vZHVsZSwgbW9kdWxlLmV4cG9ydHMpO1xyXG5cdH1jYXRjaChlcnIpe1xyXG5cdFx0Y29uc29sZS5lcnJvcihgRXJyb3IgbG9hZGluZyBzY3JpcHQgJHtuYW1lfTpcXG5gLCBlcnIpO1xyXG5cdFx0Ly8gdG9kbzogcG9zdE1lc3NhZ2Ugd3JpdGUgdG8gbG9ncy50eHQgaW4gY2xpZW50IGZvbGRlclxyXG5cdH1cclxufSIsIid1c2Ugc3RyaWN0JztcclxuXHJcbm1vZHVsZS5leHBvcnRzID0gX1JVTlRJTUVfREFUQV87IiwiJ3VzZSBzdHJpY3QnO1xuXG5jbGFzcyBVdGlscyB7XG5cdGlzX2hvc3QodXJsLCAuLi5ob3N0cyl7XG5cdFx0cmV0dXJuIGhvc3RzLnNvbWUoaG9zdCA9PiB1cmwuaG9zdG5hbWUgPT0gaG9zdCB8fCB1cmwuaG9zdG5hbWUuZW5kc1dpdGgoJy4nICsgaG9zdCkpO1xuXHR9XG5cdHJvdW5kKG4sIHIpe1xuXHRcdHJldHVybiBNYXRoLnJvdW5kKG4gKiBNYXRoLnBvdygxMCwgcikpIC8gTWF0aC5wb3coMTAsIHIpO1xuXHR9XG5cdGFkZF9lbGUobm9kZV9uYW1lLCBwYXJlbnQsIGF0dHJpYnV0ZXMgPSB7fSl7XG5cdFx0dmFyIGNydCA9IHRoaXMuY3J0X2VsZShub2RlX25hbWUsIGF0dHJpYnV0ZXMpO1xuXHRcdFxuXHRcdGlmKHR5cGVvZiBwYXJlbnQgPT0gJ2Z1bmN0aW9uJyl0aGlzLndhaXRfZm9yKHBhcmVudCkudGhlbihkYXRhID0+IGRhdGEuYXBwZW5kKGNydCkpO1xuXHRcdGVsc2UgaWYodHlwZW9mIHBhcmVudCA9PSAnb2JqZWN0JyAmJiBwYXJlbnQgIT0gbnVsbCAmJiBwYXJlbnQuYXBwZW5kKXBhcmVudC5hcHBlbmQoY3J0KTtcblx0XHRlbHNlIHRocm93IG5ldyBFcnJvcignUGFyZW50IGlzIG5vdCByZXNvbHZhYmxlIHRvIGEgRE9NIGVsZW1lbnQnKTtcblx0XHRcblx0XHRyZXR1cm4gY3J0O1xuXHR9XG5cdGNydF9lbGUobm9kZV9uYW1lLCBhdHRyaWJ1dGVzID0ge30pe1xuXHRcdHZhciBhZnRlciA9IHt9O1xuXHRcdFxuXHRcdGZvcihsZXQgcHJvcCBpbiBhdHRyaWJ1dGVzKWlmKHR5cGVvZiBhdHRyaWJ1dGVzW3Byb3BdID09ICdvYmplY3QnICYmIGF0dHJpYnV0ZXNbcHJvcF0gIT0gbnVsbClhZnRlcltwcm9wXSA9IGF0dHJpYnV0ZXNbcHJvcF0sIGRlbGV0ZSBhdHRyaWJ1dGVzW3Byb3BdO1xuXHRcblx0XHR2YXIgbm9kZTtcblx0XHRcblx0XHRpZihub2RlX25hbWUgPT0gJ3Jhdycpbm9kZSA9IHRoaXMuY3J0X2VsZSgnZGl2JywgeyBpbm5lckhUTUw6IGF0dHJpYnV0ZXMuaHRtbCB9KS5maXJzdENoaWxkO1xuXHRcdGVsc2UgaWYobm9kZV9uYW1lID09ICd0ZXh0Jylub2RlID0gZG9jdW1lbnQuY3JlYXRlVGV4dE5vZGUoJycpO1xuXHRcdGVsc2Ugbm9kZSA9IGRvY3VtZW50LmNyZWF0ZUVsZW1lbnQobm9kZV9uYW1lKTtcblx0XHRcblx0XHR2YXIgY2xzID0gYXR0cmlidXRlcy5jbGFzc05hbWU7XG5cdFx0XG5cdFx0aWYoY2xzKXtcblx0XHRcdGRlbGV0ZSBhdHRyaWJ1dGVzLmNsYXNzTmFtZTtcblx0XHRcdG5vZGUuc2V0QXR0cmlidXRlKCdjbGFzcycsIGNscyk7XG5cdFx0fVxuXHRcdFxuXHRcdHZhciBldmVudHMgPSBhZnRlci5ldmVudHM7XG5cdFx0XG5cdFx0aWYoZXZlbnRzKXtcblx0XHRcdGRlbGV0ZSBhZnRlci5ldmVudHM7XG5cdFx0XHRcblx0XHRcdGZvcihsZXQgZXZlbnQgaW4gZXZlbnRzKW5vZGUuYWRkRXZlbnRMaXN0ZW5lcihldmVudCwgZXZlbnRzW2V2ZW50XSk7XG5cdFx0fVxuXHRcdFxuXHRcdE9iamVjdC5hc3NpZ24obm9kZSwgYXR0cmlidXRlcyk7XG5cdFx0XG5cdFx0Zm9yKGxldCBwcm9wIGluIGFmdGVyKU9iamVjdC5hc3NpZ24obm9kZVtwcm9wXSwgYWZ0ZXJbcHJvcF0pO1xuXHRcdFxuXHRcdHJldHVybiBub2RlO1xuXHR9XG5cdHdhaXRfZm9yKGNoZWNrLCB0aW1lKXtcblx0XHRyZXR1cm4gbmV3IFByb21pc2UocmVzb2x2ZSA9PiB7XG5cdFx0XHR2YXIgaW50ZXJ2YWwsXG5cdFx0XHRcdHJ1biA9ICgpID0+IHtcblx0XHRcdFx0XHR0cnl7XG5cdFx0XHRcdFx0XHR2YXIgcmVzdWx0ID0gY2hlY2soKTtcblx0XHRcdFx0XHRcdFxuXHRcdFx0XHRcdFx0aWYocmVzdWx0KXtcblx0XHRcdFx0XHRcdFx0aWYoaW50ZXJ2YWwpY2xlYXJJbnRlcnZhbChpbnRlcnZhbCk7XG5cdFx0XHRcdFx0XHRcdHJlc29sdmUocmVzdWx0KTtcblx0XHRcdFx0XHRcdFx0XG5cdFx0XHRcdFx0XHRcdHJldHVybiB0cnVlO1xuXHRcdFx0XHRcdFx0fVxuXHRcdFx0XHRcdH1jYXRjaChlcnIpe2NvbnNvbGUubG9nKGVycil9XG5cdFx0XHRcdH07XG5cdFx0XHRcblx0XHRcdGludGVydmFsID0gcnVuKCkgfHwgc2V0SW50ZXJ2YWwocnVuLCB0aW1lIHx8IDUwKTtcblx0XHR9KTtcblx0fVxuXHRzYW5pdGl6ZShzdHJpbmcpe1xuXHRcdHZhciBub2RlID0gZG9jdW1lbnQuY3JlYXRlRWxlbWVudCgnZGl2Jyk7XG5cdFx0XG5cdFx0bm9kZS50ZXh0Q29udGVudCA9IHN0cmluZztcblx0XHRcblx0XHRyZXR1cm4gbm9kZS5pbm5lckhUTUw7XG5cdH1cblx0dW5zYW5pdGl6ZShzdHJpbmcpe1xuXHRcdHZhciBub2RlID0gZG9jdW1lbnQuY3JlYXRlRWxlbWVudCgnZGl2Jyk7XG5cdFx0XG5cdFx0bm9kZS5pbm5lckhUTUwgPSBzdHJpbmc7XG5cdFx0XG5cdFx0cmV0dXJuIG5vZGUudGV4dENvbnRlbnQ7XG5cdH1cblx0bm9kZV90cmVlKG5vZGVzLCBwYXJlbnQgPSBkb2N1bWVudCl7XG5cdFx0dmFyIG91dHB1dCA9IHtcblx0XHRcdFx0cGFyZW50OiBwYXJlbnQsXG5cdFx0XHR9LFxuXHRcdFx0bWF0Y2hfY29udGFpbmVyID0gL15cXCRcXHMrPj8vZyxcblx0XHRcdG1hdGNoX3BhcmVudCA9IC9eXFxeXFxzKz4/L2c7XG5cdFx0XG5cdFx0Zm9yKHZhciBsYWJlbCBpbiBub2Rlcyl7XG5cdFx0XHR2YXIgdmFsdWUgPSBub2Rlc1tsYWJlbF07XG5cdFx0XHRcblx0XHRcdGlmKHZhbHVlIGluc3RhbmNlb2YgTm9kZSlvdXRwdXRbbGFiZWxdID0gdmFsdWU7XG5cdFx0XHRlbHNlIGlmKHR5cGVvZiB2YWx1ZSA9PSAnb2JqZWN0JylvdXRwdXRbbGFiZWxdID0gdGhpcy5ub2RlX3RyZWUodmFsdWUsIG91dHB1dC5jb250YWluZXIpO1xuXHRcdFx0ZWxzZSBpZihtYXRjaF9jb250YWluZXIudGVzdChub2Rlc1tsYWJlbF0pKXtcblx0XHRcdFx0aWYoIW91dHB1dC5jb250YWluZXIpe1xuXHRcdFx0XHRcdGNvbnNvbGUud2FybignTm8gY29udGFpbmVyIGlzIGF2YWlsYWJsZSwgY291bGQgbm90IGFjY2VzcycsIHZhbHVlKTtcblx0XHRcdFx0XHRjb250aW51ZTtcblx0XHRcdFx0fVxuXHRcdFx0XHRcblx0XHRcdFx0b3V0cHV0W2xhYmVsXSA9IG91dHB1dC5jb250YWluZXIucXVlcnlTZWxlY3Rvcihub2Rlc1tsYWJlbF0ucmVwbGFjZShtYXRjaF9jb250YWluZXIsICcnKSk7XG5cdFx0XHR9ZWxzZSBpZihtYXRjaF9wYXJlbnQudGVzdChub2Rlc1tsYWJlbF0pKXtcblx0XHRcdFx0aWYoIW91dHB1dC5wYXJlbnQpe1xuXHRcdFx0XHRcdGNvbnNvbGUud2FybignTm8gcGFyZW50IGlzIGF2YWlsYWJsZSwgY291bGQgbm90IGFjY2VzcycsIHZhbHVlKTtcblx0XHRcdFx0XHRjb250aW51ZTtcblx0XHRcdFx0fVxuXHRcdFx0XHRcblx0XHRcdFx0b3V0cHV0W2xhYmVsXSA9IG91dHB1dC5wYXJlbnQucXVlcnlTZWxlY3Rvcihub2Rlc1tsYWJlbF0ucmVwbGFjZShtYXRjaF9wYXJlbnQsICcnKSk7XG5cdFx0XHR9ZWxzZSBvdXRwdXRbbGFiZWxdID0gcGFyZW50LnF1ZXJ5U2VsZWN0b3Iobm9kZXNbbGFiZWxdKTtcblx0XHRcdFxuXHRcdFx0aWYoIW91dHB1dFtsYWJlbF0pY29uc29sZS53YXJuKCdObyBub2RlIGZvdW5kLCBjb3VsZCBub3QgYWNjZXNzJywgdmFsdWUpO1xuXHRcdH1cblx0XHRcblx0XHRyZXR1cm4gb3V0cHV0O1xuXHR9XG5cdHN0cmluZ19rZXkoa2V5KXtcblx0XHRyZXR1cm4ga2V5LnJlcGxhY2UoL14oW0EtWl1bYS16XSs/KShbQS1aMC05XVthLXpdKj8pLywgKG1hdGNoLCB0eXBlLCBrZXkpID0+IFsnRGlnaXQnLCAnS2V5J10uaW5jbHVkZXModHlwZSkgPyBrZXkgOiBgJHtrZXl9ICR7dHlwZX1gKTtcblx0fVxuXHRjbG9uZV9vYmoob2JqKXtcblx0XHRyZXR1cm4gSlNPTi5wYXJzZShKU09OLnN0cmluZ2lmeShvYmopKTtcblx0fVxuXHRhc3NpZ25fZGVlcCh0YXJnZXQsIC4uLm9iamVjdHMpe1xuXHRcdGZvcihsZXQgaW5kIGluIG9iamVjdHMpZm9yKGxldCBrZXkgaW4gb2JqZWN0c1tpbmRdKXtcblx0XHRcdGlmKHR5cGVvZiBvYmplY3RzW2luZF1ba2V5XSA9PSAnb2JqZWN0JyAmJiBvYmplY3RzW2luZF1ba2V5XSAhPSBudWxsICYmIGtleSBpbiB0YXJnZXQpdGhpcy5hc3NpZ25fZGVlcCh0YXJnZXRba2V5XSwgb2JqZWN0c1tpbmRdW2tleV0pO1xuXHRcdFx0ZWxzZSBpZih0eXBlb2YgdGFyZ2V0ID09ICdvYmplY3QnICYmIHRhcmdldCAhPSBudWxsKU9iamVjdC5kZWZpbmVQcm9wZXJ0eSh0YXJnZXQsIGtleSwgT2JqZWN0LmdldE93blByb3BlcnR5RGVzY3JpcHRvcihvYmplY3RzW2luZF0sIGtleSkpXG5cdFx0fVxuXHRcdFxuXHRcdHJldHVybiB0YXJnZXQ7XG5cdH1cblx0ZmlsdGVyX2RlZXAodGFyZ2V0LCBtYXRjaCl7XG5cdFx0Zm9yKGxldCBrZXkgaW4gdGFyZ2V0KXtcblx0XHRcdGlmKCEoa2V5IGluIG1hdGNoKSlkZWxldGUgdGFyZ2V0W2tleV07XG5cdFx0XHRcblx0XHRcdGlmKHR5cGVvZiBtYXRjaFtrZXldID09ICdvYmplY3QnICYmIG1hdGNoW2tleV0gIT0gbnVsbCl0aGlzLmZpbHRlcl9kZWVwKHRhcmdldFtrZXldLCBtYXRjaFtrZXldKTtcblx0XHR9XG5cdFx0XG5cdFx0cmV0dXJuIHRhcmdldDtcblx0fVxuXHRyZWRpcmVjdChuYW1lLCBmcm9tLCB0byl7XG5cdFx0dmFyIHByb3h5ID0gU3ltYm9sKCk7XG5cdFx0XG5cdFx0dG8uYWRkRXZlbnRMaXN0ZW5lcihuYW1lLCBldmVudCA9PiB7XG5cdFx0XHRpZihldmVudFtwcm94eV0pcmV0dXJuO1xuXHRcdH0pO1xuXHRcdFxuXHRcdGZyb20uYWRkRXZlbnRMaXN0ZW5lcihuYW1lLCBldmVudCA9PiB0by5kaXNwYXRjaEV2ZW50KE9iamVjdC5hc3NpZ24obmV3KGV2ZW50LmNvbnN0cnVjdG9yKShuYW1lLCBldmVudCksIHtcblx0XHRcdFtwcm94eV06IHRydWUsXG5cdFx0XHRzdG9wSW1tZWRpYXRlUHJvcGFnYXRpb246IGV2ZW50LnN0b3BJbW1lZGlhdGVQcm9wYWdhdGlvbi5iaW5kKGV2ZW50KSxcblx0XHRcdHByZXZlbnREZWZhdWx0OiBldmVudC5wcmV2ZW50RGVmYXVsdC5iaW5kKGV2ZW50KSxcblx0XHR9KSkpO1xuXHR9XG5cdHByb21pc2UoKXtcblx0XHR2YXIgdGVtcCxcblx0XHRcdHByb21pc2UgPSBuZXcgUHJvbWlzZSgocmVzb2x2ZSwgcmVqZWN0KSA9PiB0ZW1wID0geyByZXNvbHZlLCByZWplY3QgfSk7XG5cdFx0XG5cdFx0T2JqZWN0LmFzc2lnbihwcm9taXNlLCB0ZW1wKTtcblx0XHRcblx0XHRwcm9taXNlLnJlc29sdmVfaW4gPSAodGltZSA9IDAsIGRhdGEpID0+IHNldFRpbWVvdXQoKCkgPT4gcHJvbWlzZS5yZXNvbHZlKGRhdGEpLCB0aW1lKTtcblx0XHRcblx0XHRyZXR1cm4gcHJvbWlzZTtcblx0fVxuXHRydG4obnVtYmVyLCB1bml0KXtcblx0XHRyZXR1cm4gKG51bWJlciAvIHVuaXQpLnRvRml4ZWQoKSAqIHVuaXQ7XG5cdH1cbn07XG5cbm1vZHVsZS5leHBvcnRzID0gVXRpbHM7XG4iLCIndXNlIHN0cmljdCc7XG5cbmNsYXNzIFV0aWxzIHtcblx0aXNfaG9zdCh1cmwsIC4uLmhvc3RzKXtcblx0XHRyZXR1cm4gaG9zdHMuc29tZShob3N0ID0+IHVybC5ob3N0bmFtZSA9PSBob3N0IHx8IHVybC5ob3N0bmFtZS5lbmRzV2l0aCgnLicgKyBob3N0KSk7XG5cdH1cblx0cm91bmQobiwgcil7XG5cdFx0cmV0dXJuIE1hdGgucm91bmQobiAqIE1hdGgucG93KDEwLCByKSkgLyBNYXRoLnBvdygxMCwgcik7XG5cdH1cblx0YWRkX2VsZShub2RlX25hbWUsIHBhcmVudCwgYXR0cmlidXRlcyA9IHt9KXtcblx0XHR2YXIgY3J0ID0gdGhpcy5jcnRfZWxlKG5vZGVfbmFtZSwgYXR0cmlidXRlcyk7XG5cdFx0XG5cdFx0aWYodHlwZW9mIHBhcmVudCA9PSAnZnVuY3Rpb24nKXRoaXMud2FpdF9mb3IocGFyZW50KS50aGVuKGRhdGEgPT4gZGF0YS5hcHBlbmQoY3J0KSk7XG5cdFx0ZWxzZSBpZih0eXBlb2YgcGFyZW50ID09ICdvYmplY3QnICYmIHBhcmVudCAhPSBudWxsICYmIHBhcmVudC5hcHBlbmQpcGFyZW50LmFwcGVuZChjcnQpO1xuXHRcdGVsc2UgdGhyb3cgbmV3IEVycm9yKCdQYXJlbnQgaXMgbm90IHJlc29sdmFibGUgdG8gYSBET00gZWxlbWVudCcpO1xuXHRcdFxuXHRcdHJldHVybiBjcnQ7XG5cdH1cblx0Y3J0X2VsZShub2RlX25hbWUsIGF0dHJpYnV0ZXMgPSB7fSl7XG5cdFx0dmFyIGFmdGVyID0ge307XG5cdFx0XG5cdFx0Zm9yKGxldCBwcm9wIGluIGF0dHJpYnV0ZXMpaWYodHlwZW9mIGF0dHJpYnV0ZXNbcHJvcF0gPT0gJ29iamVjdCcgJiYgYXR0cmlidXRlc1twcm9wXSAhPSBudWxsKWFmdGVyW3Byb3BdID0gYXR0cmlidXRlc1twcm9wXSwgZGVsZXRlIGF0dHJpYnV0ZXNbcHJvcF07XG5cdFxuXHRcdHZhciBub2RlO1xuXHRcdFxuXHRcdGlmKG5vZGVfbmFtZSA9PSAncmF3Jylub2RlID0gdGhpcy5jcnRfZWxlKCdkaXYnLCB7IGlubmVySFRNTDogYXR0cmlidXRlcy5odG1sIH0pLmZpcnN0Q2hpbGQ7XG5cdFx0ZWxzZSBpZihub2RlX25hbWUgPT0gJ3RleHQnKW5vZGUgPSBkb2N1bWVudC5jcmVhdGVUZXh0Tm9kZSgnJyk7XG5cdFx0ZWxzZSBub2RlID0gZG9jdW1lbnQuY3JlYXRlRWxlbWVudChub2RlX25hbWUpO1xuXHRcdFxuXHRcdHZhciBjbHMgPSBhdHRyaWJ1dGVzLmNsYXNzTmFtZTtcblx0XHRcblx0XHRpZihjbHMpe1xuXHRcdFx0ZGVsZXRlIGF0dHJpYnV0ZXMuY2xhc3NOYW1lO1xuXHRcdFx0bm9kZS5zZXRBdHRyaWJ1dGUoJ2NsYXNzJywgY2xzKTtcblx0XHR9XG5cdFx0XG5cdFx0dmFyIGV2ZW50cyA9IGFmdGVyLmV2ZW50cztcblx0XHRcblx0XHRpZihldmVudHMpe1xuXHRcdFx0ZGVsZXRlIGFmdGVyLmV2ZW50cztcblx0XHRcdFxuXHRcdFx0Zm9yKGxldCBldmVudCBpbiBldmVudHMpbm9kZS5hZGRFdmVudExpc3RlbmVyKGV2ZW50LCBldmVudHNbZXZlbnRdKTtcblx0XHR9XG5cdFx0XG5cdFx0T2JqZWN0LmFzc2lnbihub2RlLCBhdHRyaWJ1dGVzKTtcblx0XHRcblx0XHRmb3IobGV0IHByb3AgaW4gYWZ0ZXIpT2JqZWN0LmFzc2lnbihub2RlW3Byb3BdLCBhZnRlcltwcm9wXSk7XG5cdFx0XG5cdFx0cmV0dXJuIG5vZGU7XG5cdH1cblx0d2FpdF9mb3IoY2hlY2ssIHRpbWUpe1xuXHRcdHJldHVybiBuZXcgUHJvbWlzZShyZXNvbHZlID0+IHtcblx0XHRcdHZhciBpbnRlcnZhbCxcblx0XHRcdFx0cnVuID0gKCkgPT4ge1xuXHRcdFx0XHRcdHRyeXtcblx0XHRcdFx0XHRcdHZhciByZXN1bHQgPSBjaGVjaygpO1xuXHRcdFx0XHRcdFx0XG5cdFx0XHRcdFx0XHRpZihyZXN1bHQpe1xuXHRcdFx0XHRcdFx0XHRpZihpbnRlcnZhbCljbGVhckludGVydmFsKGludGVydmFsKTtcblx0XHRcdFx0XHRcdFx0cmVzb2x2ZShyZXN1bHQpO1xuXHRcdFx0XHRcdFx0XHRcblx0XHRcdFx0XHRcdFx0cmV0dXJuIHRydWU7XG5cdFx0XHRcdFx0XHR9XG5cdFx0XHRcdFx0fWNhdGNoKGVycil7Y29uc29sZS5sb2coZXJyKX1cblx0XHRcdFx0fTtcblx0XHRcdFxuXHRcdFx0aW50ZXJ2YWwgPSBydW4oKSB8fCBzZXRJbnRlcnZhbChydW4sIHRpbWUgfHwgNTApO1xuXHRcdH0pO1xuXHR9XG5cdHNhbml0aXplKHN0cmluZyl7XG5cdFx0dmFyIG5vZGUgPSBkb2N1bWVudC5jcmVhdGVFbGVtZW50KCdkaXYnKTtcblx0XHRcblx0XHRub2RlLnRleHRDb250ZW50ID0gc3RyaW5nO1xuXHRcdFxuXHRcdHJldHVybiBub2RlLmlubmVySFRNTDtcblx0fVxuXHR1bnNhbml0aXplKHN0cmluZyl7XG5cdFx0dmFyIG5vZGUgPSBkb2N1bWVudC5jcmVhdGVFbGVtZW50KCdkaXYnKTtcblx0XHRcblx0XHRub2RlLmlubmVySFRNTCA9IHN0cmluZztcblx0XHRcblx0XHRyZXR1cm4gbm9kZS50ZXh0Q29udGVudDtcblx0fVxuXHRub2RlX3RyZWUobm9kZXMsIHBhcmVudCA9IGRvY3VtZW50KXtcblx0XHR2YXIgb3V0cHV0ID0ge1xuXHRcdFx0XHRwYXJlbnQ6IHBhcmVudCxcblx0XHRcdH0sXG5cdFx0XHRtYXRjaF9jb250YWluZXIgPSAvXlxcJFxccys+Py9nLFxuXHRcdFx0bWF0Y2hfcGFyZW50ID0gL15cXF5cXHMrPj8vZztcblx0XHRcblx0XHRmb3IodmFyIGxhYmVsIGluIG5vZGVzKXtcblx0XHRcdHZhciB2YWx1ZSA9IG5vZGVzW2xhYmVsXTtcblx0XHRcdFxuXHRcdFx0aWYodmFsdWUgaW5zdGFuY2VvZiBOb2RlKW91dHB1dFtsYWJlbF0gPSB2YWx1ZTtcblx0XHRcdGVsc2UgaWYodHlwZW9mIHZhbHVlID09ICdvYmplY3QnKW91dHB1dFtsYWJlbF0gPSB0aGlzLm5vZGVfdHJlZSh2YWx1ZSwgb3V0cHV0LmNvbnRhaW5lcik7XG5cdFx0XHRlbHNlIGlmKG1hdGNoX2NvbnRhaW5lci50ZXN0KG5vZGVzW2xhYmVsXSkpe1xuXHRcdFx0XHRpZighb3V0cHV0LmNvbnRhaW5lcil7XG5cdFx0XHRcdFx0Y29uc29sZS53YXJuKCdObyBjb250YWluZXIgaXMgYXZhaWxhYmxlLCBjb3VsZCBub3QgYWNjZXNzJywgdmFsdWUpO1xuXHRcdFx0XHRcdGNvbnRpbnVlO1xuXHRcdFx0XHR9XG5cdFx0XHRcdFxuXHRcdFx0XHRvdXRwdXRbbGFiZWxdID0gb3V0cHV0LmNvbnRhaW5lci5xdWVyeVNlbGVjdG9yKG5vZGVzW2xhYmVsXS5yZXBsYWNlKG1hdGNoX2NvbnRhaW5lciwgJycpKTtcblx0XHRcdH1lbHNlIGlmKG1hdGNoX3BhcmVudC50ZXN0KG5vZGVzW2xhYmVsXSkpe1xuXHRcdFx0XHRpZighb3V0cHV0LnBhcmVudCl7XG5cdFx0XHRcdFx0Y29uc29sZS53YXJuKCdObyBwYXJlbnQgaXMgYXZhaWxhYmxlLCBjb3VsZCBub3QgYWNjZXNzJywgdmFsdWUpO1xuXHRcdFx0XHRcdGNvbnRpbnVlO1xuXHRcdFx0XHR9XG5cdFx0XHRcdFxuXHRcdFx0XHRvdXRwdXRbbGFiZWxdID0gb3V0cHV0LnBhcmVudC5xdWVyeVNlbGVjdG9yKG5vZGVzW2xhYmVsXS5yZXBsYWNlKG1hdGNoX3BhcmVudCwgJycpKTtcblx0XHRcdH1lbHNlIG91dHB1dFtsYWJlbF0gPSBwYXJlbnQucXVlcnlTZWxlY3Rvcihub2Rlc1tsYWJlbF0pO1xuXHRcdFx0XG5cdFx0XHRpZighb3V0cHV0W2xhYmVsXSljb25zb2xlLndhcm4oJ05vIG5vZGUgZm91bmQsIGNvdWxkIG5vdCBhY2Nlc3MnLCB2YWx1ZSk7XG5cdFx0fVxuXHRcdFxuXHRcdHJldHVybiBvdXRwdXQ7XG5cdH1cblx0c3RyaW5nX2tleShrZXkpe1xuXHRcdHJldHVybiBrZXkucmVwbGFjZSgvXihbQS1aXVthLXpdKz8pKFtBLVowLTldW2Etel0qPykvLCAobWF0Y2gsIHR5cGUsIGtleSkgPT4gWydEaWdpdCcsICdLZXknXS5pbmNsdWRlcyh0eXBlKSA/IGtleSA6IGAke2tleX0gJHt0eXBlfWApO1xuXHR9XG5cdGNsb25lX29iaihvYmope1xuXHRcdHJldHVybiBKU09OLnBhcnNlKEpTT04uc3RyaW5naWZ5KG9iaikpO1xuXHR9XG5cdGFzc2lnbl9kZWVwKHRhcmdldCwgLi4ub2JqZWN0cyl7XG5cdFx0Zm9yKGxldCBpbmQgaW4gb2JqZWN0cylmb3IobGV0IGtleSBpbiBvYmplY3RzW2luZF0pe1xuXHRcdFx0aWYodHlwZW9mIG9iamVjdHNbaW5kXVtrZXldID09ICdvYmplY3QnICYmIG9iamVjdHNbaW5kXVtrZXldICE9IG51bGwgJiYga2V5IGluIHRhcmdldCl0aGlzLmFzc2lnbl9kZWVwKHRhcmdldFtrZXldLCBvYmplY3RzW2luZF1ba2V5XSk7XG5cdFx0XHRlbHNlIGlmKHR5cGVvZiB0YXJnZXQgPT0gJ29iamVjdCcgJiYgdGFyZ2V0ICE9IG51bGwpT2JqZWN0LmRlZmluZVByb3BlcnR5KHRhcmdldCwga2V5LCBPYmplY3QuZ2V0T3duUHJvcGVydHlEZXNjcmlwdG9yKG9iamVjdHNbaW5kXSwga2V5KSlcblx0XHR9XG5cdFx0XG5cdFx0cmV0dXJuIHRhcmdldDtcblx0fVxuXHRmaWx0ZXJfZGVlcCh0YXJnZXQsIG1hdGNoKXtcblx0XHRmb3IobGV0IGtleSBpbiB0YXJnZXQpe1xuXHRcdFx0aWYoIShrZXkgaW4gbWF0Y2gpKWRlbGV0ZSB0YXJnZXRba2V5XTtcblx0XHRcdFxuXHRcdFx0aWYodHlwZW9mIG1hdGNoW2tleV0gPT0gJ29iamVjdCcgJiYgbWF0Y2hba2V5XSAhPSBudWxsKXRoaXMuZmlsdGVyX2RlZXAodGFyZ2V0W2tleV0sIG1hdGNoW2tleV0pO1xuXHRcdH1cblx0XHRcblx0XHRyZXR1cm4gdGFyZ2V0O1xuXHR9XG5cdHJlZGlyZWN0KG5hbWUsIGZyb20sIHRvKXtcblx0XHR2YXIgcHJveHkgPSBTeW1ib2woKTtcblx0XHRcblx0XHR0by5hZGRFdmVudExpc3RlbmVyKG5hbWUsIGV2ZW50ID0+IHtcblx0XHRcdGlmKGV2ZW50W3Byb3h5XSlyZXR1cm47XG5cdFx0fSk7XG5cdFx0XG5cdFx0ZnJvbS5hZGRFdmVudExpc3RlbmVyKG5hbWUsIGV2ZW50ID0+IHRvLmRpc3BhdGNoRXZlbnQoT2JqZWN0LmFzc2lnbihuZXcoZXZlbnQuY29uc3RydWN0b3IpKG5hbWUsIGV2ZW50KSwge1xuXHRcdFx0W3Byb3h5XTogdHJ1ZSxcblx0XHRcdHN0b3BJbW1lZGlhdGVQcm9wYWdhdGlvbjogZXZlbnQuc3RvcEltbWVkaWF0ZVByb3BhZ2F0aW9uLmJpbmQoZXZlbnQpLFxuXHRcdFx0cHJldmVudERlZmF1bHQ6IGV2ZW50LnByZXZlbnREZWZhdWx0LmJpbmQoZXZlbnQpLFxuXHRcdH0pKSk7XG5cdH1cblx0cHJvbWlzZSgpe1xuXHRcdHZhciB0ZW1wLFxuXHRcdFx0cHJvbWlzZSA9IG5ldyBQcm9taXNlKChyZXNvbHZlLCByZWplY3QpID0+IHRlbXAgPSB7IHJlc29sdmUsIHJlamVjdCB9KTtcblx0XHRcblx0XHRPYmplY3QuYXNzaWduKHByb21pc2UsIHRlbXApO1xuXHRcdFxuXHRcdHByb21pc2UucmVzb2x2ZV9pbiA9ICh0aW1lID0gMCwgZGF0YSkgPT4gc2V0VGltZW91dCgoKSA9PiBwcm9taXNlLnJlc29sdmUoZGF0YSksIHRpbWUpO1xuXHRcdFxuXHRcdHJldHVybiBwcm9taXNlO1xuXHR9XG5cdHJ0bihudW1iZXIsIHVuaXQpe1xuXHRcdHJldHVybiAobnVtYmVyIC8gdW5pdCkudG9GaXhlZCgpICogdW5pdDtcblx0fVxufTtcblxubW9kdWxlLmV4cG9ydHMgPSBVdGlscztcbiIsIi8vIFRoZSBtb2R1bGUgY2FjaGVcbnZhciBfX3dlYnBhY2tfbW9kdWxlX2NhY2hlX18gPSB7fTtcblxuLy8gVGhlIHJlcXVpcmUgZnVuY3Rpb25cbmZ1bmN0aW9uIF9fd2VicGFja19yZXF1aXJlX18obW9kdWxlSWQpIHtcblx0Ly8gQ2hlY2sgaWYgbW9kdWxlIGlzIGluIGNhY2hlXG5cdHZhciBjYWNoZWRNb2R1bGUgPSBfX3dlYnBhY2tfbW9kdWxlX2NhY2hlX19bbW9kdWxlSWRdO1xuXHRpZiAoY2FjaGVkTW9kdWxlICE9PSB1bmRlZmluZWQpIHtcblx0XHRyZXR1cm4gY2FjaGVkTW9kdWxlLmV4cG9ydHM7XG5cdH1cblx0Ly8gQ3JlYXRlIGEgbmV3IG1vZHVsZSAoYW5kIHB1dCBpdCBpbnRvIHRoZSBjYWNoZSlcblx0dmFyIG1vZHVsZSA9IF9fd2VicGFja19tb2R1bGVfY2FjaGVfX1ttb2R1bGVJZF0gPSB7XG5cdFx0Ly8gbm8gbW9kdWxlLmlkIG5lZWRlZFxuXHRcdC8vIG5vIG1vZHVsZS5sb2FkZWQgbmVlZGVkXG5cdFx0ZXhwb3J0czoge31cblx0fTtcblxuXHQvLyBFeGVjdXRlIHRoZSBtb2R1bGUgZnVuY3Rpb25cblx0X193ZWJwYWNrX21vZHVsZXNfX1ttb2R1bGVJZF0obW9kdWxlLCBtb2R1bGUuZXhwb3J0cywgX193ZWJwYWNrX3JlcXVpcmVfXyk7XG5cblx0Ly8gUmV0dXJuIHRoZSBleHBvcnRzIG9mIHRoZSBtb2R1bGVcblx0cmV0dXJuIG1vZHVsZS5leHBvcnRzO1xufVxuXG4iLCIndXNlIHN0cmljdCc7XHJcblxyXG4vKlxyXG5jb25zb2xlLmxvZygnUnVubmluZycpO1xyXG5cclxudmFyIGxvZyA9IGNvbnNvbGUubG9nO1xyXG5PYmplY3QuZGVmaW5lUHJvcGVydHkoY29uc29sZSwgJ2xvZycsIHtcclxuXHRnZXQoKXtcclxuXHRcdHJldHVybiBsb2c7XHJcblx0fSxcclxuXHRzZXQodmFsdWUpe1xyXG5cdFx0cmV0dXJuIHZhbHVlO1xyXG5cdH0sXHJcblx0Y29uZmlndXJhYmxlOiBmYWxzZSxcclxufSk7XHJcbiovXHJcblxyXG5yZXF1aXJlKCcuL0ZpeExvYWQnKTtcclxucmVxdWlyZSgnLi9SZXNvdXJjZXMnKTtcclxuXHJcbnZhciBIVE1MUHJveHkgPSByZXF1aXJlKCcuL0hUTUxQcm94eScpLFxyXG5cdENhdGVnb3J5ID0gcmVxdWlyZSgnLi9NZW51VUkvV2luZG93L0NhdGVnb3J5JyksXHJcblx0SVBDID0gcmVxdWlyZSgnLi9JUEMnKSxcclxuXHRVdGlscyA9IHJlcXVpcmUoJy4vVXRpbHMnKSxcclxuXHRFdmVudHMgPSByZXF1aXJlKCcuL0V2ZW50cycpLFxyXG5cdHV0aWxzID0gbmV3IFV0aWxzKCksXHJcblx0aXBjID0gbmV3IElQQygoLi4uZGF0YSkgPT4gY2hyb21lLndlYnZpZXcucG9zdE1lc3NhZ2UoSlNPTi5zdHJpbmdpZnkoZGF0YSkpKTtcclxuXHJcbmNocm9tZS53ZWJ2aWV3LmFkZEV2ZW50TGlzdGVuZXIoJ21lc3NhZ2UnLCAoeyBkYXRhIH0pID0+IGlwYy5lbWl0KC4uLkpTT04ucGFyc2UoZGF0YSkpKTtcclxuXHJcbmNsYXNzIE1lbnUgZXh0ZW5kcyBFdmVudHMge1xyXG5cdGh0bWwgPSBuZXcgSFRNTFByb3h5KCk7XHJcblx0Y29uZmlnID0gcmVxdWlyZSgnLi9SdW50aW1lJykuY29uZmlnO1xyXG5cdGRlZmF1bHRfY29uZmlnID0gcmVxdWlyZSgnLi4vLi4vQ2xpZW50L0NvbmZpZy5qc29uJyk7XHJcblx0dGFiID0ge1xyXG5cdFx0Y29udGVudDogdGhpcy5odG1sLFxyXG5cdFx0d2luZG93OiB7XHJcblx0XHRcdG1lbnU6IHRoaXMsXHJcblx0XHR9LFxyXG5cdH07XHJcblx0Y29uc3RydWN0b3IoKXtcclxuXHRcdHN1cGVyKCk7XHJcblx0XHRcclxuXHRcdHRoaXMubWFpbigpO1xyXG5cdFx0XHJcblx0XHR2YXIgSW5zdCA9IHRoaXMuY2F0ZWdvcnkoJ0luc3RhbGxhdGlvbicpO1xyXG5cdFx0XHJcblx0XHRJbnN0LmNvbnRyb2woJ0ZvbGRlcicsIHtcclxuXHRcdFx0dHlwZTogJ2Z1bmN0aW9uJyxcclxuXHRcdFx0YnV0dG9uOiAnT3BlbicsXHJcblx0XHRcdHZhbHVlKCl7XHJcblx0XHRcdFx0aXBjLnNlbmQoJ29wZW4gZm9sZGVyJyk7XHJcblx0XHRcdH0sXHJcblx0XHR9KTtcclxuXHRcdFxyXG5cdFx0SW5zdC5jb250cm9sKCdVbmNhcCBGUFMnLCB7XHJcblx0XHRcdHR5cGU6ICdib29sZWFuJyxcclxuXHRcdFx0d2FsazogJ2NsaWVudC51bmNhcF9mcHMnLFxyXG5cdFx0fSkub24oJ2NoYW5nZScsICh2YWx1ZSwgaW5pdCkgPT4gIWluaXQgJiYgdGhpcy5yZWxhdW5jaCgpKTtcclxuXHRcdFxyXG5cdFx0dmFyIEdhbWUgPSB0aGlzLmNhdGVnb3J5KCdHYW1lJyk7XHJcblx0XHRcclxuXHRcdC8vIGxvYWRzIGtydW5rZXIgZnJvbSBhcGkuc3lzMzIuZGV2XHJcblx0XHRHYW1lLmNvbnRyb2woJ0Zhc3QgTG9hZGluZycsIHtcclxuXHRcdFx0dHlwZTogJ2Jvb2xlYW4nLFxyXG5cdFx0XHR3YWxrOiAnZ2FtZS5mYXN0X2xvYWQnLFxyXG5cdFx0fSk7XHJcblx0XHRcclxuXHRcdGZvcihsZXQgY2F0ZWdvcnkgb2YgdGhpcy5jYXRlZ29yaWVzKWNhdGVnb3J5LnVwZGF0ZSh0cnVlKTtcclxuXHR9XHJcblx0cmVsYXVuY2goKXtcclxuXHRcdFxyXG5cdH1cclxuXHRjYXRlZ29yaWVzID0gbmV3IFNldCgpO1xyXG5cdGNhdGVnb3J5KGxhYmVsKXtcclxuXHRcdHZhciBjYXQgPSBuZXcgQ2F0ZWdvcnkodGhpcy50YWIsIGxhYmVsKTtcclxuXHRcdHRoaXMuY2F0ZWdvcmllcy5hZGQoY2F0KTtcclxuXHRcdHJldHVybiBjYXQ7XHJcblx0fVxyXG5cdGFzeW5jIHNhdmVfY29uZmlnKCl7XHJcblx0XHRpcGMuc2VuZCgnc2F2ZSBjb25maWcnLCB0aGlzLmNvbmZpZyk7XHJcblx0fVxyXG5cdGFzeW5jIG1haW4oKXtcclxuXHRcdHZhciBhcnJheSA9IGF3YWl0IHV0aWxzLndhaXRfZm9yKCgpID0+IHR5cGVvZiB3aW5kb3dzID09ICdvYmplY3QnICYmIHdpbmRvd3MpLFxyXG5cdFx0XHRzZXR0aW5ncyA9IGFycmF5WzBdLFxyXG5cdFx0XHRpbmRleCA9IHNldHRpbmdzLnRhYnMubGVuZ3RoLFxyXG5cdFx0XHRnZXQgPSBzZXR0aW5ncy5nZXRTZXR0aW5ncztcclxuXHRcclxuXHRcdHNldHRpbmdzLnRhYnMucHVzaCh7IG5hbWU6ICdDbGllbnQnLCBjYXRlZ29yaWVzOiBbXSB9KTtcclxuXHRcdFxyXG5cdFx0c2V0dGluZ3MuZ2V0U2V0dGluZ3MgPSAoKSA9PiBzZXR0aW5ncy50YWJJbmRleCA9PSBpbmRleCA/IHRoaXMuaHRtbC5nZXQoKSA6IGdldC5jYWxsKHNldHRpbmdzKTtcclxuXHR9XHJcbn07XHJcblxyXG5uZXcgTWVudSgpOyJdLCJuYW1lcyI6W10sInNvdXJjZVJvb3QiOiIifQ==