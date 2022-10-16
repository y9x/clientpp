import Control from "./Control";
import type { BaseControlProps } from "./Control";
import type { JSX, ComponentChild } from "preact";

export interface SelectProps extends BaseControlProps {
  children?: ComponentChild;
  value?: string;
  defaultValue?: string;
  onChange?: JSX.GenericEventHandler<HTMLSelectElement>;
}

export default function Select({
  title,
  attention,
  description,
  children,
  defaultValue,
  value,
  onChange,
}: SelectProps) {
  return (
    <Control title={title} attention={attention} description={description}>
      <select
        className="inputGrey2"
        value={value}
        defaultValue={defaultValue}
        onChange={onChange}
      >
        {children}
      </select>
    </Control>
  );
}
