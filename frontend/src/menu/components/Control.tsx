import type { ReactNode } from "react";

export type onChange<T> = (value: T, init: boolean) => void;

export interface BaseControlProps {
  /**
   * This control's title.
   * Required for accessibility.
   */
  title: string;
}

export interface ControlProps extends BaseControlProps {
  /**
   * If the title will be rendered in Control. Recommended to not disable, however it is necessary for links.
   */
  renderTitle?: "on" | "off";
  /**
   * This control's body.
   */
  children?: ReactNode;
}

/**
 *
 */
export default function Control({
  title,
  children,
  renderTitle,
}: ControlProps) {
  return (
    <div className="settName" title={title}>
      {renderTitle !== "off" ? (
        <>
          {title} {children}
        </>
      ) : (
        children
      )}
    </div>
  );
}
