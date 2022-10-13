/* eslint-disable no-var */
/* eslint-disable @typescript-eslint/consistent-type-imports */

declare module "*IPCMessages.h" {
  export const IM: Record<string, number>;
  export const LogType: Record<string, number>;
}

declare function getRuntimeData(): {
  css: [name: string, data: string][];
  js: [name: string, data: string][];
  config: typeof import("../../resources/config.json");
};

declare const chrome: {
  webview: EventTarget & {
    postMessage: (data: string) => void;
    addEventListener(
      type: "message",
      listener: (
        this: Window,
        ev: MessageEvent<[string, ...unknown[]]>
      ) => unknown,
      options?: boolean | AddEventListenerOptions
    ): void;
  };
};
