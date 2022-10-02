// Legacy IDKR userscript

import currentSite from "./Site";

export default class Userscript {
  #field(compare, value) {
    if (Array.isArray(compare)) {
      if (!Array.isArray(value)) value = [].concat(value);

      if (!value.length || value.some((data) => typeof data != "string"))
        return compare;
    } else if (typeof compare != typeof value) return compare;

    return value;
  }
  #run = () => {};
  name = "Unnamed userscript";
  version = "Unknown version";
  author = "Unknown author";
  description = "No description provided";
  locations = ["game"];
  platforms = ["all"];
  settings = { used: false };
  constructor(data) {
    this.#run = this.#field(this.#run, data.run);
    this.name = this.#field(this.name, data.name);
    this.version = this.#field(this.version, data.version);
    this.author = this.#field(this.author, data.author);
    this.description = this.#field(this.description, data.description);
    this.locations = this.#field(this.locations, data.locations);
    this.platforms = this.#field(this.platforms, data.platforms);
    this.settings = this.#field(this.settings, data.settings);
  }
  async run() {
    if (
      !this.locations.includes("all") &&
      !this.locations.includes(currentSite)
    )
      return false;

    try {
      await this.#run();
      return true;
    } catch (err) {
      console.error(err);
      return false;
    }
  }
}
