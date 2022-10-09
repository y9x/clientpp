/*
 * createRoot
 */
import { waitFor } from "../utils";
import type { RenderOnDemand } from "./renderContainer";
import createRenderContainer from "./renderContainer";

// settings window ID
// showWindow(x)
// x - 1
const id = 0;

/**
 * Create a native react settings tab
 */
export default async function extendSettings(
  name: string,
  render: RenderOnDemand
) {
  const window = (
    await waitFor(
      () => typeof windows === "object" && Array.isArray(windows) && windows
    )
  )[id] as Settings | undefined;

  if (!window) throw new Error(`Couldn't find game window with ID ${id}`);

  /**
   * Keep track of the custom tab index
   */
  const indexes: Record<string, number> = {};

  for (const mode in window.tabs) {
    indexes[mode] = window.tabs[mode].length;

    window.tabs[mode].push({
      name,
      categories: [],
    });
  }

  const html = createRenderContainer(render);

  window.getSettings = new Proxy(window.getSettings, {
    apply: (target, thisArg, argArray) =>
      window.tabIndex === indexes[window.settingType]
        ? html
        : Reflect.apply(target, thisArg, argArray),
  });
}
