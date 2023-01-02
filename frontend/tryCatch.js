import MagicString from "magic-string";

/**
 * @return {import("rollup").Plugin}
 */
const tryCatch = () => {
    return {
      name: "tryCatch",
      renderChunk: (code) => {
        const magic = new MagicString(code);
        magic.appendLeft(0, "try{");
        magic.append("}catch(err){setTimeout(() => console.error(err));}");
        return { map: magic.generateMap(), code: magic.toString() };
      },
    };
  };

  export default tryCatch;