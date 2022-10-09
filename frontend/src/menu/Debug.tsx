import console from "../console";
import ColorControl from "./components/ColorControl";
import FilePickerControl from "./components/FilePickerControl";
import LinkControl from "./components/LinkControl";
import { Set } from "./components/Set";
import SliderControl from "./components/SliderControl";

export default function DebugMenu() {
  return (
    <Set title="Debug">
      <FilePickerControl
        title="Upload File"
        accept=".js"
        multiple
        onChange={async (event) => {
          for (const file of event.currentTarget.files!) {
            console.log("Decoding:", file.name);

            const ab = await file.arrayBuffer();

            console.log(new TextDecoder().decode(ab));
          }
        }}
      />
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
