/*
 * This is not a userscript intended to be used outside of debugging. This serves as a test and a reference.
 */
"use strict";

class Userscript {
  constructor() {
    this.meta = {
      name: "Hello World",
      version: "1.0a",
      author: "CreepSore",
      description: "Echos Hello World as Alert or as Console print",
    };
    this.config = {
      apiversion: "1.0",
      locations: ["all"],
      platforms: ["all"],
      settings: {
        outputType: {
          id: "outputType",
          name: "Output Type",
          cat: "Misc",
          type: "select",
          options: {
            alert: "Alert",
            document: "Document",
          },
          val: "alert",
          html: () => {
            return this.clientUtils.genCSettingsHTML(
              this.config.settings["outputType"]
            );
          },
        },
      },
    };

    // ############################################ //
    //  Injected Properties by UserscriptInitiator  //
    // ############################################ //
    this.clientUtils = null;
  }

  load(config) {
    let setting = config.get("outputType");
    switch (setting) {
      case "document":
        document.write("Hello World!");
        break;
      case "alert":
      default:
        alert("Hello World!");
        break;
    }
  }

  unload() {}
}

return new Userscript();
