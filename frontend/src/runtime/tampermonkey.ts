/*
 * Tampermonky Userscript support
 */

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

  // eslint-disable-next-line no-new-func
  const run = new Function("code", "eval(code)") as (code: string) => void;

  run(code + "\n//# sourceURL=" + new URL("file:" + script).toString());
}
