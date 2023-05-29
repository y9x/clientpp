/* eslint-disable @typescript-eslint/no-explicit-any */
/**
 * Game bundle
 */
import "./keybinds";
import "./resources";
import "./menu/createUI";
import console from "./console";
import ipc, { IM } from "./ipc";
import { js } from "./runtime";
import chiefRuntime from "./runtime/game/chief";
import idkrRuntime from "./runtime/game/idkr";
import { waitFor } from "./utils";

ipc.send(IM.pointer, false);

window.addEventListener("beforeunload", () => {
  document.exitPointerLock();
  ipc.send(IM.pointer, false);
});

let wheelListener: ((event: WheelEvent) => void) | undefined;
let mouseDownListener: ((event: MouseEvent) => void) | undefined;
let mouseUpListener: ((event: MouseEvent) => void) | undefined;
/**
 * type bypasses event.isTrusted
 * used for fakeMouse
 */
let mouseMoveListener: ((event: MouseEvent, type?: string) => void) | undefined;

HTMLCanvasElement.prototype.addEventListener = function (
  type: string,
  listener: any,
  options: any
) {
  // just catch the first canvas listeners
  switch (type) {
    case "wheel":
      wheelListener ||= listener;
      break;
    case "mousedown":
      mouseDownListener ||= listener;
      break;
    case "mouseup":
      mouseUpListener ||= listener;
      break;
    case "mousemove":
      mouseMoveListener ||= listener;
      break;
  }

  EventTarget.prototype.addEventListener.call(this, type, listener, options);
};

document.addEventListener("pointerlockchange", () => {
  if (!document.pointerLockElement) return ipc.send(IM.pointer, false);
  ipc.send(IM.pointer, true);
});

setInterval(() => {
  ipc.send(IM.pointer, !!document.pointerLockElement);
}, 200);

function spoofEvent<E extends Event>(event: E) {
  return new Proxy(event, {
    get: (target, key) => {
      if (key === "isTrusted") return true;
      if (key === "cancelable") return true;
      const value = Reflect.get(target, key);
      if (typeof value === "function") return value.bind(target);
      return value;
    },
  });
}

ipc.on(IM.mousewheel, (deltaY: number) => {
  if (wheelListener)
    wheelListener(spoofEvent(new WheelEvent("wheel", { deltaY })));
});

ipc.on(IM.mousedown, (button: number) => {
  if (mouseDownListener)
    mouseDownListener(spoofEvent(new MouseEvent("mousedown", { button })));
});

ipc.on(IM.mouseup, (button: number) => {
  if (mouseUpListener)
    mouseUpListener(spoofEvent(new MouseEvent("mouseup", { button })));
});

ipc.on(IM.mousemove, (movementX, movementY) => {
  if (mouseMoveListener)
    mouseMoveListener(
      spoofEvent(new MouseEvent("mousemove", { movementX, movementY }))
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

for (const [script, scriptID, code] of js)
  try {
    if (script.endsWith(".chief.js")) chiefRuntime(script, scriptID, code);
    else if (script.endsWith(".idkr.js")) idkrRuntime(script, scriptID, code);
  } catch (err) {
    console.error(`Failure loading ${script}:`);
    console.error(err);
  }
