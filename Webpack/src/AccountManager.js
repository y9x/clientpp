'use strict';

var utils = require('./libs/utils'),
	site = require('./Site'),
	Control = require('./libs/MenuUI/Control'),
	Events = require('./libs/Events'),
	HeaderWindow = require('./libs/MenuUI/HeaderWindow'),
	{ IM, ipc } = require('./IPC'),
	{ tick } = require('./libs/MenuUI/Sound');

require('./TableControl');

class Menu extends Events {
	save_config(){}
	config = { test: {} };
	window = new HeaderWindow(this, 'Accounts');
	async attach(){
		var opts = {
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
		};
		
		var sin = await utils.wait_for(() => document.querySelector('#signedInHeaderBar')),
			sout = await utils.wait_for(() => document.querySelector('#signedOutHeaderBar'));
		
		tick(utils.add_ele('div', sin, opts));
		tick(utils.add_ele('div', sout, opts));
		
		this.window.attach(await utils.wait_for(() => document.querySelector('#uiBase')));
	}
	async generate(){
		var list = await ipc.post(IM.account_list);
		
		for(let [ name, data ] of Object.entries(list).sort((p1, p2) => p1.order - p2.order)){
			
		}
	}
	constructor(){
		super();
		
		this.table = this.window.control('', { type: 'table' });
		
		this.generate();
	}
};

var menu = new Menu();
menu.attach();