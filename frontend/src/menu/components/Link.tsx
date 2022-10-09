import { ControlContainer, ControlTitle } from "./Control";
import type { BaseControlProps } from "./Control";
import type { MouseEvent } from "react";

export interface LinkProps extends BaseControlProps {
  href: string;
  onClick?: (event: MouseEvent<HTMLAnchorElement>) => void;
}

export default function Link({
  title,
  attention,
  description,
  href,
  onClick,
}: LinkProps) {
  return (
    <ControlContainer description={description}>
      <a href={href} onClick={onClick}>
        <ControlTitle attention={attention}>{title}</ControlTitle>
      </a>
    </ControlContainer>
  );
}
