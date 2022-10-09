import Control from "./Control";
import type { BaseControlProps } from "./Control";
import type { MouseEvent } from "react";

export interface ButtonProps extends BaseControlProps {
  text: string;
  onClick?: (event: MouseEvent<HTMLDivElement>) => void;
}

export default function Button({
  title,
  attention,
  description,
  text,
  onClick,
}: ButtonProps) {
  return (
    <Control title={title} attention={attention} description={description}>
      <div className="settingsBtn" onClick={onClick}>
        {text}
      </div>
    </Control>
  );
}
