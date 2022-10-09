import Control from "./Control";
import type { BaseControlProps } from "./Control";
import type { ChangeEventHandler } from "react";
import { forwardRef, useRef, useImperativeHandle } from "react";

export interface FilePickerControlProps extends BaseControlProps {
  accept?: string;
  multiple?: boolean;
  value?: string;
  defaultValue?: string;
  onChange?: ChangeEventHandler<HTMLInputElement>;
}

/**
 * Control to get the handle of a file.
 */
const FilePickerControl = forwardRef<HTMLInputElement, FilePickerControlProps>(
  (
    { title, attention, description, value, defaultValue, multiple, onChange },
    ref
  ) => {
    const picker = useRef<HTMLInputElement | null>(null);

    useImperativeHandle(ref, () => picker.current!, [picker]);

    return (
      <Control title={title} attention={attention} description={description}>
        <input
          ref={picker}
          hidden
          type="file"
          value={value}
          defaultValue={defaultValue}
          onChange={onChange}
          multiple={multiple}
        />
        <div
          className="settingsBtn"
          style={{ width: 100 }}
          onClick={() => picker.current!.click()}
        >
          Browse
        </div>
      </Control>
    );
  }
);

export default FilePickerControl;
