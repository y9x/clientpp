/*
 * Chief Userscript support (non-game)
 */
import executeUserScript from "./chiefCommon";

/**
 * Run a Chief userscript
 * @param script The userscript's path in the filesystem (for debugging purposes).
 * @param scriptID The unique identifier of the userscript.
 * @param code The userscript's source code.
 */
export default function chiefRuntime(
  script: string,
  scriptID: string,
  code: string
) {
  executeUserScript(script, scriptID, code, () => {
    // NOOP
  });
}
