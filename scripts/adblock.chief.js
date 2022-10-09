/*
 * Chief Client - Adblock
 * v1.0.0
 */

function main() {
  // set default
  if (!localStorage.getItem("adblock enabled"))
    localStorage.setItem("adblock enabled", "on");

  // set at page load
  const enabled = localStorage.getItem("adblock enabled") === "on";

  const adblock =
    /^https:\/\/(?:krunker.io\/libs\/frvr-|cookie-cdn\.cookiepro\.com\/|api.adinplay.com\/|www\.googletagmanager\.com\/|pagead2\.googlesyndication\.com\/pagead\/|a\.pub\.network\/|unpkg\.com\/web3@latest\/dist\/web3\.min\.js)/;

  function filterURL(url) {
    if (!enabled) return false;

    return adblock.test(url.toString());
  }

  new MutationObserver((mutations) => {
    for (const mutation of mutations) {
      for (const node of mutation.addedNodes)
        if (node instanceof HTMLScriptElement && filterURL(node.src))
          node.remove();
    }
  }).observe(document.documentElement, {
    childList: true,
    subtree: true,
  });

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

function Settings() {
  const [localEnabled, setLocalEnabled] = useLocalStorage("adblock enabled");

  return html`
  <${UI.Set} title="Adblock">
    <${UI.SwitchControl}
      title="Adblock"
      description="Requires Restart"
      attention
      defaultChecked=${localEnabled === "on"}
      onChange=${(event) => {
        const newValue = event.currentTarget.checked ? "on" : "off";
        setLocalEnabled(newValue);
        location.reload();
      }}
    />
  </${UI.Set} />`;
}

exportUserscript({
  main,
  Settings,
});
