/*
 * IDKR Userscript support
 */
import EventEmitter from "../EventEmitter";
import ColorPicker from "../menu/components/ColorPicker";
import Control from "../menu/components/Control";
import Select from "../menu/components/Select";
import { Set } from "../menu/components/Set";
import Slider from "../menu/components/Slider";
import Switch from "../menu/components/Switch";
import Text from "../menu/components/Text";
import currentSite from "../site";
import { renderSettings } from "./chief";
import MagicString from "magic-string";
import type { ReactNode } from "react";

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
    | ("all" | "docs" | "game" | "social" | "viewer" | "editor" | "unknown")[]
    | String[];
  platforms?: "all"[] | String[];
  settings?: ISettingsCollection;
}

type ConfigData = Record<string, unknown>;
type SaveConfigCallback = (data: ConfigData) => void;

class Config {
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

function createConfig(script: string) {
  let data: ConfigData;

  try {
    data = JSON.parse(localStorage.getItem(script) || "");
  } catch (err) {
    data = {};
  }

  return new Config(data, (data) =>
    localStorage.setItem(script, JSON.stringify(data))
  );
}

interface IUserscript {
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
  html(): (config: Config) => ReactNode;
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
  val: string | number | ReadonlyArray<string> | undefined;
  placeholder?: string;
}

type SomeSetting =
  | ISelectSetting
  | ICheckboxSetting
  | ISliderSetting
  // extended UI
  | ITextSetting
  | IColorSetting
  | IUnknownSetting;

type ISettingsCollection = Record<string, SomeSetting>;

interface IClientUtil {
  events: typeof EventEmitter;
  // settings: { [key: string]: ISetting };
  // setCSetting(name: string, value: any): ReactNode;
  // genCSettingsHTML(setting: ISetting): ReactNode;
  // delayIDs: Record<string, NodeJS.Timeout>;
  // delaySetCSetting(name: String, target: HTMLInputElement, delay?: Number);
  // searchMatches(entry: ISetting);
  /**
   * Sane API to generate settings...
   */
  genCSettingsHTML(setting: SomeSetting): (config: Config) => ReactNode;
  // initUtil();
}

type IUserscriptExports =
  | (Partial<IUserscriptModule> & {
      run?: IUserscript["load"];
    })
  | void;
type IUserscriptModule = { exports: IUserscriptExports };

type UserscriptContext = (
  code: string,
  // eslint-disable-next-line @typescript-eslint/consistent-type-imports
  console: typeof import("../console").default,
  clientUtils: IClientUtil,
  exports: IUserscriptExports,
  module: IUserscriptModule
) => IUserscript;

const clientUtils: IClientUtil = Object.freeze({
  events: EventEmitter,
  genCSettingsHTML: (setting: SomeSetting) => (config: Config) => {
    switch (setting.type) {
      case "checkbox":
        return (
          <Switch
            attention={setting.needsRestart}
            description={setting.needsRestart ? "Requires Restart" : undefined}
            title={setting.name}
            defaultChecked={config.get(setting.id, setting.val)}
            onChange={(event) => {
              config.set(setting.id, event.currentTarget.checked);
              if (
                setting.needsRestart &&
                global.confirm(
                  "The game will be restarted for this setting to take affect."
                )
              )
                global.location.reload();
            }}
          />
        );
      case "select":
        return (
          <Select
            attention={setting.needsRestart}
            description={setting.needsRestart ? "Requires Restart" : undefined}
            title={setting.name}
            defaultValue={config.get(setting.id, setting.val)}
            onChange={(event) => {
              config.set(setting.id, event.currentTarget.value);
              if (
                setting.needsRestart &&
                global.confirm(
                  "The game will be restarted for this setting to take affect."
                )
              )
                global.location.reload();
            }}
          >
            {Object.entries(setting.options).map(([id, name]) => (
              <option value={id}>{name}</option>
            ))}
          </Select>
        );
      case "slider":
        return (
          <Slider
            attention={setting.needsRestart}
            description={setting.needsRestart ? "Requires Restart" : undefined}
            title={setting.name}
            defaultValue={config.get(setting.id, setting.val)}
            onChange={(event) => {
              config.set(setting.id, event.currentTarget.value);
              if (
                setting.needsRestart &&
                global.confirm(
                  "The game will be restarted for this setting to take affect."
                )
              )
                global.location.reload();
            }}
          />
        );
      // extended:
      case "color":
        return (
          <ColorPicker
            attention={setting.needsRestart}
            description={setting.needsRestart ? "Requires Restart" : undefined}
            title={setting.name}
            defaultValue={config.get(setting.id, setting.val)}
            onChange={(event) => {
              config.set(setting.id, event.currentTarget.value);
              if (
                setting.needsRestart &&
                global.confirm(
                  "The game will be restarted for this setting to take affect."
                )
              )
                global.location.reload();
            }}
          />
        );
      case "text":
        return (
          <Text
            attention={setting.needsRestart}
            description={setting.needsRestart ? "Requires Restart" : undefined}
            title={setting.name}
            placeholder={setting.placeholder}
            defaultValue={config.get(setting.id, setting.val)}
            onChange={(event) => {
              config.set(setting.id, event.currentTarget.value);
              if (
                setting.needsRestart &&
                global.confirm(
                  "The game will be restarted for this setting to take affect."
                )
              )
                global.location.reload();
            }}
          />
        );
      // catch-all similiar to the current one in IDKR
      default:
        return (
          <Control
            attention={setting.needsRestart}
            description={setting.needsRestart ? "Requires Restart" : undefined}
            title={setting.name}
          >
            <input
              type={setting.type}
              className="inputGrey2"
              placeholder={setting.placeholder}
              defaultValue={config.get(setting.id, setting.val)}
              onChange={(event) => {
                config.set(setting.id, event.currentTarget.value);
                if (
                  setting.needsRestart &&
                  global.confirm(
                    "The game will be restarted for this setting to take affect."
                  )
                )
                  global.location.reload();
              }}
            />
          </Control>
        );
    }
  },
});

const IDKRComponent = ({
  settings,
  config,
}: {
  settings: ISettingsCollection;
  config: Config;
}) => {
  const categories: Record<string, ReactNode[]> = {};

  for (const key in settings) {
    const setting = settings[key];

    if (!(setting.cat in categories)) categories[setting.cat] = [];

    const category = categories[setting.cat];

    const render = setting.html();

    category.push(render(config));
  }

  return (
    <>
      {Object.entries(categories).map(([title, children]) => (
        <Set title={title} key={title}>
          {children}
        </Set>
      ))}
    </>
  );
};

function executeUserscript(script: string, code: string): IUserscript {
  // eslint-disable-next-line no-new-func
  const run = new Function(
    "code",
    "console",
    "clientUtils",
    "exports",
    "module",
    "return eval(code)()"
  ) as UserscriptContext;

  const magic = new MagicString(code);
  const identifier = "sourceMappingURL";

  magic.appendLeft(0, "()=>{");
  magic.append("}");

  const module = {
    exports: {},
  } as IUserscriptModule;

  const ret = run(
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
    console,
    clientUtils,
    module.exports,
    module
  );

  if (ret) {
    ret.clientUtils = clientUtils;
  }

  const userscript = {
    config: ret?.config || module?.exports,
    meta: ret?.meta || module?.exports,
    load: ret?.load || module?.exports?.run,
    unload: ret?.unload,
  } as IUserscript;

  return userscript;
}

/**
 * Run a IDKR userscript
 */
export default function idkrRuntime(script: string, code: string) {
  const userscript = executeUserscript(script, code);

  if (
    userscript.config?.locations?.includes("all") ||
    userscript.config?.locations?.includes(currentSite || "unknown")
  ) {
    const config = createConfig(script);
    userscript.load(config);

    if (userscript.config.settings)
      renderSettings.push(() => (
        <IDKRComponent settings={userscript.config.settings!} config={config} />
      ));
  }
}
