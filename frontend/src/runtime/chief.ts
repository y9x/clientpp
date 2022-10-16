/*
 * Chief Userscript support (non-game)
 */
import executeUserScript from "./chiefCommon";

/**
 * Run a Chief userscript
 */
export default function chiefRuntime(script: string, code: string) {
  executeUserScript(script, code, () => {
    // NOOP
  });
}
