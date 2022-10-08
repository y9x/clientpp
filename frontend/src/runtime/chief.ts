/*
 * Chief Userscript support
 */
import ipc, { IM, ipcConsole } from "../ipc";
import ButtonControl from "../menu/components/ButtonControl";
import ColorControl from "../menu/components/ColorControl";
import Control from "../menu/components/Control";
import FileControl from "../menu/components/FileControl";
import LinkControl from "../menu/components/LinkControl";
import SelectControl from "../menu/components/SelectControl";
import { Set, HeadlessSet } from "../menu/components/Set";
import SliderControl from "../menu/components/SliderControl";
import SwitchControl from "../menu/components/SwitchControl";
import TextControl from "../menu/components/TextControl";
import currentSite from "../site";
import useLocalStorage from "../useLocalStorage";
import htm from "htm";
import MagicString from "magic-string";
import type { FunctionComponent } from "react";
import React from "react";

const UserscriptUI = Object.freeze({
  ButtonControl,
  ColorControl,
  Control,
  FileControl,
  LinkControl,
  SelectControl,
  Set,
  HeadlessSet,
  SliderControl,
  SwitchControl,
  TextControl,
});

type ResourceType =
  | "all"
  | "document"
  | "stylesheet"
  | "image"
  | "media"
  | "font"
  | "x"
  | "fetch"
  | "websocket"
  | "manifest"
  | "signed"
  | "ping"
  | "other"
  | "unknown";

const urlFilters: [
  script: string,
  filter: Required<ExportedUserscriptData>["filterURL"]
][] = [];

export const renderSettings: Required<ExportedUserscriptData>["Settings"][] =
  [];

ipc.on(IM.will_block_url, (id: number, url: string, type: ResourceType) => {
  for (const [script, filter] of urlFilters)
    if (filter(new URL(url), type)) {
      ipcConsole.debug(`${script} blocked ${type}: ${url}`);
      return ipc.send(id, true);
    }

  ipc.send(id, false);
});

interface ExportedUserscriptData {
  /**
   * Core
   */
  main?: (data: UserscriptRuntime) => void;
  /*
   * Feature - Filters an incoming network request.
   */
  filterURL?: (url: URL, resourceType: ResourceType) => boolean;
  /**
   * Feature - Extends the settings GUI.
   */
  Settings?: FunctionComponent;
}

type ExportUserscriptCallback = (data: ExportedUserscriptData) => void;

type UserscriptContext = (
  code: string,
  // eslint-disable-next-line @typescript-eslint/consistent-type-imports
  React: typeof import("react"),
  // expose `htm` to allow for manipulating JSX
  html: typeof htm,
  // important hook for lightweight configs
  // eslint-disable-next-line @typescript-eslint/consistent-type-imports
  useLocalStorage: typeof import("../useLocalStorage").default,
  // expose components for building a GUI extension
  UI: typeof UserscriptUI,
  // eslint-disable-next-line @typescript-eslint/consistent-type-imports
  console: typeof import("../console").default,
  exportUserscript: ExportUserscriptCallback
) => void;

interface UserscriptRuntime {
  getSite: () => typeof currentSite;
}

/**
 * Run a Chief userscript
 */
export default function chiefRuntime(script: string, code: string) {
  // eslint-disable-next-line no-new-func
  const run = new Function(
    "code",
    "React",
    "html",
    "useLocalStorage",
    "UI",
    "console",
    "exportUserscript",
    "eval(code)"
  ) as UserscriptContext;

  const magic = new MagicString(code);
  const identifier = "sourceMappingURL";

  run(
    magic.toString() +
      "//# " +
      identifier +
      "=data:application/json," +
      encodeURI(
        JSON.stringify(
          magic.generateMap({
            source: new URL("file:" + script).toString(),
          })
        )
      ),
    React,
    htm.bind(React.createElement),
    useLocalStorage,
    UserscriptUI,
    console,
    (data) => {
      if (data.filterURL) urlFilters.push([script, data.filterURL]);

      if (data.main)
        data.main({
          getSite: () => currentSite,
        });

      if (data.Settings) renderSettings.push(data.Settings);
    }
  );
}
