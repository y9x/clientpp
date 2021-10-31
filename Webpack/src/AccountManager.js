'use strict';

var utils = require('./libs/utils'),
	site = require('./Site'),
	Control = require('./libs/MenuUI/Control'),
	Events = require('./libs/Events'),
	HeaderWindow = require('./libs/MenuUI/HeaderWindow'),
	{ IM, ipc } = require('./IPC'),
	{ tick } = require('./libs/MenuUI/Sound');

require('./TableControl');

class AccountPop {
	constructor(body, callback){
		this.callback = callback;
		
		this.container = utils.add_ele('div', body, {
			id: 'importPop',
			style: { 'z-index': 1e9 },
		});
		
		utils.add_ele('div', this.container, {
			className: 'pubHed',
			textContent: 'Add Account',
		});
		
		this.username = utils.add_ele('input', this.container, {
			type: 'text',
			placeholder: 'Enter Username',
			className: 'inputGrey2',
			style: { width: '379px', 'margin-bottom': '5px', },
		});
		
		this.password = utils.add_ele('input', this.container, {
			type: 'text',
			placeholder: 'Enter Password',
			className: 'inputGrey2',
			style: { width: '379px' },
		});
		
		utils.add_ele('div', this.container, {
			className: 'mapFndB',
			textContent: 'Add',
			style: {
				width: '180px',
				'margin-top': '10px',
				'margin-left': 0,
				background: '#4af',
			},
			events: {
				click: () => {
					try{
						this.callback(this.username.value, this.password.value);
					}catch(err){ console.error(err); }
					
					this.username.value = '';
					this.password.value = '';
					this.hide();
				},
			},
		});
		
		utils.add_ele('div', this.container, {
			className: 'mapFndB',
			textContent: 'Cancel',
			style: {
				width: '180px',
				'margin-top': '10px',
				'margin-left': 0,
				background: '#122',
			},
			events: { click: () => this.hide() },
		});
		
		this.hide();
	}
	show(){
		this.container.style.display = 'block';
	}
	hide(){
		this.container.style.display = 'none';
	}
};

class Menu extends Events {
	save_config(){}
	config = {};
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
		
		tick(utils.add_ele('div', () => document.querySelector('#signedInHeaderBar'), opts));
		tick(utils.add_ele('div', () => document.querySelector('#signedOutHeaderBar'), opts));
	}
	async generate(){
		var list = await ipc.post(IM.account_list);
		
		for(let [ name, data ] of Object.entries(list).sort((p1, p2) => p1.order - p2.order)){
			
		}
	}
	constructor(){
		super();
		
		this.account_pop = new AccountPop(this.window.node, (username, password) => {
			if(!username || !password)return;
			
			
		});
		
		this.table = this.window.control('', { type: 'table' });
		
		this.window.control('Account', {
			type: 'function',
			text: 'Add',
			color: 'blue',
			value: () => this.account_pop.show(),
		});
		
		this.window.on('hide', () => this.account_pop.hide());
		
		this.generate();
	}
};

var menu = new Menu();
window.menu = menu;
menu.attach();