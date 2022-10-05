/**
 * createRoot
 */
import { createRoot } from "react-dom/client";
import { wait_for } from "../utils";
import HTMLProxy from "./HTMLProxy";

interface GameWindowTab {
  name: string;
  categories: [];
}

interface GameWindow {
  getSettings(): string;
  /**
   * Record<settingType: string, GameWindowTab[]>
   */
  tabs: Record<string, GameWindowTab[]>;
  settingType: string;
  tabIndex: number;
}

declare global {
  const windows: GameWindow[];
}

// settings window ID
// showWindow(x)
// x - 1
const id = 0;

export default async function extendSettings(name: string) {
  const htmlProxy = new HTMLProxy();

  const window = (
    await wait_for(
      () => typeof windows === "object" && Array.isArray(windows) && windows
    )
  )[id];

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

  window.getSettings = new Proxy(window.getSettings, {
    apply: (target, thisArg, argArray) =>
      window.tabIndex === indexes[window.settingType]
        ? htmlProxy.get()
        : Reflect.apply(target, thisArg, argArray),
  });

  return createRoot(htmlProxy);
}
