import Control from "./Control";
import type { BaseControlProps } from "./Control";
import type { ChangeEvent, ReactNode } from "react";

export interface SelectProps extends BaseControlProps {
  children?: ReactNode;
  value?: string;
  defaultValue?: string;
  onChange?: (event: ChangeEvent<HTMLSelectElement>) => void;
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
