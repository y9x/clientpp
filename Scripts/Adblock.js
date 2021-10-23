'use strict';

const metadata = {
	"name": "Adblock",
	"author": "Chief Software",
	"description": "Adblocking script",
	"version": 0.1,
	"locations": ["all"],
	"features": {
		"block_hosts": [
			"cookie-cdn.cookiepro.com",
			"googletagmanager.com",
			"googlesyndication.com",
			"pub.network",
			"paypalobjects.com",
			"doubleclick.net",
			"adinplay.com",
			"syndication.twitter.com"
		],
		"gui": {
			"Adblock": {
				"Enabled": {
					"type": "boolean",
					"walk": "adblock",
					"change": "adblock_change"
				}
			}
		},
		"libs": {
			"utils": true
		},
		"config": {
			"adblock": true
		}
	}
};

// destructure
const { config, libs } = metadata.features;

if(config.adblock)libs.utils.add_ele('style', () => document.documentElement, {
	textContent: `*[id*='aHider'] { display: none !IMPORTANT; }`,
});

export function adblock_change(value, init){
	if(!init)location.reload();
}