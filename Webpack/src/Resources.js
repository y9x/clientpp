'use strict';

var { site_location, utils } = require('./Consts'),
	{ css, js } = require('./Runtime'),
	ipc = require('./IPC'),
	IPCConsole = require('./IPCConsole'),
	Userscript = require('./Userscript');

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