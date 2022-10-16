import { target } from "./setLocalStorage";
import { useEffect, useState } from "preact/hooks";

/**
 * localStorage hook.
 * If another hook or setLocalStorage updates the key, this component will trigger a state update.
 */
export default function useLocalStorage(key: string): [
  string | null,
  /**
   * @param value If null, the key will be deleted.
   */
  (value: string | null) => void
] {
  // trigger re-render with useState
  const [state, setState] = useState(localStorage.getItem(key));

  const event = `set ${key}`;

  useEffect(() => {
    function listener() {
      setState(localStorage.getItem(key));
    }

    target.addEventListener(event, listener);

    return () => target.removeEventListener(event, listener);
  });

  return [
    state,
    (value) => {
      // null = nuke the item
      if (value === null) localStorage.removeItem(key);
      else localStorage.setItem(key, value);

      setState(value);
      target.dispatchEvent(new Event(event));
    },
  ];
}

/**
 * Set/remove a localStorage key and trigger a hook from outside of a React component.
 *
 * @param key
 * @param value If null, the key will be deleted.
 */
export function setLocalStorage(key: string, value: string | null) {
  if (value === null) localStorage.removeItem(key);
  else localStorage.setItem(key, value);
  target.dispatchEvent(new Event(`set ${key}`));
}
