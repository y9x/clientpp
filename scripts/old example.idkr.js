module.exports = {
  // Name of the script
  name: "Selectable Chat",

  // Name of the author
  author: "Mixaz",

  // Version of the script
  version: "1.0.0",

  // Allowed values: "all" | "docs" | "game" | "social" | "viewer" | "editor"
  locations: ["game"],

  settings: {
    disableFrameRateLimit: {
      name: "Disable Frame Rate Limit",
      id: "disableFrameRateLimit",
      cat: "Performance",
      type: "checkbox",
      val: true,
      needsRestart: true,
      html() {
        return clientUtils.genCSettingsHTML(this);
      },
    },
  },

  // Callback function to run
  // The first argument is an instance of electron-store to allow easier config operations (not used in this example)
  run: (config) => {
    console.log(
      "disableFrameRateLimit:",
      config.get("disableFrameRateLimit", true)
    );

    const newCSS = Object.assign(document.createElement("style"), {
      id: "selectableChat",
      innerHTML: `#chatList * {
				user-select: text;
			}`,
    });
    document.addEventListener("DOMContentLoaded", () =>
      document.head.appendChild(newCSS)
    );
  },
};
