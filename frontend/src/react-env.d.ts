/* eslint-disable no-var */
/* eslint-disable @typescript-eslint/consistent-type-imports */

declare module "*IPCMessages.h" {
  export const IM: Record<string, number>;
  export const LogType: Record<string, number>;
}

declare var _RUNTIME_DATA_:
  | {
      css: [name: string, data: string][];
      js: [
        name: string,
        data: string,
        metadata: import("./userscript").Userscript,
        errors: string
      ][];
      config: typeof import("../../resources/config.json");
    }
  | undefined;
