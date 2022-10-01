"use strict";

import Events from "../Events";

export default class Addon extends Events {
  constructor(menu, args) {
    super();

    this.menu = menu;
    this.window = menu.window;

    this.create(...args);
  }
  ready() {
    console.info(this.name, "loaded");
    this.emit("ready");
  }
  create() {}
}
