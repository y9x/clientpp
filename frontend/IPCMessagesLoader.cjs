function ipcMessagesLoader(code) {
  const ns = {};

  code
    .replace(/\/\/.*?$/gm, "")
    .replace(/namespace\s+(\w+)\s*?{([^]*?)}/g, (match, namespace, data) => {
      var add = {};
      data.replace(/(\w+)\s*?=\s*?(\d+)/g, (match, label, value) => {
        add[label] = JSON.parse(value);
      });
      ns[namespace] = add;
    });

  return `const {${Object.keys(ns).join(",")}}=${JSON.stringify(
    ns
  )}; export {${Object.keys(ns).join(",")}};`;
}

module.exports = ipcMessagesLoader;
