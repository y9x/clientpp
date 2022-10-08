import RPC from "../RPC";
import ipc, { IM } from "../ipc";
import { renderSettings } from "../runtime/chief";
import ButtonControl from "./components/ButtonControl";
import FileControl from "./components/FileControl";
import LinkControl from "./components/LinkControl";
import SelectControl from "./components/SelectControl";
import { HeadlessSet, Set } from "./components/Set";
import SwitchControl from "./components/SwitchControl";
import TextControl from "./components/TextControl";
import { useConfig } from "./useConfig";
import type { MouseEvent } from "react";

const seek_modes = [
  "Free for All",
  "Team Deathmatch",
  "Hardpoint",
  "Capture the Flag",
  "Parkour",
  "Hide & Seek",
  "Infected",
  "Race",
  "Last Man Standing",
  "Simon Says",
  "Gun Game",
  "Prop Hunt",
  "Boss Hunt",
  "unused",
  "unused",
  "Stalker",
  "King of the Hill",
  "One in the Chamber",
  "Trade",
  "Kill Confirmed",
  "Defuse",
  "Sharp Shooter",
  "Traitor",
  "Raid",
  "Blitz",
  "Domination",
  "Squad Deathmatch",
  "Kranked FFA",
];

function open_in_shell(event: MouseEvent<HTMLAnchorElement>) {
  event.preventDefault();
  ipc.send(IM.shell_open, "url", event.currentTarget.href);
}

const rpc = new RPC();

export default function Menu() {
  const [config, setConfig] = useConfig();

  return (
    <>
      <HeadlessSet>
        <LinkControl
          title="GitHub"
          href="https://github.com/6ct/clientpp"
          onClick={open_in_shell}
        />
        <LinkControl
          title="Discord"
          href="https://discord.gg/4r47ZwdSQj"
          onClick={open_in_shell}
        />
        <SwitchControl
          title="Devtools"
          defaultChecked={config.client.devtools}
          onChange={(event) => {
            config.client.devtools = event.currentTarget.checked;
            setConfig(config);
          }}
        />
      </HeadlessSet>
      <Set title="Folders">
        <ButtonControl
          title="Scripts"
          text="Open"
          onClick={() => ipc.send(IM.shell_open, "scripts")}
        />
        <ButtonControl
          title="Styles"
          text="Open"
          onClick={() => ipc.send(IM.shell_open, "styles")}
        />
        <ButtonControl
          title="Resource Swapper"
          text="Open"
          onClick={() => ipc.send(IM.shell_open, "swapper")}
        />
      </Set>
      <Set title="Rendering">
        <SwitchControl
          title="Fullscreen"
          defaultChecked={config.render.fullscreen}
          onChange={(event) => {
            config.render.fullscreen = event.currentTarget.checked;
            setConfig(config);
            ipc.send(IM.fullscreen);
          }}
        />
        <SwitchControl
          title="Uncap FPS"
          defaultChecked={config.render.uncap_fps}
          onChange={(event) => {
            config.render.uncap_fps = event.currentTarget.checked;
            setConfig(config);
            ipc.send(IM.relaunch_webview);
          }}
        />
        <SelectControl
          title="Angle backend"
          defaultValue={config.render.angle}
          onChange={(event) => {
            config.render.angle = event.currentTarget.value;
            setConfig(config);
            ipc.send(IM.relaunch_webview);
          }}
        >
          <option value="default">Default</option>
          <option value="d3d11on12">Direct3D 11 on 12</option>
          <option value="d3d11">Direct3D 11</option>
          <option value="d3d9">Direct3D 9 </option>
          <option value="gl">OpenGL</option>
        </SelectControl>
        <SelectControl
          title="Color profile"
          defaultValue={config.render.color}
          onChange={(event) => {
            config.render.color = event.currentTarget.value;
            setConfig(config);
            ipc.send(IM.relaunch_webview);
          }}
        >
          <option value="default">Default</option>
          <option value="d3d11on12">SRGB</option>
          <option value="generic-rgb">RGB</option>
        </SelectControl>
      </Set>
      <Set title="Game">
        <SwitchControl
          title="Seek new Lobby [F4]"
          defaultChecked={config.game.seek.F4}
          onChange={(event) => {
            config.game.seek.F4 = event.currentTarget.checked;
            setConfig(config);
          }}
        />
        <SwitchControl
          title="Custom seek (enables map, mode, and customs)"
          defaultChecked={config.game.seek.custom_logic}
          onChange={(event) => {
            config.game.seek.custom_logic = event.currentTarget.checked;
            setConfig(config);
          }}
        />
        <TextControl
          title="Seek Map"
          placeholder="Map ID"
          defaultValue={config.game.seek.map}
          onChange={(event) => {
            config.game.seek.map = event.currentTarget.value;
            setConfig(config);
          }}
        />
        <SelectControl
          title="Seek mode"
          defaultValue={config.game.seek.mode}
          onChange={(event) => {
            config.game.seek.mode = event.currentTarget.value;
            setConfig(config);
          }}
        >
          <option value="">Any</option>
          {seek_modes.map((mode, i) => (
            <option key={i}>{mode}</option>
          ))}
        </SelectControl>
        <SwitchControl
          title="Seek customs"
          defaultChecked={config.game.seek.customs}
          onChange={(event) => {
            config.game.seek.customs = event.currentTarget.checked;
            setConfig(config);
          }}
        />
      </Set>
      <Set title="Discord RPC">
        <SwitchControl
          title="Enabled"
          defaultChecked={config.rpc.enabled}
          onChange={(event) => {
            config.rpc.enabled = event.currentTarget.checked;
            setConfig(config);
            if (config.rpc.enabled) {
              ipc.send(IM.rpc_init);
              rpc.update(true);
            } else {
              ipc.send(IM.rpc_clear);
            }
          }}
        />
        <SwitchControl
          title="Show username"
          defaultChecked={config.rpc.name}
          onChange={(event) => {
            config.rpc.enabled = event.currentTarget.checked;
            setConfig(config);
            ipc.send(IM.rpc_clear);
            ipc.send(IM.rpc_init);
            rpc.update(true);
          }}
        />
      </Set>
      <Set title="Window">
        <SwitchControl
          title="Replace Icon & Title "
          defaultChecked={config.window.meta.replace}
          onChange={(event) => {
            config.window.meta.replace = event.currentTarget.checked;
            setConfig(config);
            if (config.window.meta.replace) ipc.send(IM.update_meta);
            else ipc.send(IM.revert_meta);
          }}
        />
        <TextControl
          title="New Title"
          defaultValue={config.window.meta.title}
          onChange={(event) => {
            config.window.meta.title = event.currentTarget.value;
            setConfig(config);
            if (config.window.meta.replace) ipc.send(IM.update_meta);
          }}
        />
        <FileControl
          title="New Icon"
          defaultValue={config.window.meta.icon}
          onPick={(value) => {
            config.window.meta.icon = value;
            setConfig(config);
            if (config.window.meta.replace) ipc.send(IM.update_meta);
          }}
        >
          <option value="*.ico">Icon</option>
          <option value="*.*">All types</option>
        </FileControl>
      </Set>
      {renderSettings.map((Settings, i) => (
        <Settings key={i} />
      ))}
    </>
  );
}
