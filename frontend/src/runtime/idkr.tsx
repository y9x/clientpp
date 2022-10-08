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
  html(): string;
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

type ISettingsCollection = Record<
  string,
  ISelectSetting | ICheckboxSetting | ISliderSetting
>;

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
  genCSettingsHTML(
    setting: ISelectSetting | ICheckboxSetting | ISliderSetting
  ): ReactNode;
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

const clientUtils: IClientUtil = {
  events: EventEmitter,
  genCSettingsHTML: (setting) => {
    console.log("gen", setting, "...");
    return <></>;
  },
};

const renderFactory = (settings: ISettingsCollection, config: Config) => {
  const Component = () => {
    const categories: Record<string, ReactNode[]> = {};

    for (const setting in settings) {
      const data = settings[setting];

      if (!(data.cat in categories)) categories[data.cat] = [];

      const category = categories[data.cat];

      switch (data.type) {
        case "checkbox":
          category.push(
            <SwitchControl
              attention={data.needsRestart}
              description={data.needsRestart ? "Requires Restart" : undefined}
              title={data.name}
              defaultChecked={config.get(data.id, data.val)}
              key={setting}
              onChange={(event) => {
                config.set(data.id, event.currentTarget.value);
                if (data.needsRestart) global.location.reload();
              }}
            />
          );
          break;
        case "select":
          category.push(
            <SelectControl
              attention={data.needsRestart}
              description={data.needsRestart ? "Requires Restart" : undefined}
              title={data.name}
              defaultValue={config.get(data.id, data.val)}
              key={setting}
              onChange={(event) => {
                config.set(data.id, event.currentTarget.value);
                if (data.needsRestart) global.location.reload();
              }}
            >
              {Object.entries(data.options).map(([id, name]) => (
                <option value={id}>{name}</option>
              ))}
            </SelectControl>
          );
          break;
        case "slider":
          category.push(
            <SliderControl
              attention={data.needsRestart}
              description={data.needsRestart ? "Requires Restart" : undefined}
              title={data.name}
              defaultValue={config.get(data.id, data.val)}
              key={setting}
              onChange={(event) => {
                config.set(data.id, event.currentTarget.value);
                if (data.needsRestart) global.location.reload();
              }}
            />
          );
          break;
      }
    }

    return (
      <>
        {Object.entries(categories).map(([title, children]) => (
          <Set title={title}>{children}</Set>
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
    "return eval(code)();"
  ) as UserscriptContext;

  const userscript = run.apply(
    {
      clientUtils,
    },
    [
      "()=>{" +
        code +
        "\n}//# sourceURL=" +
        new URL("file:" + script).toString(),
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
