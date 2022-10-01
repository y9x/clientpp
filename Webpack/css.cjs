function cssLoader(source) {
  return `const css = ${JSON.stringify(source)}; export default css;`;
}

module.exports = cssLoader;
