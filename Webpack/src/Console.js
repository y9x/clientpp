'use strict';

// script executes before devtools listens on logs

var methods = ['log','info','warn','error','debug','trace'],
	initial = {},
	buffer = {},
	tempcall = (type, ...data) => {
		
	};

for(let method of methods){
	initial[method] = console[method].bind(console);
	exports[method] = (...data) => {
		if(!buffer[method])buffer[method] = [];
		buffer[method].push(data);
	};
}

// devtools hooks after

setTimeout(() => {
	for(let method of methods){
		exports[method] = initial[method];
		if(buffer[method])for(let data of buffer[method])exports[method](...data);
	}
	
	buffer = null;
	initial = null;
});