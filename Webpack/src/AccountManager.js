'use strict';

var utils = require('./libs/utils'),
	site = require('./Site'),
	Control = require('./libs/MenuUI/Control'),
	Events = require('./libs/Events'),
	HeaderWindow = require('./libs/MenuUI/HeaderWindow'),
	Hex3 = require('./libs/Hex3'),
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

class AccountTile {
	constructor(node, window, username, data){
		this.data = data;
		this.username = username;
		this.window = window;
		
		this.create(node);
	}
	create(node){
		var hex = new Hex3(this.data.color);
		
		this.container = utils.add_ele('div', node, {
			className: 'account-tile',
		});
		
		this.label = utils.add_ele('div', this.container, {
			className: 'text',
			textContent: this.username,
			events: {
				click: this.click.bind(this),
			},
			style: {
				'background-color': `rgba(${hex.hex}, var(--alpha))`,
			},
		});
		
		this.buttons = utils.add_ele('div', this.container, {
			className: 'buttons',
		});
		
		utils.add_ele('span', this.buttons, {
			className: 'edit material-icons',
			textContent: 'edit',
			events: {
				click: this.edit.bind(this),
			},
		});
		
		utils.add_ele('span', this.buttons, {
			className: 'delete material-icons',
			textContent: 'delete',
			events: {
				click: this.delete.bind(this),
			},
		});
	}
	delete(){
		ipc.send(IM.account_remove, this.username);
	}
	edit(){
		
	}
	async click(){
		var password = await ipc.post(IM.account_password, this.username);
		window.accName = { value: this.username };
		window.accPass = { value: this.data.password };
		loginAcc();
		delete window.accName;
		delete window.accPass;
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
	async generate(list){
		for(let node of this.table.node.children)node.remove();
		
		for(let [ username, data ] of Object.entries(list).sort((p1, p2) => p1.order - p2.order))new AccountTile(this.table.node, this.window, username, data);
	}
	constructor(){
		super();
		
		this.account_pop = new AccountPop(this.window.node, (username, password) => {
			if(!username || !password)return;
			
			ipc.send(IM.account_set_password, username, password, '#2196f3', 0);
		});
		
		this.table = this.window.control('', { type: 'table' });
		
		this.window.control('Account', {
			type: 'function',
			text: 'Add',
			color: 'blue',
			value: () => this.account_pop.show(),
		});
		
		this.window.on('hide', () => this.account_pop.hide());
		
		utils.add_ele('style', this.window.node, {
			textContent: require('./AccountManager.css'),
		});
		
		ipc.post(IM.account_list).then(list => this.generate(list));
	}
};

var menu = new Menu();
window.menu = menu;
menu.attach();

ipc.on(IM.account_regen, list => menu.generate(list));