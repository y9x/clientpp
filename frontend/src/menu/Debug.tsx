import ColorControl from "./components/ColorControl";
import LinkControl from "./components/LinkControl";
import { Set } from "./components/Set";
import SliderControl from "./components/SliderControl";

export default function DebugMenu() {
  return (
    <Set title="Debug">
      <LinkControl
        title="LinkControl"
        href="about:blank"
        onClick={() => alert("click")}
      />
      <SliderControl
        title="SliderControl"
        defaultValue={12}
        min={-100}
        max={100}
        step={4.5}
      />
      <ColorControl title="ColorControl" defaultValue="#FF99DA" />
    </Set>
  );
}
