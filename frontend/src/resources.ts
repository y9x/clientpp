import console from "./console";
import { css, js } from "./runtime";
import chiefRuntime from "./runtime/chief";
import idkrRuntime from "./runtime/idkr";
import tampermonkeyRuntime from "./runtime/tampermonkey";

function addCSS() {
  for (const [name, data] of css) {
    const style = document.createElement("style");
    style.textContent = data + `/*# sourceURL=${name} */`;
    document.head.appendChild(style);
  }
}

new MutationObserver((mutations, observer) => {
  for (const mutation of mutations) {
    for (const node of mutation.addedNodes) {
      if (
        node instanceof HTMLLinkElement &&
        new URL(node.href || "/", global.location.toString()).pathname ===
          "/css/main_custom.css"
      ) {
        addCSS();
        observer.disconnect();
      }
    }
  }
}).observe(document, {
  childList: true,
  subtree: true,
});

for (const [script, code] of js)
  try {
    if (script.endsWith(".chief.js")) chiefRuntime(script, code);
    else if (script.endsWith(".idkr.js")) idkrRuntime(script, code);
    else if (script.endsWith(".user.js")) tampermonkeyRuntime(script, code);
  } catch (err) {
    console.error(`Failure loading ${script}:`);
    console.error(err);
  }
