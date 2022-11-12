/*
 * IDKR Userscript support (non-game)
 */
import EventEmitter from "../EventEmitter";
import currentSite from "../site";
import type { IClientUtil } from "./idkrCommon";
import { createConfig, executeUserScript } from "./idkrCommon";

const clientUtils: IClientUtil = Object.freeze({
  events: EventEmitter,
  genCSettingsHTML: () => () => undefined,
});

/**
 * Run an IDKR userscript
 * @param script The userscript's path in the filesystem (for debugging purposes).
 * @param scriptID The unique identifier of the userscript.
 * @param code The userscript's source code.
 */
export default function idkrRuntime(
  script: string,
  scriptID: string,
  code: string
) {
  const userscript = executeUserScript(script, scriptID, code, clientUtils);

  if (
    userscript.config?.locations?.includes("all") ||
    userscript.config?.locations?.includes(currentSite || "unknown")
  ) {
    const config = createConfig(scriptID);
    userscript.load(config);
  }
}
