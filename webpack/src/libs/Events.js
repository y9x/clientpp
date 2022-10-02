export default class Events {
  static original = Symbol();
  #events = new Map();
  #resolve(event) {
    let callbacks = this.#events.get(event);

    if (!callbacks) {
      callbacks = new Set();
      this.#events.set(event, callbacks);
    }

    return callbacks;
  }
  on(event, callback) {
    if (typeof callback != "function")
      throw new TypeError("Callback is not a function.");

    this.#resolve(event).add(callback);

    return this;
  }
  once(event, callback) {
    const cb = function (...data) {
      this.off(event, callback);
      callback.call(this, ...data);
    };

    callback[Events.original] = cb;

    return this.on(event, cb);
  }
  off(event, callback) {
    if (typeof callback != "function")
      throw new TypeError("Callback is not a function.");

    if (callback[Events.original]) callback = callback[Events.original];

    const list = this.#resolve(event);

    return list.delete(callback);
  }
  emit(event, ...data) {
    const set = this.#resolve(event);

    if (!set.size) {
      if (event == "error") throw data[0];
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
