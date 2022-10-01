"use strict";

import Window from "./Window";
import utils from "../Utils";
import Category from "./Window/Category";

export default class HeaderWindow extends Window {
  categories = new Set();
  constructor(menu, label) {
    super(menu);

    this.window = this;

    this.content = utils.add_ele("div", this.container, { id: "settHolder" });

    utils.add_ele("div", this.content, {
      id: "referralHeader",
      textContent: label,
    });

    this.hide();
  }
  category(label) {
    var category = (this.last_category = new Category(this, label));

    this.categories.add(category);

    return category;
  }
  control(...args) {
    var category = this.last_category;

    if (!category || !category.is_default) {
      category = this.category();
      category.is_default = true;
    }

    return category.control(...args);
  }
  show() {
    super.show();
    for (let category of this.categories) category.fix();
  }
  hide() {
    super.hide();
  }
}
