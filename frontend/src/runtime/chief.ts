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
import useLocalStorage, { setLocalStorage } from "../useLocalStorage";
import { sourceMappingURL } from "./common";
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
  console: typeof import("../console").default,
  getSite: () => typeof currentSite,
  // eslint-disable-next-line @typescript-eslint/consistent-type-imports
  React: typeof import("react"),
  // expose `htm` to allow for manipulating JSX
  html: typeof htm,
  // expose components for building a GUI extension
  UI: typeof UserscriptUI,
  // important hook for lightweight configs
  // eslint-disable-next-line @typescript-eslint/consistent-type-imports
  useLocalStorage: typeof import("../useLocalStorage").default,
  // eslint-disable-next-line @typescript-eslint/consistent-type-imports
  setLocalStorage: typeof import("../useLocalStorage").setLocalStorage,
  exportUserscript: ExportUserscriptCallback
) => void;

/**
 * Run a Chief userscript
 */
export default function chiefRuntime(script: string, code: string) {
  // eslint-disable-next-line no-new-func
  const run = new Function(
    "code",
    "console",
    "getSite",
    "React",
    "html",
    "UI",
    "useLocalStorage",
    "setLocalStorage",
    "exportUserscript",
    "eval(code)"
  ) as UserscriptContext;

  const magic = new MagicString(code);

  run(
    magic.toString() +
      "//# " +
      sourceMappingURL(
        magic.generateMap({
          source: new URL("file:" + script).toString(),
        })
      ),
    console,
    () => currentSite,
    React,
    htm.bind(React.createElement),
    UserscriptUI,
    useLocalStorage,
    setLocalStorage,
    (data) => {
      if (data.Settings) renderSettings.push(data.Settings);
    }
  );
}
