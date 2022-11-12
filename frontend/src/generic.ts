/**
 * Generic bundle (social, editor, viewer)
 */
import "./keybinds";
import "./resources";
import console from "./console";
import { js } from "./runtime";
import chiefRuntime from "./runtime/chief";
import idkrRuntime from "./runtime/idkr";

for (const [script, scriptID, code] of js)
  try {
    if (script.endsWith(".chief.js")) chiefRuntime(script, scriptID, code);
    else if (script.endsWith(".idkr.js")) idkrRuntime(script, scriptID, code);
  } catch (err) {
    console.error(`Failure loading ${script}:`);
    console.error(err);
  }
