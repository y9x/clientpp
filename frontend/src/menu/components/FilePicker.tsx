import Control from "./Control";
import type { BaseControlProps } from "./Control";
import type { JSX } from "preact";
import { forwardRef } from "preact/compat";
import { useRef, useImperativeHandle } from "preact/hooks";

export interface FilePickerProps extends BaseControlProps {
  accept?: string;
  multiple?: boolean;
  value?: string;
  defaultValue?: string;
  onChange?: JSX.GenericEventHandler<HTMLInputElement>;
}

/**
 * Control to get the handle of a file.
 */
const FilePicker = forwardRef<HTMLInputElement, FilePickerProps>(
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

export default FilePicker;
