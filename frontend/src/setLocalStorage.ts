export const target = new EventTarget();

/**
 * Set/remove a localStorage key and trigger a hook from outside of a React component.
 *
 * @param key
 * @param value If null, the key will be deleted.
 */
export default function setLocalStorage(key: string, value: string | null) {
  if (value === null) localStorage.removeItem(key);
  else localStorage.setItem(key, value);
  target.dispatchEvent(new Event(`set ${key}`));
}
