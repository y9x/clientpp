/*
 * Tampermonky Userscript support
 */
import console from "../console";
import MagicString from "magic-string";
import { parse } from "match-pattern";

function* getComments(script: string) {
  for (const line of script.split("\n")) {
    const commentI = line.indexOf("//");
    if (commentI === -1) continue;
    const comment = line.slice(commentI + 2).trim();
    yield comment.trim();
  }
}

const parseMetadata = (script: string) => {
  const metadata = {
    iterable: {
      include: [] as string[],
      match: [] as string[],
      exclude: [] as string[],
      require: [] as string[],
      resource: [] as string[],
      connect: [] as string[],
      grant: [] as string[],
    },
    single: {} as Record<string, string>,
  };

  const comments = getComments(script);

  let userscript = false;

  for (const comment of comments) {
    if (!userscript) {
      if (comment === "==UserScript==") userscript = true;
      continue;
    }

    for (const comment of comments) {
      if (comment === "==/UserScript==") break;

      const spaceI = comment.search(/\s/);

      const attribute =
        spaceI === -1 ? comment : comment.slice(0, spaceI).trim();
      const value = spaceI === -1 ? "" : comment.slice(spaceI + 1).trim();

      if (!attribute.startsWith("@")) continue;

      const name = attribute.slice(1);

      if (name in metadata.iterable)
        metadata.iterable[name as keyof typeof metadata.iterable].push(value);
      else metadata.single[name] = value;
    }
  }

  if (userscript) return metadata;
};

type UserscriptContext = (
  code: string,
  // eslint-disable-next-line @typescript-eslint/consistent-type-imports
  console: typeof import("../console").default,
  unsafeWindow: typeof globalThis,
  GM_getValue: (key: string) => string | null,
  GM_setValue: (key: string, value: string) => void
) => void;

function parseIncludeExclude(value: string) {
  if (value.startsWith("/") && value.endsWith("/"))
    return new RegExp(value.slice(1, -1).replace(/\//, "\\/"));
  else return parse(value);
}

/**
 * Run a Tampermonkey userscript
 */
export default function tampermonkeyRuntime(script: string, code: string) {
  /*
   * TODO: read tampermonkey header
   * Provide shim GM_ functions such as getValue & setValue depending on @grant
   * Provide unsafeWindow depending on @grant
   * May require emulating the localized window variables...
   */

  const metadata = parseMetadata(code);

  if (metadata) {
    const testURL = new URL(window.location.toString());
    testURL.search = "";
    testURL.hash = "";
    const testStr = testURL.toString();

    let matched =
      !metadata.iterable.include.length && !metadata.iterable.match.length;

    if (!matched)
      for (const match of metadata.iterable.match)
        if (parse(match).test(testStr)) {
          matched = true;
          break;
        }

    if (!matched)
      for (const match of metadata.iterable.include)
        if (parseIncludeExclude(match).test(testStr)) {
          matched = true;
          break;
        }

    if (matched)
      for (const exclude of metadata.iterable.exclude)
        if (parseIncludeExclude(exclude).test(testStr)) {
          matched = false;
          break;
        }

    if (!matched) return;

    for (const require of metadata.iterable.require.map((req) =>
      new URL(req).toString()
    )) {
      const xml = new XMLHttpRequest();
      xml.open("GET", require, false);
      xml.send();

      const magic = new MagicString(xml.responseText);
      const identifier = "sourceMappingURL";

      // eslint-disable-next-line no-new-func
      const run = new Function("code", "eval(code)") as (code: string) => void;

      run(
        magic.toString() +
          "//# " +
          identifier +
          "=data:application/json," +
          encodeURI(
            JSON.stringify(
              magic.generateMap({
                source: require,
              })
            )
          )
      );
    }
  }

  // eslint-disable-next-line no-new-func
  const run = new Function(
    "code",
    "console",
    "unsafeWindow",
    "GM_getValue",
    "GM_setValue",
    "eval(code)"
  ) as UserscriptContext;

  const magic = new MagicString(code);
  const identifier = "sourceMappingURL";

  run(
    magic.toString() +
      "//# " +
      identifier +
      "=data:application/json," +
      encodeURI(
        JSON.stringify(
          magic.generateMap({
            source: new URL("file:" + script).toString(),
          })
        )
      ),
    console,
    window,
    (key) => localStorage.getItem(key),
    (key, value) => localStorage.setItem(key, value)
  );
}
