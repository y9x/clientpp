import Control from "./Control";
import type { BaseControlProps } from "./Control";
import ipc, { IM } from "../../ipc";
import type { ReactElement } from "react";

export interface FileControlProps extends BaseControlProps {
  onPick?: (file: string) => void;
  children: ReactElement<
    Omit<Omit<JSX.IntrinsicElements["option"], "children">, "value"> & {
      value?: string;
      children?: string;
    }
  >[];
  value?: string;
  defaultValue?: string;
}

export default function FileControl({
  title,
  children,
  value,
  defaultValue,
  onPick,
}: FileControlProps) {
  return (
    <Control title={title}>
      <div
        className="settingsBtn"
        style={{ width: 100 }}
        onClick={async () => {
          const filters: [name: string, pattern: string][] = [];

          for (const option of children) {
            filters.push([
              option.props.children || option.props.value || "",
              option.props.value || option.props.children || "",
            ]);
          }

          const file = (await ipc.post(
            IM.browse_file,
            title,
            filters
          )) as string;

          if (onPick) onPick(file);
        }}
      >
        Browse
      </div>
      <input
        type="text"
        name="text"
        className="inputGrey2"
        placeholder="Custom Title"
        value={value}
        defaultValue={defaultValue}
        onInput={(event) => {
          if (onPick) onPick(event.currentTarget.value);
        }}
      />
    </Control>
  );
}
