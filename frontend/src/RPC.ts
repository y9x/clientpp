import ipc, { IM } from "./ipc";

declare global {
  function getGameActivity(): { user: string; map: string; mode: string };
}

export default class RPC {
  start = Date.now();
  last?: string;
  interval: ReturnType<typeof setInterval>;
  listener() {
    this.update();
  }
  constructor() {
    this.interval = setInterval(() => this.update(), 1000);
  }
  delete() {
    clearInterval(this.interval);
  }
  update(force = false) {
    if (!window.getGameActivity) return;

    let activity;

    try {
      activity = window.getGameActivity();
    } catch (err) {
      return;
    }

    const { user, map, mode } = activity;
    const args = [user || "", map || "", mode || ""];
    const jargs = JSON.stringify(args);

    // detect change via jargs
    if (!force && jargs !== this.last) {
      ipc.send(IM.rpc_update, this.start, ...args);
      this.last = jargs;
    }
  }
}
