import type { ComponentChild } from "preact";

export type onChange<T> = (value: T, init: boolean) => void;

export interface BaseControlProps {
  /**
   * This control's title.
   */
  title: ComponentChild;
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
  children: ComponentChild;
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
  children: ComponentChild;
}) {
  return (
    <div className="settName" title={description}>
      {children}
    </div>
  );
}

/**
 * A control.
 */
export default function Control({
  title,
  description,
  attention,
  children,
}: BaseControlProps & {
  children?: ComponentChild;
}) {
  return (
    <div className="settName" title={description}>
      {title ? (
        <>
          <ControlTitle attention={attention}>{title}</ControlTitle> {children}
        </>
      ) : (
        children
      )}
    </div>
  );
}
