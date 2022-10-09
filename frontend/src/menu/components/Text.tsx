import Control from "./Control";
import type { BaseControlProps } from "./Control";
import type { ChangeEvent } from "react";

export interface TextProps extends BaseControlProps {
  placeholder?: string;
  value?: string;
  defaultValue?: string;
  /**
   * False by default.
   */
  spellCheck?: boolean;
  onChange?: (event: ChangeEvent<HTMLInputElement>) => void;
}

export default function Text({
  title,
  attention,
  description,
  placeholder,
  value,
  defaultValue,
  spellCheck,
  onChange,
}: TextProps) {
  return (
    <Control title={title} attention={attention} description={description}>
      <input
        type="text"
        name="text"
        className="inputGrey2"
        placeholder={placeholder}
        value={value}
        defaultValue={defaultValue}
        onChange={onChange}
        spellCheck={spellCheck === true}
      />
    </Control>
  );
}
