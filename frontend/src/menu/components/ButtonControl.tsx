import Control from "./Control";
import type { BaseControlProps } from "./Control";
import type { MouseEvent } from "react";

export interface ButtonControlProps extends BaseControlProps {
  text: string;
  onClick?: (event: MouseEvent<HTMLDivElement>) => void;
}

export default function ButtonControl({
  title,
  attention,
  description,
  text,
  onClick,
}: ButtonControlProps) {
  return (
    <Control title={title} attention={attention} description={description}>
      <div className="settingsBtn" onClick={onClick}>
        {text}
      </div>
    </Control>
  );
}
