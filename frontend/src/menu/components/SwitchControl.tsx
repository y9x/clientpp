import Control from "./Control";
import type { BaseControlProps } from "./Control";
import type { ChangeEvent } from "react";

export interface SwitchControlProps extends BaseControlProps {
  checked?: boolean;
  defaultChecked?: boolean;
  onChange?: (event: ChangeEvent<HTMLInputElement>) => void;
}

export default function SwitchControl({
  title,
  checked,
  defaultChecked,
  onChange,
}: SwitchControlProps) {
  return (
    <Control title={title}>
      <label className="switch" style={{ marginLeft: 10 }}>
        <input
          type="checkbox"
          onChange={onChange}
          checked={checked}
          defaultChecked={defaultChecked}
        />
        <span className="slider"></span>
      </label>
    </Control>
  );
}
