"use strict";

const userscript = {
  // Name of the script
  name: "Selectable Chat",
  // Name of the author
  author: "Mixaz",
  // Version of the script
  version: "1.0.0",
  // Allowed values: "all" | "game" | "social" | "viewer" | "editor"
  locations: ["game"],
  // Callback function to run
  // The first argument is an instance of electron-store to allow easier config operations (not used in this example)
  // run: (config) => {...}
  run: () => {
    const newCSS = document.createElement("style");

    newCSS.id = "selectableChat";
    newCSS.innerHTML = "#chatList * { user-select: text; }";

    document.addEventListener("DOMContentLoaded", () =>
      document.head.appendChild(newCSS)
    );
  },
};

module.exports = userscript;
