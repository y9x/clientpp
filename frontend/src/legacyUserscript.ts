/* eslint-disable no-new-func */
/**
 * Legacy IDKR userscript
 */
import { ipcConsole } from "./ipc";
import currentSite from "./site";

interface LegacyUserscriptExports {
  run: () => unknown;
  name: string;
  version: string;
  author: string;
  description: string;
  locations: ("game" | "social" | "editor" | "all")[];
  platforms: string[];
  settings: { used: boolean };
}

class LegacyUserscript {
  data: LegacyUserscriptExports = {
    run: () => {
      //
    },
    name: "Unnamed userscript",
    version: "Unknown version",
    author: "Unknown author",
    description: "No description provided",
    locations: ["game"],
    platforms: ["all"],
    settings: { used: false },
  };
  constructor(data: LegacyUserscriptExports) {
    for (const key in this.data) {
      const compare = this.data[key as keyof LegacyUserscriptExports];
      const value = data[key as keyof LegacyUserscriptExports];

      if (typeof compare === typeof value)
        (
          this.data as unknown as Record<
            string,
            LegacyUserscriptExports[keyof LegacyUserscriptExports]
          >
        )[key] = compare;
    }
  }
  async run() {
    if (
      !this.data.locations.includes("all") &&
      currentSite &&
      !this.data.locations.includes(currentSite)
    )
      return false;

    try {
      await this.data.run();
      return true;
    } catch (err) {
      console.error(err);
      return false;
    }
  }
}

export default function evalLegacyUserscript(name: string, data: string) {
  // legacy idkr, unknown
  // quick fix
  if (data.includes("// ==UserScript==") && currentSite !== "game") return;

  const module = { exports: {} as Partial<LegacyUserscriptExports> };
  let func;
  const context = {
    module,
    exports: module.exports,
    console,
  };

  try {
    func = new Function("script", ...Object.keys(context), "eval(script);");
  } catch (err) {
    ipcConsole.warn(`Error parsing userscript: ${name}\n`, err);
    console.error(`Error parsing userscript ${name}:\n${err}`);
    return;
  }

  try {
    func(data + `\n//# sourceURL=${name}`, ...Object.values(context));

    const userscript = new LegacyUserscript(
      module.exports as LegacyUserscriptExports
    );

    userscript.run();
  } catch (err) {
    console.warn(`Error executing userscript: ${name}\n`, err);
    ipcConsole.error(`Error executing userscript ${name}:\n${err}`);
    return;
  }
}
