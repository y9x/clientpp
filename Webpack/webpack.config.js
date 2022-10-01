import TerserPlugin from "terser-webpack-plugin";
import { fileURLToPath } from "node:url";

const isDevelopment = process.env.NODE_ENV === "development";

/**
 * @type {import('webpack').Configuration}
 */
const config = {
  entry: fileURLToPath(new URL("./src/index.js", import.meta.url)),
  output: {
    path: fileURLToPath(new URL("./dist/", import.meta.url)),
    filename: "Webpack.js",
  },
  // inline work can be done from the client
  // base64 strings are 3x larger
  devtool: isDevelopment ? "eval" : "source-map",
  mode: isDevelopment ? "development" : "production",
  optimization: {
    minimize: !isDevelopment,
    minimizer: [
      new TerserPlugin({
        terserOptions: {
          mangle: {
            eval: true,
          },
          format: {
            quote_style: 1,
          },
        },
      }),
    ],
  },
  module: {
    rules: [
      {
        test: /\.m?js/,
        resolve: {
          fullySpecified: false,
        },
      },
      {
        test: /IPCMessages\.h$/,
        use: fileURLToPath(new URL("./IPCMessagesLoader.cjs", import.meta.url)),
      },
      {
        test: /\.css$/,
        use: [
          {
            loader: fileURLToPath(new URL("./css.cjs", import.meta.url)),
          },
        ],
      },
    ],
  },
};

export default config;
