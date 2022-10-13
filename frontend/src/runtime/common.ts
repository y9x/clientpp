import type { SourceMap } from "magic-string";

const u = () => "URL";

// template to bypass webpack source-map-loader errors
export const sourceMappingURL = (data: SourceMap) =>
  `sourceMapping${u()}=data:application/json,${encodeURI(
    JSON.stringify(data)
  )}`;
