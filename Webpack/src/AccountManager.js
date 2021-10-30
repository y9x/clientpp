'use strict';

var utils = require('./libs/utils'),
	site = require('./Site'),
	Window = require('./libs/MenuUI/Window'),
	{ tick } = require('./libs/MenuUI/consts');

class Menu {
	window = new Window(this);
	save_config(){}
	async attach(){
		var sin = await utils.wait_for(() => document.querySelector('#signedInHeaderBar')),
			sout = await utils.wait_for(() => document.querySelector('#signedOutHeaderBar'));
		
		tick(utils.add_ele('div', sin, {
			className: 'button buttonG lgn',
			style: {
				width: '300px',
				'margin-right': 0,
				'padding-top': '5px',
				'margin-left': '5px',
				'padding-bottom': '13px',
			},
			innerHTML: `Accounts <span class="material-icons" style="vertical-align:middle;color: #fff;font-size:36px;margin-top:-8px;">switch_account</span>`,
			events: {
				click: () => this.window.show(),
			},
		}));
		
		tick(utils.add_ele('div', sout, {
			className: 'button buttonG lgn',
			style: {
				width: '200px',
				'margin-right': 0,
				'padding-top': '5px',
				'margin-left': '5px',
				'padding-bottom': '13px',
			},
			innerHTML: `Accounts <span class="material-icons" style="vertical-align:middle;color: #fff;font-size:36px;margin-top:-8px;">switch_account</span>`,
			events: {
				click: () => this.window.show(),
			},
		}));
		
		this.window.attach(await utils.wait_for(() => document.querySelector('#uiBase')));
	}
	constructor(){
		this.attach();
	}
};

new Menu();