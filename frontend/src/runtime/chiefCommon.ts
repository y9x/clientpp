/*
 * Chief Userscript support (in-game)
 */
import type Button from "../menu/components/Button";
import type ColorPicker from "../menu/components/ColorPicker";
import type Control from "../menu/components/Control";
import type FilePath from "../menu/components/FilePath";
import type FilePicker from "../menu/components/FilePicker";
import type Link from "../menu/components/Link";
import type Select from "../menu/components/Select";
import type { Set, HeadlessSet } from "../menu/components/Set";
import type Slider from "../menu/components/Slider";
import type Switch from "../menu/components/Switch";
import type Text from "../menu/components/Text";
import setLocalStorage from "../setLocalStorage";
import currentSite from "../site";
import type useLocalStorage from "../useLocalStorage";
import { nameCode } from "./common";
import type { html } from "htm/preact";
import type { VNode } from "preact";
import type Preact from "preact/compat";

type RenderCallback = (data: CallSettingsData) => VNode | void;

export const renderSettings: RenderCallback[] = [];

export interface CallSettingsData {
  // expose more all-in-one preact
  // we want to expose: hooks, states, createElement, JSX
  Preact: typeof Preact;
  // expose `htm` to allow for manipulating JSX
  html: typeof html;
  // expose components for building a GUI extension
  UI: {
    Button: typeof Button;
    ColorPicker: typeof ColorPicker;
    Control: typeof Control;
    FilePath: typeof FilePath;
    FilePicker: typeof FilePicker;
    Link: typeof Link;
    Select: typeof Select;
    Set: typeof Set;
    HeadlessSet: typeof HeadlessSet;
    Slider: typeof Slider;
    Switch: typeof Switch;
    Text: typeof Text;
  };
  // important hook for lightweight configs
  useLocalStorage: typeof useLocalStorage;
}

interface ExportedUserscriptData {
  /**
   * Extend the client settings GUI.
   * We do not intend for you to build Preact hooks/components using this interface. This is simply for extending the in-game interface.
   */
  renderGameSettings?: RenderCallback;
}

type ExportUserscriptCallback = (data: ExportedUserscriptData) => void;

export type UserscriptContext = (
  code: string,
  // eslint-disable-next-line @typescript-eslint/consistent-type-imports
  console: typeof import("../console").default,
  getSite: () => typeof currentSite,
  // eslint-disable-next-line @typescript-eslint/consistent-type-imports
  setLocalStorage: typeof import("../setLocalStorage").default,
  exportUserscript: ExportUserscriptCallback
) => void;

/**
 * Run a Chief userscript
 */
export default function executeUserScript(
  script: string,
  code: string,
  exportUserscript: ExportUserscriptCallback
) {
  // eslint-disable-next-line no-new-func
  const run = new Function(
    "code",
    "console",
    "getSite",
    "setLocalStorage",
    "exportUserscript",
    "eval(code)"
  ) as UserscriptContext;

  run(
    nameCode(script, code, true),
    console,
    () => currentSite,
    setLocalStorage,
    exportUserscript
  );
}
