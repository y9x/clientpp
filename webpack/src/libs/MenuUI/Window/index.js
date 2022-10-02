import Category from "./Category";
import utils from "../../Utils";
import Events from "../../Events";

export default class Window extends Events {
  constructor(menu) {
    super();

    this.menu = menu;

    this.shadow = utils.add_ele(
      "div",
      () => document.querySelector("#uiBase"),
      {
        style: {
          position: "absolute",
          width: "100%",
          height: "100%",
          left: 0,
          top: 0,
          "z-index": 1e9,
        },
      }
    );

    this.node = this.shadow.attachShadow({ mode: "closed" });

    this.styles = new Set();

    new MutationObserver((mutations) => {
      for (const mutation of mutations)
        for (const node of mutation.addedNodes)
          if (["LINK", "STYLE"].includes(node.tagName)) this.update_styles();
    }).observe(document, { childList: true, subtree: true });

    this.holder = utils.add_ele("div", this.node, {
      id: "windowHolder",
      className: "popupWin",
      style: {
        "pointer-events": "all",
      },
    });

    this.container = utils.add_ele("div", this.holder, {
      id: "menuWindow",
      className: "stickyHeader dark",
      style: {
        "overflow-y": "auto",
        width: "1200px",
        "max-height": "calc(100% - 250px)",
        top: "50%",
        transform: "translate(-50%, -50%)",
      },
    });

    this.header = utils.add_ele("div", this.container, {
      className: "settingsHeader",
    });

    this.holder.addEventListener("click", (event) => {
      if (event.target == this.holder) this.hide();
    });

    this.hide();
  }
  update_styles() {
    for (const style of this.styles) style.remove(), this.styles.delete(style);

    for (const sheet of document.styleSheets) {
      const style = utils.add_ele("style", this.node);

      this.styles.add(style);

      if (sheet.href)
        style.textContent +=
          "@import url(" + JSON.stringify(sheet.href) + ");\n";
      else
        try {
          for (const rule of sheet.cssRules)
            style.textContent += rule.cssText + "\n";
        } catch (err) {
          console.error(err);
        }
    }
  }
  header(label) {
    return new Category(this, label);
  }
  show() {
    this.emit("show");
    this.shadow.style.display = "block";
  }
  hide() {
    this.emit("hide");
    this.shadow.style.display = "none";
  }
}
