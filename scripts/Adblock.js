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
		"libs": {
			"utils": true
		}
	}
};

// destructure
const { libs } = metadata.features;

libs.utils.add_ele('style', () => document.documentElement, {
	textContent: `*[id*='aHider'] { display: none !IMPORTANT; }`,
});