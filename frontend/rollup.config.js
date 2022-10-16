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
        exclude: /@babel(?:\/|\\{1,2})runtime/,
        loader: "source-map-loader",
      },
      {
        oneOf: [
          {
            test: /\.[mc]?[jt]sx?$/,
            include: resolve("src"),
            use: [
              {
                loader: "babel-loader",
                options: {
                  customize: resolve(
                    "node_modules/babel-preset-react-app/webpack-overrides.js"
                  ),
                  presets: [
                    [
                      "@babel/preset-typescript",
                      {
                        allowDeclareFields: true,
                      },
                    ],
                    [
                      "babel-preset-react-app",
                      {
                        runtime: "automatic",
                      },
                    ],
                  ],
                  plugins: [
                    [
                      "@babel/plugin-transform-react-jsx",
                      {
                        runtime: "automatic",
                        importSource: "preact",
                      },
                    ],
                  ],
                  // This is a feature of `babel-loader` for webpack (not Babel itself).
                  // It enables caching results in ./node_modules/.cache/babel-loader/
                  // directory for faster rebuilds.
                  cacheDirectory: true,
                  // See #6846 for context on why cacheCompression is disabled
                  cacheCompression: false,
                  compact: !isDevelopment,
                },
              },
            ],
          },
          {
            test: /\.(js|mjs)$/,
            exclude: /@swc(?:\/|\\{1,2})helpers/,
            loader: "babel-loader",
            options: {
              babelrc: false,
              configFile: false,
              compact: false,
              presets: [
                ["babel-preset-react-app/dependencies", { helpers: true }],
              ],
              cacheDirectory: true,
              // See #6846 for context on why cacheCompression is disabled
              cacheCompression: false,

              // Babel sourcemaps are needed for debugging into node_modules
              // code.  Without the options below, debuggers like VSCode
              // show incorrect code and set breakpoints on the wrong lines.
              sourceMaps: true,
              inputSourceMap: true,
            },
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
