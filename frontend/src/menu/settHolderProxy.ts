/**
 * Unique HTML string that acts as a fragment.
 */
export default class HTMLProxy extends DocumentFragment {
  #id = "a-" + Math.random().toString().slice(2);
  constructor() {
    super();

    const replaceWith = (proxy: HTMLProxyElement) => proxy.replaceWith(this);

    class HTMLProxyElement extends HTMLElement {
      connectedCallback() {
        replaceWith(this);
      }
    }

    customElements.define(this.#id, HTMLProxyElement);
  }
  get() {
    return `<${this.#id} />`;
  }
}
