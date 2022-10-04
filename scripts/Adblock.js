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
		]
	}
};

const style = document.createElement("style");
style.textContent = "*[id*='aHider'] { display: none !IMPORTANT; }";

document.addEventListener('DOMContentLoaded', () => {
	document.documentElement.append(style);
});
