'use strict';

const metadata = {
	"name": "Sky Color",
	"author": "Chief Software",
	"description": "Adds a sky color picker to your client's settings.",
	"version": 0.2,
	"locations": ["all"],
	"features": {
		"gui": {
			"Sky Color": {
				"Enabled": {
					"type": "boolean",
					"walk": "skycolor.enabled",
					"change": "skycolor_change"
				},
				"Color": {
					"type": "color",
					"walk": "skycolor.color",
					"change": "skycolor_change"
				}
			},
		},
		"config": {
			"skycolor": {
				"enabled": true,
				"color": "#64d9a8"
			}
		}
	}
};

var { config } = metadata.features,
	color;

Object.defineProperty(Object.prototype, 'skyCol', {
	get(){
		return config.skycolor.color;
	},
	set(value){
		return color = value;
	},
});