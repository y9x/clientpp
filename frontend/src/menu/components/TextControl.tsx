import Control from "./Control";
import type { BaseControlProps } from "./Control";
import type { ChangeEvent } from "react";

export interface TextControlProps extends BaseControlProps {
  value?: string;
  defaultValue?: string;
  onChange?: (event: ChangeEvent<HTMLInputElement>) => void;
}

export default function TextControl({
  title,
  value,
  defaultValue,
  onChange,
}: TextControlProps) {
  return (
    <Control title={title}>
      <input
        type="text"
        name="text"
        className="inputGrey2"
        placeholder="Custom Title"
        value={value}
        defaultValue={defaultValue}
        onChange={onChange}
      />
    </Control>
  );
}
