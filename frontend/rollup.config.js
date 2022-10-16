import commonjs from "@rollup/plugin-commonjs";
import { nodeResolve } from "@rollup/plugin-node-resolve";
import typescript from "@rollup/plugin-typescript";
import MagicString from "magic-string";
import { terser } from "rollup-plugin-terser";

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

const isDevelopment = process.env.NODE_ENV === "development";

/**
 * @type {import("rollup").RollupOptions[]}
 */
const configs = [
  {
    input: "./src/game.ts",
    output: {
      sourcemap: "inline",
      sourcemapPathTransform: (relativeSourcePath) =>
        new URL(
          "./frontend/" + relativeSourcePath,
          "rollup://chief/frontend/"
        ).toString(),
      dir: "./dist/",
      format: "iife",
    },
    plugins: [
      commonjs(),
      nodeResolve(),
      typescript({
        sourceMap: true,
        inlineSources: true,
      }),
      tryCatch(),
      !isDevelopment && terser(),
    ],
  },
  {
    input: "./src/generic.ts",
    output: {
      sourcemap: "inline",
      sourcemapPathTransform: (relativeSourcePath) =>
        new URL(
          "./frontend/" + relativeSourcePath,
          "rollup://chief/frontend/"
        ).toString(),
      dir: "./dist/",
      format: "iife",
    },
    plugins: [
      commonjs(),
      nodeResolve(),
      typescript({
        sourceMap: true,
        inlineSources: true,
      }),
      tryCatch(),
      !isDevelopment && terser(),
    ],
  },
];

export default configs;
