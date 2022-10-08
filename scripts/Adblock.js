/*
 * Chief Client - Adblock
 * v1.0.0
 */

function main() {
  window.FRVR = {
    init: () => {},
    lifecycle: {},
  };

  const style = document.createElement("style");
  style.textContent = "#adCon, *[id*='aHider'] { display: none !IMPORTANT; }";

  document.addEventListener("DOMContentLoaded", () => {
    document.documentElement.append(style);
  });
}

function filterURL(url) {
  return /^https:\/\/(?:krunker.io\/libs\/frvr-|cookie-cdn\.cookiepro\.com\/|api.adinplay.com\/|www\.googletagmanager\.com\/|pagead2\.googlesyndication\.com\/pagead\/|a\.pub\.network\/|unpkg\.com\/web3@latest\/dist\/web3\.min\.js)/.test(
    url.toString()
  );
}

exportUserscript({
  main,
  filterURL,
});
