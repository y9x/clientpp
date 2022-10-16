import type { ComponentChild } from "preact";
import { useState } from "preact/hooks";

interface SetProps {
  /**
   * This set's header.
   */
  title: string;
  /**
   * This set's body.
   */
  children?: ComponentChild;
}

/**
 *
 */
export function Set({ title, children }: SetProps) {
  const [collapsed, setCollapsed] = useState(false);

  return (
    <>
      <div className="setHed" onClick={() => setCollapsed(!collapsed)}>
        <span className="material-icons plusOrMinus">
          {collapsed ? "keyboard_arrow_right" : "keyboard_arrow_down"}
        </span>{" "}
        {title}
      </div>
      <div
        className="setBodH"
        style={{ display: collapsed ? "none" : undefined }}
      >
        {children}
      </div>
    </>
  );
}

interface HeadlessSetProps {
  /**
   * This set's body.
   */
  children: ComponentChild;
}

export function HeadlessSet({ children }: HeadlessSetProps) {
  return <div className="setBodH">{children}</div>;
}
