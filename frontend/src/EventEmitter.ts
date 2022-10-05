export const original = Symbol();

type EventType = string | number;

type Callback = ((this: EventEmitter, ...args: never[]) => unknown) & {
  [original]?: Callback;
};

export default class EventEmitter {
  #events = new Map();
  #resolve(event: EventType) {
    let callbacks = this.#events.get(event);

    if (!callbacks) {
      callbacks = new Set();
      this.#events.set(event, callbacks);
    }

    return callbacks;
  }
  on(event: EventType, callback: Callback) {
    if (typeof callback !== "function")
      throw new TypeError("Callback is not a function.");

    this.#resolve(event).add(callback);

    return this;
  }
  once(event: EventType, callback: Callback) {
    const cb = (...data: never[]) => {
      this.off(event, callback);
      callback.call(this, ...data);
    };

    callback[original] = cb;

    return this.on(event, cb);
  }
  off(event: EventType, callback: Callback) {
    if (typeof callback !== "function")
      throw new TypeError("Callback is not a function.");

    if (callback[original]) callback = callback[original];

    const list = this.#resolve(event);

    return list.delete(callback);
  }
  emit(event: EventType, ...data: unknown[]) {
    const set = this.#resolve(event);

    if (!set.size) {
      if (event === "error") throw data[0];
      return false;
    } else
      for (const item of set)
        try {
          item.call(this, ...data);
        } catch (err) {
          this.emit("error", err);
        }

    return true;
  }
}
