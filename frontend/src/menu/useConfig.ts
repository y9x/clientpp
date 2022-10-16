import ipc, { IM } from "../ipc";
import { config } from "../runtime";
import { useEffect, useState } from "preact/hooks";

const listeners = new Set<() => unknown>();
let currentConfig = config;

/**
 * Returns latest config.
 */
export default function getConfig() {
  return currentConfig;
}

ipc.on(IM.update_menu, (config) => setConfig(config));

/**
 * Sets the config and triggers hooks.
 */
export function setConfig(newConfig: typeof config) {
  ipc.send(IM.save_config, currentConfig);
  currentConfig = newConfig;
  for (const listener of listeners) listener();
}

export function useConfig(): [
  config: typeof config,
  setConfig: (newConfig: typeof config) => void
] {
  const [state, setState] = useState(currentConfig);

  useEffect(() => {
    const listener = () => {
      setState(currentConfig);
    };

    listeners.add(listener);

    return () => {
      listeners.delete(listener);
    };
  });

  return [state, setConfig];
}
