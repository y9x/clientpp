import type { ReactNode } from "react";

export type onChange<T> = (value: T, init: boolean) => void;

export interface BaseControlProps {
  /**
   * This control's title.
   */
  title: string;
  /**
   * Description of the control.
   */
  description?: string;
  /**
   * Asterisk symbol next to settings that require attention.
   */
  attention?: boolean;
}

/**
 * Base title
 */
export function ControlTitle({
  attention,
  children,
}: {
  attention?: boolean;
  children: string;
}) {
  return (
    <>
      {children} {attention && <span style={{ color: "#eb5656" }}>*</span>}
    </>
  );
}

/**
 * Base title
 */
export function ControlContainer({
  description,
  children,
}: {
  description?: string;
  children: ReactNode;
}) {
  return (
    <div className="settName" title={description}>
      {children}
    </div>
  );
}

/**
 * A control. If title isn't specified, this control will simply become a container.
 */
export default function Control({
  title,
  description,
  attention,
  children,
}: BaseControlProps & {
  children: ReactNode;
}) {
  return (
    <div className="settName" title={description}>
      {typeof title === "string" ? (
        <>
          <ControlTitle attention={attention}>{title}</ControlTitle> {children}
        </>
      ) : (
        children
      )}
    </div>
  );
}
