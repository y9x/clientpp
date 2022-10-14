/*
 * Chief Client - Adblock
 * v1.0.0
 */

// set default
if (!localStorage.getItem("adblock enabled"))
  localStorage.setItem("adblock enabled", "on");

// set at page load
const enabled = localStorage.getItem("adblock enabled") === "on";

if (enabled) {
  const style = document.createElement("style");
  style.textContent = "#adCon, *[id*='aHider'] { display: none !IMPORTANT; }";

  document.addEventListener("DOMContentLoaded", () => {
    document.documentElement.append(style);
  });

  const shimFRVR = Object.freeze({
    init: () => {},
    lifecycle: Object.freeze({}),
    tracker: Object.freeze({
      addExtraFieldFunction: () => {},
      logEvent: () => {},
    }),
    bootstrapper: Object.freeze({
      init: () => Promise.resolve(),
      setProgress: () => {},
      complete: () => {},
    }),
  });

  // :((((
  // we break the AD loader in head by making this non-configurable :(
  Object.defineProperty(window, "FRVR", {
    get: () => shimFRVR,
    set: (value) => {
      // krunker sets FRVR = window.FRVR || {}
      // we can detect this expression by checking if the value is equal to window.FRVR
      if (value === shimFRVR) {
        throw new Error(
          "This error is safe to ignore! FRVR was blocked from loading."
        );
      }
    },
  });

  // now we have to configure the AD networks ourselves...
  window.canShowAds = false;
  window.useFRANads = false;
  window.useFreestarAds = false;
  window.useAdinplayAds = false;

  // and the app restrictions...
  window.canShowExternalLinks = true;
  window.canShowPaypal = false;
  window.canShowSocialHub = true;
  window.canShowMods = true;
  window.canConnectExternalAccounts = true;
  window.canShowMarketplace = true;
  window.canShowNFTs = true;
  window.canShowTwitch = true;
  window.canShowKrunkerEngine = true;
  window.isFrvrDotCom = false;
}

function Settings() {
  const [localEnabled, setLocalEnabled] = useLocalStorage("adblock enabled");

  return html`
  <${UI.Set} title="Adblock">
    <${UI.Switch}
      title="Adblock"
      description="Requires Restart"
      attention
      defaultChecked=${localEnabled === "on"}
      onChange=${(event) => {
        const newValue = event.currentTarget.checked ? "on" : "off";
        setLocalEnabled(newValue);
        if (
          confirm("The game will be reloaded for this setting to take affect.")
        )
          location.reload();
      }}
    />
  </${UI.Set} />`;
}

exportUserscript({
  Settings,
});
