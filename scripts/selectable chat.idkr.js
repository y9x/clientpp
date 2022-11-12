"use strict";

function makeChatSelectable() {
  const newCSS = document.createElement("style");

  newCSS.id = "selectableChat";
  newCSS.innerHTML = "#chatList * { user-select: text; }";

  document.addEventListener("DOMContentLoaded", () =>
    document.head.appendChild(newCSS)
  );
}

module.exports = {
  // Name of the script
  name: "Selectable Chat",
  // Name of the author
  // This script was reworked to be more functional for the purpose of demonstrating IDKR userscripts in Chief Client.
  // Mixaz was author of the original script.
  author: "Mixaz",
  // Version of the script
  version: "1.0.0",
  // Allowed values: "all" | "game" | "social" | "viewer" | "editor"
  locations: ["game"],
  // Settings
  settings: {
    selectableChat: {
      name: "Selectable Chat",
      id: "selectableChat",
      cat: "Mixaz",
      type: "checkbox",
      val: true,
      needsRestart: true,
      html() {
        return clientUtils.genCSettingsHTML(this);
      },
    },
  },
  // Callback function to run
  // The first argument is an instance of electron-store to allow easier config operations
  run(config) {
    // config.get(config key, default value)
    if (
      config.get(
        this.settings.selectableChat.id,
        this.settings.selectableChat.val
      )
    )
      makeChatSelectable();
  },
};
