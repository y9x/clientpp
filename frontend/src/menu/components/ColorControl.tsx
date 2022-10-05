import Control from "./Control";
import type { BaseControlProps } from "./Control";
import type { ChangeEvent } from "react";

export interface ColorControlProps extends BaseControlProps {
  value?: string;
  defaultValue?: string;
  onChange?: (event: ChangeEvent<HTMLInputElement>) => void;
}

export default function ColorControl({
  title,
  value,
  defaultValue,
  onChange,
}: ColorControlProps) {
  return (
    <Control title={title}>
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
