export default class Hex3 {
  hex = [0, 0, 0];
  constructor(string = "#000") {
    this.set_style(string);
  }
  add_scalar(scalar) {
    for (let ind in this.hex) this.hex[ind] += scalar;
    return this.normalize();
  }
  sub_scalar(scalar) {
    for (let ind in this.hex) this.hex[ind] -= scalar;
    return this.normalize();
  }
  normalize() {
    for (let ind in this.hex)
      this.hex[ind] = Math.max(Math.min(this.hex[ind], 255), 0);
    return this;
  }
  set(r, g, b) {
    this.hex[0] = r;
    this.hex[1] = g;
    this.hex[2] = b;

    return this;
  }
  set_style(string) {
    let hex_index = 0,
      offset = string[0] == "#" ? 1 : 0,
      chunk = string.length - offset < 5 ? 1 : 2;

    for (let index = offset; index < string.length; index += chunk) {
      let part = string.substr(index, chunk);

      if (chunk == 1) part += part;

      this.hex[hex_index++] = parseInt(part, 16);
    }

    return this;
  }
  toString() {
    var string = "#";

    for (let color of this.hex) string += color.toString(16).padStart(2, 0);

    return string;
  }
}
