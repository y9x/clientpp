import Control from "./Control";
import type { BaseControlProps } from "./Control";
import type { ChangeEvent } from "react";
import { useState } from "react";

export interface SliderControlProps extends BaseControlProps {
  value?: number;
  defaultValue?: number;
  min?: number;
  max?: number;
  step?: number;
  onChange?: (event: ChangeEvent<HTMLInputElement>) => void;
}

export default function SliderControl({
  title,
  value,
  defaultValue,
  min,
  max,
  step,
  onChange,
}: SliderControlProps) {
  const [localValue, setLocalValue] = useState(value);

  return (
    <Control title={title}>
      <input
        type="number"
        className="sliderVal"
        min={min}
        max={max}
        step={step}
        value={localValue}
        defaultValue={defaultValue}
        onChange={(event) => {
          setLocalValue(event.currentTarget.valueAsNumber);
          if (onChange) onChange(event);
        }}
        style={{ marginRight: 0, borderWidth: 0 }}
      />
      <div className="slidecontainer" style={{ marginTop: -8 }}>
        <input
          className="sliderM"
          type="range"
          value={localValue}
          defaultValue={defaultValue}
          min={min}
          max={max}
          step={step}
          onChange={(event) => {
            setLocalValue(event.currentTarget.valueAsNumber);
            if (onChange) onChange(event);
          }}
        />
      </div>
    </Control>
  );
}
