import type { SourceMap } from "magic-string";
import MagicString from "magic-string";
import { transfer } from "multi-stage-sourcemap";
import { parse } from "path";
import webpack from "webpack";
import type { Compilation, Compiler, WebpackPluginInstance } from "webpack";

type Source = Compilation["assets"][""];

export interface Options {
  /**
   * If source maps should be produced.
   */
  sourceMap?: boolean;
  /**
   * Allowed file extensions, each starting with a period.
   * @default ['.js', '.mjs']
   */
  allowedExtensions?: string[];
  /**
   * Code to execute when an error is caught. `err` is the variable
   */
  catch?: string;
}

interface NormalizedOptions {
  sourceMap: boolean;
  allowedExtensions: string[];
  catch: string;
}

export default class TryCatchWebpackPlugin implements WebpackPluginInstance {
  private options: NormalizedOptions;
  constructor(options: Options = {}) {
    this.options = {
      sourceMap: !!options.sourceMap,
      allowedExtensions:
        Array.isArray(options.allowedExtensions) &&
        options.allowedExtensions.every(
          (value) => typeof value === "string" && value.startsWith(".")
        )
          ? options.allowedExtensions
          : [".js", ".mjs"],
      catch: options.catch || "console.error(err);",
    };
  }
  apply(compiler: Compiler) {
    compiler.hooks.compilation.tap("TryCatchWebpackPlugin", (compilation) => {
      compilation.hooks.processAssets.tap(
        {
          name: "TryCatchWebpackPlugin",
          stage: webpack.Compilation.PROCESS_ASSETS_STAGE_DEV_TOOLING,
        },
        (assets) => {
          const sourcemapOutput: Record<string, SourceMap> = {};

          for (const chunk of compilation.chunks) {
            for (const fileName of chunk.files) {
              const parsedFileName = parse(fileName);

              if (this.options.sourceMap && parsedFileName.ext === ".map") {
                const srcName = fileName
                  .toLowerCase()
                  .slice(0, fileName.length - 4);

                const transferredSourceMap = transfer({
                  fromSourceMap: sourcemapOutput[srcName],
                  toSourceMap: compilation.assets[fileName].source().toString(),
                });

                const finalSourcemap = JSON.parse(transferredSourceMap);

                finalSourcemap["sourcesContent"] = JSON.parse(
                  assets[fileName].source().toString()
                )["sourcesContent"];

                assets[fileName] = new webpack.sources.RawSource(
                  JSON.stringify(finalSourcemap),
                  false
                );

                continue;
              }

              if (!this.options.allowedExtensions.includes(parsedFileName.ext))
                continue;

              const asset = compilation.assets[fileName];
              const { inputSource, inputSourceMap } =
                this.extractSourceAndSourceMap(asset);

              const magic = new MagicString(inputSource.toString());

              magic.appendLeft(0, "try{");
              magic.append("}catch(err){" + this.options.catch + "}");

              const patchSource = magic.toString();

              if (this.options.sourceMap && inputSourceMap) {
                const patchSourceMap = magic.generateMap({
                  source: fileName,
                  file: fileName + ".map",
                  // quality is quickly lost when set to false
                  hires: true,
                });

                sourcemapOutput[fileName] = patchSourceMap;

                const transferredSourceMap = transfer({
                  fromSourceMap: JSON.stringify(patchSourceMap),
                  toSourceMap: JSON.stringify(inputSourceMap),
                });

                const finalSourcemap = JSON.parse(transferredSourceMap);

                finalSourcemap["sourcesContent"] = (
                  inputSourceMap as Record<string, unknown>
                )["sourcesContent"];

                assets[fileName] = new webpack.sources.SourceMapSource(
                  patchSource,
                  fileName,
                  finalSourcemap
                );
              } else {
                assets[fileName] = new webpack.sources.RawSource(
                  patchSource,
                  false
                );
              }
            }
          }
        }
      );
    });
  }
  private extractSourceAndSourceMap(asset: Source) {
    if (asset.sourceAndMap) {
      const { source, map } = asset.sourceAndMap();
      return { inputSource: source, inputSourceMap: map };
    } else {
      return {
        inputSource: asset.source(),
        inputSourceMap: asset.map() as unknown,
      };
    }
  }
}
