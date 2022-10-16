import commonjs from "@rollup/plugin-commonjs";
import { nodeResolve } from "@rollup/plugin-node-resolve";
import typescript from "@rollup/plugin-typescript";
import MagicString from "magic-string";
import sourcemaps from "rollup-plugin-sourcemaps";

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

// const isDevelopment = process.env.NODE_ENV === "development";

/**
 * @type {import("rollup").RollupOptions[]}
 */
const configs = [
  {
    input: "./src/game.ts",
    output: {
      sourcemap: "inline",
      sourcemapBaseUrl: "chief://the/frontend/",
      dir: "./dist/",
      format: "iife",
    },
    plugins: [
      commonjs(),
      nodeResolve(),
      typescript({
        inlineSources: true,
      }),
      sourcemaps(),
      tryCatch(),
    ],
  },
  {
    input: "./src/generic.ts",
    output: {
      sourcemap: "inline",
      dir: "./dist/",
      format: "iife",
    },
    plugins: [
      commonjs(),
      nodeResolve(),
      typescript(),
      sourcemaps(),
      tryCatch(),
    ],
  },
];

export default configs;
