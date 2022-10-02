export const keybinds = new Set();

export default class Keybind {
  constructor(key, callback) {
    this.keys = new Set();
    this.callbacks = new Set();
    keybinds.add(this);

    if (typeof key === "string") {
      this.key(key);
      key = callback;
    }

    if (typeof key === "function") this.callback(callback);
  }
  delete() {
    keybinds.delete(this);
  }
  set_key(...args) {
    return (this.keys = new Set()), this.key(...args);
  }
  set_callback(...args) {
    return (this.callbacks = new Set()), this.key(...args);
  }
  key(...keys) {
    for (const key of keys) this.keys.add(key);
    return this;
  }
  callback(...funcs) {
    for (const func of funcs) this.callbacks.add(func);
    return this;
  }
}
window.addEventListener("keydown", (event) => {
  if (event.repeat) return;
  for (const node of [...event.composedPath()])
    if (node.tagName)
      for (const part of ["INPUT", "TEXTAREA"])
        if (node.tagName.includes(part)) return;

  //  || keybind.repeat
  for (const keybind of keybinds)
    if (!event.repeat && keybind.keys.has(event.code)) {
      event.preventDefault();
      for (const callback of keybind.callbacks) callback(event);
    }
});
