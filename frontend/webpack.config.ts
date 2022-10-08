import TryCatchPlugin from "./TryCatchWebpackPlugin.js";
import type { CSSLoaderOptions } from "./css-loader.js";
import type swcrcSchema from "./swcrc.js";
import type { JsMinifyOptions } from "@swc/core";
import CssMinimizerPlugin from "css-minimizer-webpack-plugin";
import ESLintPlugin from "eslint-webpack-plugin";
import ForkTsCheckerPlugin from "fork-ts-checker-webpack-plugin";
import MiniCssExtractPlugin from "mini-css-extract-plugin";
import { fileURLToPath } from "node:url";
import { resolve } from "path";
import ModuleNotFoundPlugin from "react-dev-utils/ModuleNotFoundPlugin.js";
import getCSSModuleLocalIdent from "react-dev-utils/getCSSModuleLocalIdent.js";
import TerserPlugin from "terser-webpack-plugin";
import type { Configuration, RuleSetRule } from "webpack";

const isDevelopment = process.env.NODE_ENV === "development";

// common function to get style loaders
const getStyleLoaders = (
  cssOptions: CSSLoaderOptions,
  preProcessor?: string
) => {
  const loaders: (RuleSetRule | string | false)[] = [
    isDevelopment && "style-loader",
    !isDevelopment && {
      loader: MiniCssExtractPlugin.loader,
      // css is located in `static/css`, use '../../' to locate index.html folder
      // in production `paths.publicUrlOrPath` can be a relative path
    },
    {
      loader: "css-loader",
      options: cssOptions,
    },
    {
      // Options for PostCSS as we reference these options twice
      // Adds vendor prefixing based on your specified browser support in
      // package.json
      loader: "postcss-loader",
      options: {
        postcssOptions: {
          // Necessary for external CSS imports to work
          // https://github.com/facebook/create-react-app/issues/2677
          ident: "postcss",
          config: false,
          plugins: [
            "postcss-flexbugs-fixes",
            [
              "postcss-preset-env",
              {
                autoprefixer: {
                  flexbox: "no-2009",
                },
                stage: 3,
              },
            ],
            // Adds PostCSS Normalize as the reset css with default options,
            // so that it honors browserslist config in package.json
            // which in turn let's users customize the target behavior as per their needs.
            "postcss-normalize",
          ],
        },
        sourceMap: true,
      },
    },
  ].filter(Boolean);

  if (preProcessor) {
    loaders.push(
      {
        loader: "resolve-url-loader",
        options: {
          sourceMap: true,
          root: resolve("src"),
        },
      },
      {
        loader: preProcessor,
        options: {
          sourceMap: true,
        },
      }
    );
  }
  return loaders as (RuleSetRule | string)[];
};

const cssRegex = /\.css$/;
const cssModuleRegex = /\.module\.css$/;

const config: Configuration = {
  entry: fileURLToPath(new URL("./src/index.ts", import.meta.url)),
  output: {
    path: fileURLToPath(new URL("./dist/", import.meta.url)),
  },
  // inline work can be done from the client
  // base64 strings are 3x larger
  devtool: isDevelopment ? "eval" : "source-map",
  mode: isDevelopment ? "development" : "production",
  optimization: {
    minimize: !isDevelopment,
    minimizer: [
      new TerserPlugin<JsMinifyOptions>({
        minify: TerserPlugin.swcMinify,
      }),
      new CssMinimizerPlugin(),
    ],
  },
  resolve: {
    extensions: [".mjs", ".js", ".ts", ".tsx", ".json", ".jsx"],
  },
  module: {
    rules: [
      {
        enforce: "pre",
        test: /\.(js|mjs|jsx|ts|tsx|css)$/,
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
            test: cssRegex,
            exclude: cssModuleRegex,
            use: getStyleLoaders({
              importLoaders: 1,
              sourceMap: true,
              modules: {
                mode: "icss",
              },
            }),
            // Don't consider CSS imports dead code even if the
            // containing package claims to have no side effects.
            // Remove this when webpack adds a warning or an error for this.
            // See https://github.com/webpack/webpack/issues/6571
            sideEffects: true,
          },
          {
            test: cssModuleRegex,
            use: getStyleLoaders({
              importLoaders: 1,
              sourceMap: true,
              modules: {
                mode: "local",
                getLocalIdent: getCSSModuleLocalIdent,
              },
            }),
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
    new TryCatchPlugin(),
  ],
};

export default config;
