"use strict";

import utils from "../../Utils";
import { resolveControl } from "../Control";

export default class Category {
  constructor(tab, label) {
    this.tab = tab;

    this.controls = new Set();

    if (label) {
      this.label = label;

      this.header = utils.add_ele("div", this.tab.content, {
        className: "setHed",
      });

      this.header_status = utils.add_ele("span", this.header, {
        className: "material-icons plusOrMinus",
      });

      utils.add_ele("text", this.header, { nodeValue: label });

      this.header.addEventListener("click", () => this.toggle());
    }

    this.content = utils.add_ele("div", this.tab.content, {
      className: "setBodH",
    });

    if (label) this.expand();
  }
  toggle() {
    if (this.collapsed) this.expand();
    else this.collapse();
  }
  collapse() {
    this.collapsed = true;
    this.update();
  }
  expand() {
    this.collapsed = false;
    this.update();
  }
  update(init) {
    this.content.style.display = this.collapsed ? "none" : "block";

    if (this.header) {
      this.header.style.display = "block";
      this.header_status.textContent =
        "keyboard_arrow_" + (this.collapsed ? "right" : "down");
    }

    for (const control of this.controls) control.update(init);
  }
  show() {
    this.expand();
    if (this.header) this.header.style.display = "block";
  }
  hide() {
    this.content.style.display = "none";
    if (this.header) this.header.style.display = "none";
  }
  fix() {
    this.update();
    for (const control of this.controls) control.show_content();
  }
  control(name, data) {
    const control = new (resolveControl(data.type))(name, data, this);
    this.controls.add(control);
    return control;
  }
}
