/**
 * Native "Chief" userscript
 */
/* eslint-disable no-new-func */
import { ipcConsole } from "./ipc";
import console from "./Console";
import type { Userscript } from "./userscript";
import currentSite from "./site";
import menu from "./menu";

export default function evalChiefUserscript(
  name: string,
  metadata: Userscript,
  code: string
) {
  // done in host c++

  // returns false if the script failed to execute, otherwise true
  if (
    Array.isArray(metadata.locations) &&
    !metadata.locations.some((s) => s === currentSite || s === "all")
  )
    return false;

  const exports: Record<string | symbol, (() => unknown) | unknown> = {};
  const context = { _metadata: metadata, exports, console: ipcConsole };
  let run;

  try {
    // cannot use import/export, fix soon
    run = new Function("script", ...Object.keys(context), "eval(script)");
  } catch (err) {
    console.warn(`Error parsing userscript: ${name}\n`, err);
    ipcConsole.error(`Error parsing userscript ${name}:\n${err}`);
    return false;
  }

  if (menu) {
    const { userscripts } = menu.config;
    const { author, features } = metadata;

    Object.defineProperty(features, "config", {
      get() {
        return userscripts[author];
      },
    });
  }

  try {
    run(
      `${code}\n//# sourceURL=userscript://./${name}`,
      ...Object.values(context)
    );
  } catch (err) {
    console.error(`Error executing userscript ${name}:\n`, err);
    ipcConsole.error(`Error executing userscript ${name}:\n${err}`);
    return false;
  }

  /*if (metadata.features?.gui && menu)
    for (const [labelct, controls] of Object.entries(metadata.features.gui)) {
      let category;

      // use existing category when possible
      for (const ct of menu.categories) if (ct.label === labelct) category = ct;
      if (!category) category = menu.category(labelct);

      for (const [labelco, data] of Object.entries(controls)) {
        const change_callback = data.change;

        delete data.change;

        if (typeof data.walk === "string")
          data.walk = `userscripts.${metadata.author}.${data.walk}`;

        const control = category.control(labelco, data);

        if (change_callback)
          control.on("change", (value, init) => {
            const func = exports[change_callback];

            if (typeof func === "function") func(value, init);
          });
      }
    }*/

  return true;
}
