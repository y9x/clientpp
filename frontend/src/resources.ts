import { css } from "./runtime";

function addCSS() {
  for (const [script, code] of css) {
    const style = document.createElement("style");
    style.textContent = code + `/*# sourceURL=${script} */`;
    document.head.appendChild(style);
  }
}

new MutationObserver((mutations, observer) => {
  for (const mutation of mutations) {
    for (const node of mutation.addedNodes) {
      if (
        node instanceof HTMLLinkElement &&
        new URL(node.href || "/", window.location.toString()).pathname ===
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
