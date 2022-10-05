import Control from "./Control";
import type { BaseControlProps } from "./Control";
import type { MouseEvent } from "react";

export interface ButtonControlProps extends BaseControlProps {
  text: string;
  onClick?: (event: MouseEvent<HTMLDivElement>) => void;
}

export default function ButtonControl({
  title,
  text,
  onClick,
}: ButtonControlProps) {
  return (
    <Control title={title}>
      <div className="settingsBtn" onClick={onClick}>
        {text}
      </div>
    </Control>
  );
}
