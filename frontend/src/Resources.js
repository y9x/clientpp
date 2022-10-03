import { css, js } from "./Runtime";
import currentSite from "./Site";
import { ipc } from "./IPC";
import console from "./Console";
import LegacyUserscript from "./LegacyUserscript";
import evalChiefUserscript from "./ChiefUserscript";

const add_css = () => {
  for (const [, data] of css) {
    const url = URL.createObjectURL(new Blob([data], { type: "text/css" }));

    const link = document.head.appendChild(
      Object.assign(document.createElement("link"), {
        rel: "stylesheet",
        href: url,
      })
    );

    link.addEventListener("load", () => {
      URL.revokeObjectURL(url);
    });

    document.head.appendChild(link);
  }
};

export default function run_resources(menu) {
  for (const [name, data, metadata, errors] of js) {
    for (const error of errors) console.error(error);

    if (errors.length) continue;

    if (metadata)
      evalChiefUserscript(name, metadata, data, currentSite, menu);
    else {
      // legacy idkr, unknown
      // quick fix
      if (data.includes("// ==UserScript==") && currentSite !== "game")
        continue;

      const module = { exports: {} };
      let func;
      const context = {
        module,
        exports: module.exports,
        console,
      };

      try {
        func = eval(
          `(function(${Object.keys(context)}){${data}//# sourceURL=${name}\n})`
        );
      } catch (err) {
        console.warn(`Error parsing userscript: ${name}\n`, err);
        ipc.console.error(`Error parsing userscript ${name}:\n${err}`);
        break;
      }

      // try{...}catch(err){...} doesnt provide: line, column

      console.log("load", name);

      try {
        func(...Object.values(context));

        const userscript = new LegacyUserscript(module.exports);

        userscript.run();
      } catch (err) {
        console.warn(`Error executing userscript: ${name}\n`, err);
        ipc.console.error(`Error executing userscript ${name}:\n${err}`);
        break;
      }
    }
  }
}

new MutationObserver((mutations, observer) => {
  for (const mutation of mutations) {
    for (const node of mutation.addedNodes) {
      if (
        node instanceof HTMLLinkElement &&
        new URL(node.href || "/", location).pathname === "/css/main_custom.css"
      ) {
        add_css();
        observer.disconnect();
      }
    }
  }
}).observe(document, {
  childList: true,
  subtree: true,
});
