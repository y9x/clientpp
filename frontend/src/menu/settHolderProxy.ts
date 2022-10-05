/**
 * Unique HTML string that acts as a fragment.
 * Elements of the fragment will be recalled before being unloaded. Elements are always reused and never need to be recreated.
 */
export default function createSettHolderProxy() {
  const id = "a-" + Math.random().toString().slice(2);

  const fragment = new DocumentFragment();

  class HTMLProxyElement extends HTMLElement {
    connectedCallback() {
      const settHolder = this.parentElement! as HTMLDivElement;

      Reflect.defineProperty(settHolder, "innerHTML", {
        configurable: true,
        set: (html) => {
          for (const child of settHolder.childNodes) fragment.append(child);

          // remove descriptor
          Reflect.deleteProperty(settHolder, "innerHTML");

          settHolder.innerHTML = html;
        },
      });

      this.replaceWith(fragment);
    }
  }

  customElements.define(id, HTMLProxyElement);

  return {
    fragment,
    html: `<${id} />`,
  };
}
