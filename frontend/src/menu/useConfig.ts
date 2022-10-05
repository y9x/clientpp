import { useEffect, useState } from "react";
import ipc, { IM } from "../ipc";
import { config } from "../runtime";
const listeners = new Set<() => unknown>();
let currentConfig = config;

/**
 * Returns latest config.
 */
export default function getConfig() {
  return currentConfig;
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

  return [
    state,
    (newConfig) => {
      ipc.send(IM.save_config, currentConfig);
      currentConfig = newConfig;
      for (const listener of listeners) listener();
    },
  ];
}
