/*
 * IDKR Userscript support (non-game)
 */
import EventEmitter from "../../EventEmitter";
import ColorPicker from "../../menu/components/ColorPicker";
import Control from "../../menu/components/Control";
import Select from "../../menu/components/Select";
import { Set } from "../../menu/components/Set";
import Slider from "../../menu/components/Slider";
import Switch from "../../menu/components/Switch";
import Text from "../../menu/components/Text";
import currentSite from "../../site";
import type {
  Config,
  SomeSetting,
  IClientUtil,
  ISettingsCollection,
} from "../idkrCommon";
import { createConfig, executeUserScript } from "../idkrCommon";
import { renderSettings } from "./chief";
import type { ComponentChild } from "preact";

const IDKRComponent = ({
  settings,
  config,
}: {
  settings: ISettingsCollection;
  config: Config;
}) => {
  const categories: Record<string, ComponentChild[]> = {};

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
      default: {
        const got = config.get(setting.id, setting.val);
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
              defaultValue={typeof got === "number" ? got.toString() : got}
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
    }
  },
});

/**
 * Run a IDKR userscript
 */
export default function idkrRuntime(script: string, code: string) {
  const userscript = executeUserScript(script, code, clientUtils);

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
