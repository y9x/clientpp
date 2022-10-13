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

declare type GameActivityData =
  | GameActivityLoadedData
  | GameActivityPartialData;

declare interface GameActivityPartialData {
  id: null;
  time: null;
  user: string;
  class: {
    name: string;
    index: string;
  };
  map: null;
  mode: null;
  custom: boolean;
}

declare interface GameActivityLoadedData {
  id: string;
  time: number;
  user: string;
  class: {
    name: string;
    index: string;
  };
  map: string;
  mode: string;
  custom: boolean;
}

declare function getGameActivity(): GameActivityData;

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
