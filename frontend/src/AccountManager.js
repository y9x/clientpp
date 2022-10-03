/*global loginAcc,logoutAcc,closWind*/
import utils from "./libs/Utils";
import Control from "./libs/MenuUI/Control";
import Events from "./libs/Events";
import Hex3 from "./libs/Hex3";
import { IM, ipc } from "./IPC";
import { tick } from "./libs/MenuUI/Sound";
import currentSite from "./Site";
import HeaderWindow from "./libs/MenuUI/HeaderWindow";
import accountManagerCSS from "./AccountManager.css";

class AccountPop {
  constructor(body, callback) {
    this.callback = callback;

    this.container = utils.add_ele("div", body, {
      id: "importPop",
      style: { "z-index": 1e9 },
    });

    utils.add_ele("div", this.container, {
      className: "pubHed",
      textContent: "Add Account",
    });

    this.username = utils.add_ele("input", this.container, {
      type: "text",
      placeholder: "Enter Username",
      className: "inputGrey2",
      style: { width: "379px", "margin-bottom": "5px" },
    });

    this.password = utils.add_ele("input", this.container, {
      type: "password",
      placeholder: "Enter Password",
      className: "inputGrey2",
      style: { width: "379px" },
    });

    utils.add_ele("div", this.container, {
      className: "mapFndB",
      textContent: "Add",
      style: {
        width: "180px",
        "margin-top": "10px",
        "margin-left": 0,
        background: "#4af",
      },
      events: {
        click: () => {
          try {
            this.callback(this.username.value, this.password.value);
          } catch (err) {
            console.error(err);
          }

          this.username.value = "";
          this.password.value = "";
          this.hide();
        },
      },
    });

    utils.add_ele("div", this.container, {
      className: "mapFndB",
      textContent: "Cancel",
      style: {
        width: "180px",
        "margin-top": "10px",
        "margin-left": 0,
        background: "#122",
      },
      events: { click: () => this.hide() },
    });

    this.hide();
  }
  show() {
    this.container.style.display = "block";
  }
  hide() {
    this.container.style.display = "none";
  }
}

class AccountTile {
  constructor(resp, node, window, username, data) {
    this.data = data;
    this.resp = resp;
    this.username = username;
    this.window = window;
    this.create(node);
  }
  create(node) {
    const hex = new Hex3(this.data.color);

    this.container = utils.add_ele("div", node, {
      className: "account-tile",
    });

    this.label = utils.add_ele("div", this.container, {
      className: "text",
      textContent: this.username,
      events: {
        click: this.click.bind(this),
      },
      style: {
        "background-color": `rgba(${hex.hex}, var(--alpha))`,
      },
    });

    this.buttons = utils.add_ele("div", this.container, {
      className: "buttons",
    });

    utils.add_ele("span", this.buttons, {
      className: "edit material-icons",
      textContent: "edit",
      events: {
        click: this.edit.bind(this),
      },
    });

    utils.add_ele("span", this.buttons, {
      className: "delete material-icons",
      textContent: "delete",
      events: {
        click: this.delete.bind(this),
      },
    });
  }
  delete() {
    ipc.send(IM.account_remove, this.username);
  }
  edit() { }
  async click() {
    const that = this;

    // this.window.hide();

    const password = await ipc.post(IM.account_password, this.username);

    // MTZ client does this procedure
    logoutAcc();
    closWind();

    window.accName = { value: this.username };
    window.accPass = { value: password };
    window.accResp = {
      set innerHTML(value) {
        that.resp.node.innerHTML = value;
        delete window.accResp;
      },
    };

    loginAcc();

    setTimeout(() => {
      loginAcc();
      delete window.accName;
      delete window.accPass;
    }, 500);
  }
}

class Menu extends Events {
  save_config() { }
  config = {};
  window = new HeaderWindow(this, "Accounts");
  async attach() {
    const opts = {
      className: "button buttonG lgn",
      style: {
        width: "200px",
        "margin-right": 0,
        "padding-top": "5px",
        "margin-left": "5px",
        "padding-bottom": "13px",
      },
      innerHTML:
        "Accounts <span class=\"material-icons\" style=\"vertical-align:middle;color: #fff;font-size:36px;margin-top:-8px;\">switch_account</span>",
      events: {
        click: () => {
          this.resp.node.innerHTML =
            "For lost Passwords/Accounts contact <span style=\"color:rgba(255,255,255,0.8)\">recovery@yendis.ch</span>";
          this.window.show();
        },
      },
    };

    tick(
      utils.add_ele(
        "div",
        () => document.querySelector("#signedInHeaderBar"),
        opts
      )
    );
    tick(
      utils.add_ele(
        "div",
        () => document.querySelector("#signedOutHeaderBar"),
        opts
      )
    );
  }
  async generate(list) {
    this.table.node.innerHTML = "";

    for (const [username, data] of Object.entries(list).sort(
      (p1, p2) => p1.order - p2.order
    ))
      new AccountTile(this.resp, this.table.node, this.window, username, data);
  }
  constructor() {
    super();

    this.account_pop = new AccountPop(
      this.window.node,
      (username, password) => {
        if (!username || !password) return;

        ipc.send(IM.account_set_password, username, password, "#2196f3", 0);
      }
    );

    this.table = this.window.control("", {
      type: class extends Control {
        create() {
          this.node = utils.add_ele("div", this.content, {
            className: "account-tiles",
          });
        }
        update(init) {
          super.update(init);
          if (init) this.input.checked = this.value;
          this.label_text(this.name);
        }
      },
    });

    this.resp = this.window.control("", {
      type: class extends Control {
        create() {
          this.node = utils.add_ele("div", this.content, {
            id: "accResp",
            style: {
              "min-height": "1em",
              "margin-top": "20px",
              "margin-bottom": "20px",
              "font-size": "18px",
              color: "rgba(255,255,255,0.5)",
              "text-align": "center",
            },
          });
        }
        update(init) {
          super.update(init);
          if (init) this.input.checked = this.value;
          this.label_text(this.name);
        }
      },
    });

    this.window.control("Account", {
      type: "function",
      text: "Add",
      color: "blue",
      value: () => this.account_pop.show(),
    });

    this.window.on("hide", () => this.account_pop.hide());

    utils.add_ele("style", this.window.node, {
      textContent: accountManagerCSS,
    });

    ipc.post(IM.account_list).then((list) => this.generate(list));
  }
}

if (currentSite === "game") {
  const menu = new Menu();
  window.menu = menu;
  menu.attach();

  ipc.on(IM.account_regen, (list) => menu.generate(list));
}
