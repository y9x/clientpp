'use strict';

var { config, js } = require('./Runtime'),
	{ meta } = require('./consts'),
	Loader = require('./libs/Loader');

if(config.game.fast_load && !js.length){
	let loader = new Loader();
	
	loader.observe();
	
	loader.license(meta);
	
	loader.load({}, {});
}