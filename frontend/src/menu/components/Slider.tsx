import Control from "./Control";
import type { BaseControlProps } from "./Control";
import type { JSX } from "preact";
import { useState } from "preact/hooks";

export interface SliderProps extends BaseControlProps {
  value?: number;
  defaultValue?: number;
  min?: number;
  max?: number;
  step?: number;
  onChange?: JSX.GenericEventHandler<HTMLInputElement>;
}

export default function Slider({
  title,
  attention,
  description,
  value,
  defaultValue,
  min,
  max,
  step,
  onChange,
}: SliderProps) {
  const [localValue, setLocalValue] = useState(value);

  return (
    <Control title={title} attention={attention} description={description}>
      <input
        type="number"
        className="sliderVal"
        min={min}
        max={max}
        step={step}
        value={localValue}
        defaultValue={
          typeof defaultValue === "number"
            ? defaultValue.toString()
            : defaultValue
        }
        onChange={(event) => {
          setLocalValue(event.currentTarget.valueAsNumber);
          if (onChange) onChange.call(undefined as never, event);
        }}
        style={{ marginRight: 0, borderWidth: 0 }}
      />
      <div className="slidecontainer" style={{ marginTop: -8 }}>
        <input
          className="sliderM"
          type="range"
          value={localValue}
          defaultValue={
            typeof defaultValue === "number"
              ? defaultValue.toString()
              : defaultValue
          }
          min={min}
          max={max}
          step={step}
          onChange={(event) => {
            setLocalValue(event.currentTarget.valueAsNumber);
            if (onChange) onChange.call(undefined as never, event);
          }}
        />
      </div>
    </Control>
  );
}
