/* eslint-disable no-new-func */
import { css, js } from "./runtime";
import console from "./console";
import evalLegacyUserscript from "./legacyUserscript";
import evalChiefUserscript from "./chiefUserscript";
import menu from "./menu";

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

new MutationObserver((mutations, observer) => {
  for (const mutation of mutations) {
    for (const node of mutation.addedNodes) {
      if (
        node instanceof HTMLLinkElement &&
        new URL(node.href || "/", global.location.toString()).pathname ===
          "/css/main_custom.css"
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

if (menu) {
  for (const [name, data, metadata, errors] of js) {
    for (const error of errors) console.error(error);

    if (errors.length) continue;

    if (metadata) evalChiefUserscript(name, metadata, data);
    else evalLegacyUserscript(name, data);
  }

  menu.update();
}
