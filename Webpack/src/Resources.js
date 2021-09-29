'use strict';

var Utils = require('./libs/Utils'),
	utils = new Utils();

// wait for krunker css
// utils.wait_for(() => document.querySelector(`link[href*='/css/main_custom.css']`))

var { css, js } = require('./Runtime');

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