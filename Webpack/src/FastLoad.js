'use strict';

var { config } = require('./Runtime'),
	Loader = require('./libs/Loader');;

if(config.game.fast_load){
	let loader = new Loader();
	
	loader.observe();
	
	loader.license({
		github: 'https://github.com/y9x/',
		discord: 'https://y9x.github.io/discord/',
		forum: 'https://forum.sys32.dev/',
	});
	
	loader.load({}, {});
}