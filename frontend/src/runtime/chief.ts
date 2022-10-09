/*
 * Chief Userscript support
 */
import Button from "../menu/components/Button";
import ColorPicker from "../menu/components/ColorPicker";
import Control from "../menu/components/Control";
import FilePath from "../menu/components/FilePath";
import FilePicker from "../menu/components/FilePicker";
import Link from "../menu/components/Link";
import Select from "../menu/components/Select";
import { Set, HeadlessSet } from "../menu/components/Set";
import Slider from "../menu/components/Slider";
import Switch from "../menu/components/Switch";
import Text from "../menu/components/Text";
import currentSite from "../site";
import useLocalStorage from "../useLocalStorage";
import htm from "htm";
import MagicString from "magic-string";
import type { FunctionComponent } from "react";
import React from "react";

const UserscriptUI = Object.freeze({
  Button,
  ColorPicker,
  Control,
  FilePath,
  FilePicker,
  Link,
  Select,
  Set,
  HeadlessSet,
  Slider,
  Switch,
  Text,
});

export const renderSettings: FunctionComponent[] = [];

interface ExportedUserscriptData {
  /**
   * Extend the client settings GUI.
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
  getSite: () => typeof currentSite,
  exportUserscript: ExportUserscriptCallback
) => void;

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
    "getSite",
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
    () => currentSite,
    (data) => {
      if (data.Settings) renderSettings.push(data.Settings);
    }
  );
}
