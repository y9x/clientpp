/**
 * createRoot
 */
import type { RenderOnDemand } from "./settHolderProxy";
import createSettHolderProxy from "./settHolderProxy";

/**
 * Create a native react window.
 *
 * @returns Newly created window ID.
 */
export default function extendWindows(
  options: Omit<Omit<GameWindow, "gen">, "html">,
  render: RenderOnDemand
) {
  const html = createSettHolderProxy(render);

  const window = {
    ...options,
    gen: () => html,
    html: "",
  } as GameWindow;

  const id = windows.length;

  Reflect.defineProperty(windows, id, {
    // the game tends to delete our window.
    writable: false,
    value: window,
  });

  return id + 1;
}
