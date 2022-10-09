import Control from "./Control";
import type { BaseControlProps } from "./Control";
import type { ChangeEvent } from "react";

export interface SwitchProps extends BaseControlProps {
  checked?: boolean;
  defaultChecked?: boolean;
  onChange?: (event: ChangeEvent<HTMLInputElement>) => void;
}

export default function Switch({
  title,
  attention,
  description,
  checked,
  defaultChecked,
  onChange,
}: SwitchProps) {
  return (
    <Control title={title} attention={attention} description={description}>
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
