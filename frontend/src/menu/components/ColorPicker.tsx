import Control from "./Control";
import type { BaseControlProps } from "./Control";
import type { ChangeEvent } from "react";

export interface ColorPickerProps extends BaseControlProps {
  value?: string;
  defaultValue?: string;
  onChange?: (event: ChangeEvent<HTMLInputElement>) => void;
}

export default function ColorPicker({
  title,
  attention,
  description,
  value,
  defaultValue,
  onChange,
}: ColorPickerProps) {
  return (
    <Control title={title} attention={attention} description={description}>
      <input
        type="color"
        name="color"
        style={{ float: "right" }}
        value={value}
        defaultValue={defaultValue}
        onChange={onChange}
      />
    </Control>
  );
}
