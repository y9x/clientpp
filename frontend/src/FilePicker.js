import { controlTypes } from "./libs/MenuUI/Control";
import utils from "./libs/Utils";
import { IM, ipc } from "./IPC";

export default class FilePicker extends controlTypes.TextBoxControl {
  static id = "filepicker";
  create(...args) {
    super.create(...args);
    this.browse = utils.add_ele("div", this.content, {
      className: "settingsBtn",
      textContent: "Browse",
      style: {
        width: "100px",
      },
      events: {
        click: async () => {
          // send entries instead of an object, c++ json parser removes the order
          const data = await ipc.post(
            IM.browse_file,
            this.data.title,
            Object.entries(this.data.filters)
          );

          this.value = this.input.value = data;
        },
      },
    });
  }
}

controlTypes.FilePicker = FilePicker;
