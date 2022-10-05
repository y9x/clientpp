/*global webview*/
// webview is chrome.webview but captured by bootstrap.js

import Events from "./libs/Events";
import { IM, LogType } from "../../src/IPCMessages.h";

class IPCConsole {
  constructor(ipc) {
    this.ipc = ipc;
  }
  log(...args) {
    this.ipc.send(IM.log, LogType.info, args.join(" ") + "\n");
  }
  info(...args) {
    this.ipc.send(IM.log, LogType.info, args.join(" ") + "\n");
  }
  warn(...args) {
    this.ipc.send(IM.log, LogType.warn, args.join(" ") + "\n");
  }
  error(...args) {
    this.ipc.send(IM.log, LogType.error, args.join(" ") + "\n");
  }
  debug(...args) {
    this.ipc.send(IM.log, LogType.debug, args.join(" ") + "\n");
  }
}

class IPC extends Events {
  constructor() {
    super();
  }
  ongoing_posts = new Set();
  console = new IPCConsole(this);
  send(event, ...data) {
    if (typeof event !== "number")
      throw new TypeError(`Event must be a number. Recieved '${event}'`);
    webview.postMessage(JSON.stringify([event, ...data]));
    return true;
  }
  post(event, ...data) {
    let id = 0xFF; // reserved

    for (; id < 0xFFFF; id++) if (!this.ongoing_posts.has(id)) break;

    return new Promise((resolve, reject) => {
      this.once(id, (data, err) => {
        this.ongoing_posts.delete(id);
        if (err) reject(err);
        else resolve(data);
      });
      this.send(event, id, ...data);
    });
  }
}

const ipc = new IPC();

webview.addEventListener("message", ({ data }) => ipc.emit(...data));

// Object.assign(exports, messages);

export { IM, ipc };
