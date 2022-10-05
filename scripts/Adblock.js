const metadata = {
	"name": "Adblock",
	"author": "Chief Software",
	"description": "Adblocking script",
	"version": 0.1,
	"locations": ["all"],
	"features": {
		"block": "^https:\\/\\/(?:krunker.io\\/libs\\/frvr-|cookie-cdn\\.cookiepro\\.com\\/|api.adinplay.com\\/|www\\.googletagmanager\\.com\\/|pagead2\\.googlesyndication\\.com\\/pagead\\/|a\\.pub\\.network\\/|unpkg\\.com\\/web3@latest\\/dist\\/web3\\.min\\.js)"
	}
};

window.FRVR = {
	init: () => {},
	lifecycle: {},
};

const style = document.createElement("style");
style.textContent = "#adCon, *[id*='aHider'] { display: none !IMPORTANT; }";

document.addEventListener('DOMContentLoaded', () => {
	document.documentElement.append(style);
});
