import Control from "./Control";
import type { BaseControlProps } from "./Control";
import type { MouseEvent } from "react";

export interface LinkControlProps extends BaseControlProps {
  href: string;
  onClick?: (event: MouseEvent<HTMLAnchorElement>) => void;
}

export default function LinkControl({
  title,
  href,
  onClick,
}: LinkControlProps) {
  return (
    <Control title={title} renderTitle="off">
      <a href={href} onClick={onClick}>
        {title}
      </a>
    </Control>
  );
}
