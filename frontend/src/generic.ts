/**
 * Generic bundle (social, editor, viewer)
 */
import "./keybinds";
import "./resources";
import console from "./console";
import { js } from "./runtime";
import chiefRuntime from "./runtime/chief";
import idkrRuntime from "./runtime/idkr";

for (const [script, code] of js)
  try {
    if (script.endsWith(".chief.js")) chiefRuntime(script, code);
    else if (script.endsWith(".idkr.js")) idkrRuntime(script, code);
  } catch (err) {
    console.error(`Failure loading ${script}:`);
    console.error(err);
  }
