export const keybinds = new Set<Keybind>();

type Callback = (event: KeyboardEvent) => unknown;

export default interface Keybind {
  key: string;
  callback: Callback;
}

export function createKeybind(key: string, callback: Callback) {
  const keybind: Partial<Keybind> = {};

  keybind.key = key;
  keybind.callback = callback;

  keybinds.add(keybind as Keybind);

  return keybind as Keybind;
}

window.addEventListener("keydown", (event) => {
  if (event.repeat) return;

  for (const node of event.composedPath())
    if (node instanceof HTMLInputElement || node instanceof HTMLAreaElement)
      return;

  for (const keybind of keybinds)
    if (keybind.key === event.code) {
      event.preventDefault();
      keybind.callback(event);
    }
});
