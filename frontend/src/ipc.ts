import EventEmitter from "./EventEmitter";
import { IM, LogType } from "../../src/IPCMessages.h";

// webview is chrome.webview but captured by bootstrap.js
declare const webview: EventTarget & {
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

export const ipcConsole = {
  log: (...args: unknown[]) =>
    ipc.send(IM.log, LogType.info, args.join(" ") + "\n"),
  warn: (...args: unknown[]) =>
    ipc.send(IM.log, LogType.warn, args.join(" ") + "\n"),
  error: (...args: unknown[]) =>
    ipc.send(IM.log, LogType.error, args.join(" ") + "\n"),
  debug: (...args: unknown[]) =>
    ipc.send(IM.log, LogType.debug, args.join(" ") + "\n"),
};

class IPC extends EventEmitter {
  ongoing_posts = new Set<number>();
  send(event: number, ...data: unknown[]) {
    if (typeof event !== "number")
      throw new TypeError(`Event must be a number. Recieved '${event}'`);
    webview.postMessage(JSON.stringify([event, ...data]));
    return true;
  }
  post(event: number, ...data: unknown[]) {
    let id = 0xff; // reserved

    for (; id < 0xffff; id++) if (!this.ongoing_posts.has(id)) break;

    return new Promise((resolve, reject) => {
      this.once(id.toString(), (data, err) => {
        this.ongoing_posts.delete(id);
        if (err) reject(err);
        else resolve(data);
      });
      this.send(event, id.toString(), ...data);
    });
  }
}

const ipc = new IPC();

webview.addEventListener("message", ({ data }) => ipc.emit(...data));

export { IM, ipc as default };
