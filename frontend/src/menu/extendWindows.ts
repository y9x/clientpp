/**
 * createRoot
 */
import { wait_for } from "../utils";
import type { RenderOnDemand } from "./settHolderProxy";
import createSettHolderProxy from "./settHolderProxy";

/**
 * Create a native react window.
 *
 * @returns Newly created window ID.
 */
export default async function extendWindows(
  header: string,
  label: string,
  render: RenderOnDemand
): Promise<number> {
  const html = createSettHolderProxy(render);

  const window = {
    header,
    label,
    gen: () => html,
    width: 1100,
    popup: false,
  } as GameWindow;

  const wins = await wait_for(
    () => typeof windows === "object" && Array.isArray(windows) && windows
  );

  const id = wins.length;

  Reflect.defineProperty(wins, id, {
    // the game tends to delete our window.
    writable: false,
    value: window,
  });

  return id;
}
