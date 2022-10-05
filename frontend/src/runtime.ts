import type { Userscript } from "./userscript.js";

declare const _RUNTIME_DATA_: {
  css: [name: string, data: string][];
  js: [name: string, data: string, metadata: Userscript, errors: string][];
  config: any;
};

const { css, js, config } = _RUNTIME_DATA_;

export { css, js, config };
