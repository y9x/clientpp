'use strict';

var { css, js } = require('./Runtime'),
	site_location = require('./SiteLocation'),
	utils = require('./libs/Utils'),
	ipc = require('./IPC'),
	Userscript = require('./Userscript'),
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
	subtree: true,
	childList: true,
});

for(let [ name, data ] of Object.entries(js)){
	// quick fix
	if(data.includes('// ==UserScript==') && site_location != 'game')continue;
	
	let module = { exports: {} },
		func,
		context = {
			module,
			exports: module.exports,
			// console: ipc.console,
		};
	
	try{
		func = eval(`(function(${Object.keys(context)}){${data}//# sourceURL=${name}\n})`);
	}catch(err){
		console.error('Error parsing UserScript:', name, '\n', err);
	}
	
	
	try{
		func(...Object.values(context));
		
		let userscript = new Userscript(module.exports);
		
		userscript.run();
	}catch(err){
		console.warn('Error executing UserScript:', name, '\n', err);
	}
}