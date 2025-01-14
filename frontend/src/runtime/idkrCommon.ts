/*
 * IDKR Userscript support
 */
import { nameFunctionCode } from "./common";
import type { ComponentChild } from "preact";

// One too many interfaces had to be fixed... TODO: open PR @ https://github.com/idkr-client/idkr/blob/master/Userscripts.md#script-structure

interface IUserscriptMeta {
  author?: String;
  name?: String;
  version?: String;
  description?: String;
}

interface IUserscriptConfig {
  apiversion?: "1.0";
  locations?:
    | ("all" | "game" | "social" | "viewer" | "editor" | "unknown")[]
    | String[];
  platforms?: "all"[] | String[];
  settings?: ISettingsCollection;
}

type ConfigData = Record<string, unknown>;
type SaveConfigCallback = (data: ConfigData) => void;

export class Config {
  private data: ConfigData;
  saveConfig: SaveConfigCallback;
  constructor(data: ConfigData, saveConfig: SaveConfigCallback) {
    this.data = data;
    this.saveConfig = saveConfig;
  }
  /**
   *
   * @param key
   * @param def Default value if key isn't set.
   * @returns
   */
  get<T = unknown>(key: string, def: T): T {
    if (!(key in this.data) && typeof def !== "undefined") this.set(key, def);

    return this.data[key] as T;
  }
  set(key: string, value: unknown) {
    this.data[key] = value;
    this.saveConfig(this.data);
  }
}

export function createConfig(scriptID: string) {
  const lsID = "idkr_script_" + scriptID;

  let data: ConfigData;

  try {
    data = JSON.parse(localStorage.getItem(lsID) || "");
  } catch (err) {
    data = {};
  }

  return new Config(data, (data) =>
    localStorage.setItem(lsID, JSON.stringify(data))
  );
}

export interface IUserscript {
  config: IUserscriptConfig;
  meta: IUserscriptMeta;
  load(config: Config): void;
  unload(): void;
  clientUtils?: IClientUtil;
}

interface ISetting {
  id: string;
  name: string;
  info?: string;
  cat: string;
  // platforms?: NodeJS.Platform[];
  type: string;
  // type: "checkbox" | "select" | "text" | "slider" | string;
  needsRestart?: boolean;
  html(): (config: Config) => ComponentChild;
  set?(): void;
}

interface ISelectSetting extends ISetting {
  type: "select";
  options: Record<string, string>;
  val: string;
}

interface ICheckboxSetting extends ISetting {
  type: "checkbox";
  val: boolean;
}

interface ISliderSetting extends ISetting {
  type: "slider";
  max: number;
  min: number;
  step: number;
  val: number;
}

interface IColorSetting extends ISetting {
  type: "color";
  val: string;
}

interface ITextSetting extends ISetting {
  type: "text";
  val: string;
  placeholder?: string;
}

interface IUnknownSetting extends ISetting {
  type: "";
  val: string | number | undefined;
  placeholder?: string;
}

export type SomeSetting =
  | ISelectSetting
  | ICheckboxSetting
  | ISliderSetting
  // extended UI
  | ITextSetting
  | IColorSetting
  | IUnknownSetting;

export type ISettingsCollection = Record<string, SomeSetting>;

export interface IClientUtil {
  events: typeof import("../EventEmitter").default;
  // settings: { [key: string]: ISetting };
  // setCSetting(name: string, value: any): ComponentChild;
  // genCSettingsHTML(setting: ISetting): ComponentChild;
  // delayIDs: Record<string, NodeJS.Timeout>;
  // delaySetCSetting(name: String, target: HTMLInputElement, delay?: Number);
  // searchMatches(entry: ISetting);
  /**
   * Sane API to generate settings...
   */
  genCSettingsHTML(
    setting: SomeSetting
  ): (config: Config) => ComponentChild | void;
  // initUtil();
}

type IUserscriptExports =
  | (Partial<IUserscriptModule> & {
      run?: IUserscript["load"];
    })
  | void;
type IUserscriptModule = { exports: IUserscriptExports };

export type UserscriptContext = (
  code: string,
  console: typeof import("../console").default,
  clientUtils: IClientUtil,
  exports: IUserscriptExports,
  module: IUserscriptModule
) => IUserscript | void;

export function executeUserScript(
  script: string,
  scriptID: string,
  code: string,
  clientUtils: IClientUtil
): IUserscript {
  const run = new Function(
    "code",
    "console",
    "clientUtils",
    "exports",
    "module",
    "return eval(code)()"
  ) as UserscriptContext;

  const module = {
    exports: {},
  } as IUserscriptModule;

  const ret = run(
    nameFunctionCode(script, code),
    console,
    clientUtils,
    module.exports,
    module
  );

  if (typeof ret === "object" && ret !== null) ret.clientUtils = clientUtils;

  const userscript = {
    config: ret?.config || module?.exports,
    meta: ret?.meta || module?.exports,
    load: ret?.load?.bind(ret) || module?.exports?.run?.bind(module.exports),
    unload: ret?.unload,
  } as IUserscript;

  return userscript;
}
