/*global windows*/
import utils from "../libs/Utils";
import Events from "./Events";
import HTMLProxy from "./HTMLProxy";
import Category from "./MenuUI/Window/Category";

export default class ExtendMenu extends Events {
  html = new HTMLProxy();
  async save_config() {
    console.error("save_config() not implemented");
  }
  async load_config() {
    console.error("load_config() not implemented");
  }
  tab = {
    content: this.html,
    window: {
      menu: this,
    },
  };
  async attach() {
    if (this.window instanceof Object) throw new Error("Already attached");

    const array = await utils.wait_for(
      () => typeof windows == "object" && windows
    );

    this.window = array.find((window) => window.header == this.header);

    if (!this.window) throw new Error(`Unable to find header '${this.header}'`);

    const getSettings = this.window.getSettings;
    const indexes = {};

    for (const i in this.window.tabs) {
      indexes[i] = this.window.tabs[i].length;

      this.window.tabs[i].push({
        name: this.label,
        categories: [],
      });
    }

    this.window.getSettings = () =>
      this.window.tabIndex == indexes[this.window.settingType]
        ? this.html.get()
        : getSettings.call(this.window);
  }
  detach() {
    if (!(this.window instanceof Object)) throw new Error("Not attached");

    this.window.tabs.splice(this.index, 1);
    this.window.getSettings = this.getSettings;
    this.window = null;
  }
  categories = new Set();
  category(label) {
    const cat = new Category(this.tab, label);
    this.categories.add(cat);
    return cat;
  }
  update(init = false) {
    for (const category of this.categories) category.update(init);
  }
  constructor(header, label) {
    super();
    this.header = header;
    this.label = label;
  }
}