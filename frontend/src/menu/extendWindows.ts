/**
 * createRoot
 */
import type { RenderOnDemand } from "./renderContainer";
import createRenderContainer from "./renderContainer";

/**
 * Create a native react window.
 *
 * @returns Newly created window ID.
 */
export default function extendWindows(
  options: Omit<Omit<GameWindow, "gen">, "html">,
  render: RenderOnDemand
) {
  const html = createRenderContainer(render);

  const window = {
    ...options,
    gen: () => html,
    html: "",
  } as GameWindow;

  const id = windows.length;

  // we're breaking SO MUCH haha
  // they only trim the array once, in showWindow() ?

  windows = Object.create(windows);

  Reflect.defineProperty(windows, id, {
    // the game tends to delete our window.
    get: () => window,
    set: () => {
      //
    },
  });

  /*Reflect.defineProperty(windows, "length", {
    // the game tends to delete our window.
    get: () => id + 1,
    set: () => {
      // quit truncating the array!
    },
    configurable: true,
  });*/

  return id + 1;
}
