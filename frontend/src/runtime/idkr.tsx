/*
 * IDKR Userscript support
 */
import EventEmitter from "../EventEmitter";
import SelectControl from "../menu/components/SelectControl";
import { Set } from "../menu/components/Set";
import SliderControl from "../menu/components/SliderControl";
import SwitchControl from "../menu/components/SwitchControl";
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
  get<T = unknown>(key: string, def: T): T {
    if (!(key in this.data) && def !== undefined) this.data[key] = def;

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

type SomeSetting = ISelectSetting | ICheckboxSetting | ISliderSetting;

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

type UserscriptContext = (
  this: {
    clientUtils: IClientUtil;
  },
  code: string,
  // eslint-disable-next-line @typescript-eslint/consistent-type-imports
  console: typeof import("../console").default
) => IUserscript;

const clientUtils: IClientUtil = Object.freeze({
  events: EventEmitter,
  genCSettingsHTML: (setting: SomeSetting) => (config: Config) => {
    switch (setting.type) {
      case "checkbox":
        return (
          <SwitchControl
            attention={setting.needsRestart}
            description={setting.needsRestart ? "Requires Restart" : undefined}
            title={setting.name}
            defaultChecked={config.get(setting.id, setting.val)}
            onChange={(event) => {
              config.set(setting.id, event.currentTarget.value);
              if (setting.needsRestart) global.location.reload();
            }}
          />
        );
      case "select":
        return (
          <SelectControl
            attention={setting.needsRestart}
            description={setting.needsRestart ? "Requires Restart" : undefined}
            title={setting.name}
            defaultValue={config.get(setting.id, setting.val)}
            onChange={(event) => {
              config.set(setting.id, event.currentTarget.value);
              if (setting.needsRestart) global.location.reload();
            }}
          >
            {Object.entries(setting.options).map(([id, name]) => (
              <option value={id}>{name}</option>
            ))}
          </SelectControl>
        );
      case "slider":
        return (
          <SliderControl
            attention={setting.needsRestart}
            description={setting.needsRestart ? "Requires Restart" : undefined}
            title={setting.name}
            defaultValue={config.get(setting.id, setting.val)}
            onChange={(event) => {
              config.set(setting.id, event.currentTarget.value);
              if (setting.needsRestart) global.location.reload();
            }}
          />
        );
    }
  },
});

const renderFactory = (settings: ISettingsCollection, config: Config) => {
  const Component = () => {
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

  return Component;
};

/**
 * Run a IDKR userscript
 */
export default function idkrRuntime(script: string, code: string) {
  // eslint-disable-next-line no-new-func
  const run = new Function(
    "code",
    "console",
    "eval(code)"
  ) as UserscriptContext;

  const magic = new MagicString(code);
  const identifier = "sourceMappingURL";

  magic.appendLeft(0, "()=>{");
  magic.append("}");

  const userscript = run.apply(
    {
      clientUtils,
    },
    [
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
    ]
  );

  if (
    userscript?.config?.locations?.includes("all") ||
    userscript?.config?.locations?.includes(currentSite || "unknown")
  ) {
    const config = createConfig(script);
    userscript.load(config);

    if (userscript.config.settings) {
      renderSettings.push(renderFactory(userscript.config.settings, config));
    }
  }
}
