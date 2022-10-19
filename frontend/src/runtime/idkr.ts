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
 * Run a IDKR userscript
 */
export default function idkrRuntime(script: string, code: string) {
  const userscript = executeUserScript(script, code, clientUtils);

  if (
    userscript.config?.locations?.includes("all") ||
    userscript.config?.locations?.includes(currentSite || "unknown")
  ) {
    const config = createConfig(script);
    userscript.load(config);
  }
}
