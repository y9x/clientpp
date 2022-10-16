import type swcrcSchema from "./swcrc.js";
import type { JsMinifyOptions } from "@swc/core";
import ESLintPlugin from "eslint-webpack-plugin";
import ForkTsCheckerPlugin from "fork-ts-checker-webpack-plugin";
import { fileURLToPath } from "node:url";
import { resolve } from "path";
import ModuleNotFoundPlugin from "react-dev-utils/ModuleNotFoundPlugin.js";
import TerserPlugin from "terser-webpack-plugin";
import webpack from "webpack";
import type { Configuration } from "webpack";

const isDevelopment = process.env.NODE_ENV === "development";

const config: Configuration = {
  entry: {
    game: fileURLToPath(new URL("./src/game.ts", import.meta.url)),
    generic: fileURLToPath(new URL("./src/generic.ts", import.meta.url)),
  },
  output: {
    path: fileURLToPath(new URL("./dist/", import.meta.url)),
  },
  // we can't get devtools to accept our custom request hook to load sourcemaps unless we do it inline
  // execution will cost 1 MB!
  // we can't use cheap sourcemaps because they are simply cheap to work with
  // not producing a sourcemap and not minifying code is good, however bundle size is larger compared to minified + sourcemaps
  devtool: isDevelopment ? "eval" : "inline-source-map",
  mode: isDevelopment ? "development" : "production",
  optimization: {
    minimize: !isDevelopment,
    minimizer: [
      new TerserPlugin<JsMinifyOptions>({
        minify: TerserPlugin.swcMinify,
      }),
    ],
  },
  resolve: {
    extensions: [".mjs", ".js", ".ts", ".tsx", ".json", ".jsx"],
  },
  module: {
    rules: [
      {
        enforce: "pre",
        test: /\.(js|mjs|jsx|ts|tsx)$/,
        exclude: /@swc(?:\/|\\{1,2})helpers/,
        loader: "source-map-loader",
      },
      {
        oneOf: [
          {
            test: /\.[mc]?[jt]sx?$/,
            include: resolve("src"),
            loader: "swc-loader",
            resolve: {
              fullySpecified: false,
            },
            options: {
              sourceMaps: true,
              minify: !isDevelopment,
              jsc: {
                parser: {
                  syntax: "typescript",
                  tsx: true,
                  decorators: false,
                  dynamicImport: true,
                },
                transform: {
                  react: {
                    runtime: "automatic",
                  },
                },
                target: "es2015",
                externalHelpers: true,
              },
            } as swcrcSchema,
          },
          {
            test: /\.(js|mjs)$/,
            exclude: /@swc(?:\/|\\{1,2})helpers/,
            loader: "swc-loader",
            options: {
              minify: !isDevelopment,
              sourceMaps: true,
              jsc: {
                target: "es2015",
                externalHelpers: true,
              },
            } as swcrcSchema,
          },
          {
            test: /IPCMessages\.h$/,
            use: fileURLToPath(
              new URL("./IPCMessagesLoader.cjs", import.meta.url)
            ),
          },
        ],
      },
    ],
  },
  plugins: [
    new ModuleNotFoundPlugin(resolve(".")),
    new ForkTsCheckerPlugin(),
    new ESLintPlugin(),
    new webpack.BannerPlugin({
      banner: "try{",
      raw: true,
    }),
    new webpack.BannerPlugin({
      banner: "}catch(err){setTimeout(() => console.error(err));}",
      footer: true,
      raw: true,
    }),
  ],
};

export default config;
