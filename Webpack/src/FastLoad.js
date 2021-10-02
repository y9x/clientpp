'use strict';

var { config, js } = require('./Runtime'),
	Loader = require('./libs/Loader');

if(config.game.fast_load && !js.length){
	let loader = new Loader();
	
	loader.observe();
	
	loader.license({
		github: 'https://github.com/y9x/',
		discord: 'https://y9x.github.io/discord/',
		forum: 'https://forum.sys32.dev/',
	});
	
	loader.load({}, {});
}