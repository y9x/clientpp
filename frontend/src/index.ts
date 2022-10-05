import "./menu/createUI";
import "./fixes";
import "./resources";
import { createKeybind } from "./Keybind";
import ipc, { IM } from "./ipc";

window.addEventListener("beforeunload", (event) => {
  // Catch beforeunload event
  event.stopImmediatePropagation();
});

try {
  Reflect.defineProperty(window, "closeClient", {
    writable: false,
    value: () => ipc.send(IM.close_window),
  });
} catch (err) {
  //
}

createKeybind("F4", (event) => {
  if (event.altKey) ipc.send(IM.close_window);
  else ipc.send(IM.seek_game, localStorage.pingRegion7);
});

createKeybind("F11", () => {
  ipc.send(IM.toggle_fullscreen);
});

createKeybind("F10", () => {
  ipc.send(IM.open_devtools);
});

createKeybind("F5", () => ipc.send(IM.reload_window));
