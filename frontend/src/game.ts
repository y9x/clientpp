/**
 * Game bundle
 */
import "./keybinds";
import "./resources";
import "./menu/createUI";
import "./fixes";
import console from "./console";
import ipc, { IM } from "./ipc";
import { js } from "./runtime";
import chiefRuntime from "./runtime/game/chief";
import idkrRuntime from "./runtime/game/idkr";
import tampermonkeyRuntime from "./runtime/tampermonkey";
import { waitFor } from "./utils";

let lockedNode: Element | null;

ipc.send(IM.pointer, false);

window.addEventListener("beforeunload", () => {
  document.exitPointerLock();
  ipc.send(IM.pointer, false);
});

document.addEventListener("pointerlockchange", () => {
  if (!document.pointerLockElement) {
    lockedNode = null;
    return ipc.send(IM.pointer, false);
  }

  lockedNode = document.pointerLockElement;
  ipc.send(IM.pointer, true);
});

setInterval(() => {
  ipc.send(IM.pointer, !!document.pointerLockElement);
}, 200);

ipc.on(IM.mousewheel, (deltaY: number) => {
  lockedNode?.dispatchEvent(new WheelEvent("wheel", { deltaY }));
});

ipc.on(IM.mousedown, (button: number) => {
  lockedNode?.dispatchEvent(new MouseEvent("mousedown", { button }));
});

ipc.on(IM.mouseup, (button: number) => {
  lockedNode?.dispatchEvent(new MouseEvent("mouseup", { button }));
});

ipc.on(IM.mousemove, (movementX, movementY) => {
  lockedNode?.dispatchEvent(
    new MouseEvent("mousemove", { movementX, movementY })
  );
});

if (!localStorage.kro_setngss_scaleUI) localStorage.kro_setngss_scaleUI = "0.7";

const MIN_WIDTH = 1700;
const MIN_HEIGHT = 900;

if (localStorage.kro_setngss_uiScaling !== "false") {
  let scale_ui = localStorage.kro_setngss_scaleUI
    ? parseInt(localStorage.kro_setngss_scaleUI)
    : 0.7;

  scale_ui = Math.min(1, Math.max(0.1, Number(scale_ui)));
  scale_ui = 1 + (1 - scale_ui);

  const height = window.innerHeight;
  const width = window.innerWidth;
  const min_width = MIN_WIDTH * scale_ui;
  const min_height = MIN_HEIGHT * scale_ui;
  const width_scale = width / min_width;
  const height_scale = height / min_height;
  const style =
    height_scale < width_scale
      ? {
          transform: height_scale,
          width: width / height_scale,
          height: min_height,
        }
      : {
          transform: width_scale,
          width: min_width,
          height: height / width_scale,
        };

  waitFor(() => document.querySelector<HTMLDivElement>("#uiBase")).then(
    (ui_base) =>
      setTimeout(() => {
        ui_base.style.transform = "scale(" + style.transform.toFixed(3) + ")";
        ui_base.style.width = style.width.toFixed(3) + "px";
        ui_base.style.height = style.height.toFixed(3) + "px";
      }, 10)
  );
}

for (const [script, code] of js)
  try {
    if (script.endsWith(".chief.js")) chiefRuntime(script, code);
    else if (script.endsWith(".idkr.js")) idkrRuntime(script, code);
    else if (script.endsWith(".user.js")) tampermonkeyRuntime(script, code);
  } catch (err) {
    console.error(`Failure loading ${script}:`);
    console.error(err);
  }
