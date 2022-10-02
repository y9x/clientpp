import { ipc } from "./IPC";
import console from "./Console";
import utils from "./libs/Utils";

export default class ChiefUserscript {
  constructor(name, metadata) {
    this.name = name;
    this.metadata = metadata;

    // utils.assign_type(this.template, metadata);
    // done in host c++

    const { libs } = this.metadata.features;

    if (libs.utils) libs.utils = utils;
  }
  // returns false if the script failed to execute, otherwise true
  async run(script, site, menu) {
    if (!this.metadata.locations.some((s) => s == site || s == "all"))
      return false;

    const exports = {};
    let run;
    const context = { _metadata: this.metadata, exports, console };

    try {
      // cannot use import/export, fix soon
      run = new Function("script", ...Object.keys(context), `eval(script)`);
    } catch (err) {
      console.warn(`Error parsing userscript: ${this.name}\n`, err);
      ipc.console.error(`Error parsing userscript ${this.name}:\n${err}`);
      return false;
    }

    if (menu) {
      const { userscripts } = menu.config,
        { author, features } = this.metadata;

      Object.defineProperty(features, "config", {
        get() {
          return userscripts[author];
        },
      });
    }

    try {
      run(
        `${script}\n//# sourceURL=userscript://./${this.name}`,
        ...Object.values(context)
      );
    } catch (err) {
      console.error(`Error executing userscript ${this.name}:\n`, err);
      ipc.console.error(`Error executing userscript ${name}:\n${err}`);
      return false;
    }

    const { gui } = this.metadata.features;

    if (menu)
      for (const [labelct, controls] of Object.entries(gui)) {
        let category;

        // use existing category when possible
        for (const ct of menu.categories)
          if (ct.label == labelct) category = ct;
        if (!category) category = menu.category(labelct);

        for (const [labelco, data] of Object.entries(controls)) {
          const change_callback = data.change;

          delete data.change;

          if (typeof data.walk == "string")
            data.walk = `userscripts.${this.metadata.author}.${data.walk}`;

          const control = category.control(labelco, data);

          if (change_callback)
            control.on("change", (value, init) => {
              const func = exports[change_callback];

              if (func) func(value, init);
            });
        }
      }

    return true;
  }
}
