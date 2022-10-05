import type { Userscript } from "./userscript.js";
import type defaultConfig from "../../resources/config.json";

declare const _RUNTIME_DATA_: {
  css: [name: string, data: string][];
  js: [name: string, data: string, metadata: Userscript, errors: string][];
  config: typeof defaultConfig;
};

const { css, js, config } = _RUNTIME_DATA_;

export { css, js, config };
