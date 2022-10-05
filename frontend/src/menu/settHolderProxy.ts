import type { Root } from "react-dom/client";
import { createRoot } from "react-dom/client";

export type RenderOnDemand = (root: Root) => void;

/**
 * Unique HTML string that acts as a fragment.
 */
export default function createSettHolderProxy(render: RenderOnDemand) {
  const id = "a-" + Math.random().toString().slice(2);

  class HTMLProxyElement extends HTMLElement {
    connectedCallback() {
      const settHolder = this.parentElement! as HTMLDivElement;

      this.remove();

      const root = createRoot(settHolder);

      render(root);

      Reflect.defineProperty(settHolder, "innerHTML", {
        configurable: true,
        set: (html) => {
          // remove descriptor
          Reflect.deleteProperty(settHolder, "innerHTML");

          root.unmount();

          settHolder.innerHTML = html;
        },
      });
    }
  }

  customElements.define(id, HTMLProxyElement);

  return `<${id} />`;
}
