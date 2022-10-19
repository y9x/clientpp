/*
 * Chief Userscript support (in-game)
 */
import Button from "../../menu/components/Button";
import ColorPicker from "../../menu/components/ColorPicker";
import Control from "../../menu/components/Control";
import FilePath from "../../menu/components/FilePath";
import FilePicker from "../../menu/components/FilePicker";
import Link from "../../menu/components/Link";
import Select from "../../menu/components/Select";
import { Set, HeadlessSet } from "../../menu/components/Set";
import Slider from "../../menu/components/Slider";
import Switch from "../../menu/components/Switch";
import Text from "../../menu/components/Text";
import useLocalStorage from "../../useLocalStorage";
import executeUserScript from "../chiefCommon";
import type { CallSettingsData } from "../chiefCommon";
import { html } from "htm/preact";
import type { FunctionComponent } from "preact";
import * as Preact from "preact/compat";

export const renderSettings: FunctionComponent[] = [];

const callSettingsData: CallSettingsData = Object.freeze({
  Preact,
  // expose `htm` to allow for manipulating JSX
  html,
  // expose components for building a GUI extension
  UI: Object.freeze({
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
  }),
  // important hook for lightweight configs
  useLocalStorage,
});

/**
 * Run a Chief userscript
 */
export default function chiefRuntime(script: string, code: string) {
  executeUserScript(script, code, ({ renderGameSettings }) => {
    if (renderGameSettings)
      renderSettings.push(() => renderGameSettings(callSettingsData) || null);
  });
}
