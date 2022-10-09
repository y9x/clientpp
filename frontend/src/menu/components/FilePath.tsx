import ipc, { IM } from "../../ipc";
import Control from "./Control";
import type { BaseControlProps } from "./Control";
import type { ReactElement } from "react";

export interface FilePathProps extends BaseControlProps {
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

/**
 * Control to get the location of a file, not the binary content.
 */
export default function FilePath({
  title,
  attention,
  description,
  children,
  value,
  defaultValue,
  onPick,
}: FilePathProps) {
  return (
    <Control title={title} attention={attention} description={description}>
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

          const file = await ipc.post<string>(IM.browse_file, title, filters);

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
