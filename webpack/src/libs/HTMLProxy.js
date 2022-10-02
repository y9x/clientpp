export default class HTMLProxy {
  children = [];
  appendChild(node) {
    this.append(node);
    return node;
  }
  append(node) {
    this.children.push(node);
  }
  constructor() {
    this.id = "a-" + Math.random().toString().slice(2);

    const children = this.children;

    customElements.define(
      this.id,
      class extends HTMLElement {
        connectedCallback() {
          for (const node of children) this.parentNode.insertBefore(node, this);
          this.remove();
        }
      }
    );
  }
  get() {
    const html = `<${this.id}></${this.id}>`;

    // html += '<!-- ';
    // for(let node of this.children)html += node.outerHTML.replace(/-->/g, '->');
    // html += '-->';

    return html;
  }
  node() {
    return document.createElement(this.id);
  }
}
