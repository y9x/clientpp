/*
 * Chief Userscript support
 */
import ButtonControl from "../menu/components/ButtonControl";
import ColorControl from "../menu/components/ColorControl";
import Control from "../menu/components/Control";
import FilePathControl from "../menu/components/FilePathControl";
import FilePickerControl from "../menu/components/FilePickerControl";
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
  FilePathControl,
  FilePickerControl,
  LinkControl,
  SelectControl,
  Set,
  HeadlessSet,
  SliderControl,
  SwitchControl,
  TextControl,
});

export const renderSettings: Required<ExportedUserscriptData>["Settings"][] =
  [];

interface ExportedUserscriptData {
  /**
   * Core
   */
  main?: (data: UserscriptRuntime) => void;
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
      if (data.main)
        data.main({
          getSite: () => currentSite,
        });

      if (data.Settings) renderSettings.push(data.Settings);
    }
  );
}
