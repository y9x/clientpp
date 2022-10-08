import console from "./console";
import { css, js } from "./runtime";
import chiefRuntime from "./runtime/chief";
import idkrRuntime from "./runtime/idkr";
import tampermonkeyRuntime from "./runtime/tampermonkey";

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

for (const [script, code] of js) {
  if (script.endsWith(".chief.js")) chiefRuntime(script, code);
  else if (script.endsWith(".idkr.js")) idkrRuntime(script, code);
  else if (script.endsWith(".user.js")) tampermonkeyRuntime(script, code);

  console.log("GOT SCRIPT", script, "....");

  // if (metadata) evalChiefUserscript(name, metadata, data);
  // else evalLegacyUserscript(name, data);
}
