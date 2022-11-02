/*
 * Chief Client - Adblock
 * v1.0.1
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

  window.FRVR = {
    set(config) {
      window.FRVR = { config };
      throw new Error(
        "This error is safe to ignore! FRVR was blocked from loading."
      );
    },
  };

  // now we have to configure the AD networks ourselves...
  window.canShowAds = false;
  window.useFRANads = false;
  window.useFreestarAds = false;
  window.useAdinplayAds = false;

  window.isForSmallScreens = false;
  window.isMsPwa = false;
  window.isSteamClient = false;

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

exportUserscript({
  renderGameSettings: ({ html, UI, useLocalStorage }) => {
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
            confirm(
              "The game will be reloaded for this setting to take affect."
            )
          )
            location.reload();
        }}
      />
    </${UI.Set} />`;
  },
});
