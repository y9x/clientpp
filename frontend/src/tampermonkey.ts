/**
 * Dedicated Tampermonkey runtime
 */
import { js } from "./runtime";
import tampermonkeyRuntime from "./runtime/tampermonkey";

for (const [script, scriptID, code] of js)
  try {
    if (script.endsWith(".user.js"))
      tampermonkeyRuntime(script, scriptID, code);
  } catch (err) {
    console.error(`Failure loading ${script}:`);
    console.error(err);
  }
