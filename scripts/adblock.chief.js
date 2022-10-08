/*
 * Chief Client - Adblock
 * v1.0.0
 */

function main() {
  const style = document.createElement("style");
  style.textContent = "#adCon, *[id*='aHider'] { display: none !IMPORTANT; }";

  document.addEventListener("DOMContentLoaded", () => {
    document.documentElement.append(style);
  });
}

// set default
if (!localStorage.getItem("adblock enabled"))
  localStorage.setItem("adblock enabled", "on");

// set at page load
const enabled = localStorage.getItem("adblock enabled") === "on";

const adblock =
  /^https:\/\/(?:cdn\.frvr\.com\/|imasdk\.googleapis\.com\/|cookie-cdn\.cookiepro\.com\/|api.adinplay.com\/|www\.googletagmanager\.com\/|pagead2\.googlesyndication\.com\/pagead\/|a\.pub\.network\/|unpkg\.com\/web3@latest\/dist\/web3\.min\.js)/;

function filterURL(url) {
  if (!enabled) return false;

  return adblock.test(url.toString());
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
  filterURL,
  Settings,
});
