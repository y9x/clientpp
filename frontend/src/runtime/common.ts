/*
 * UserScript sourcemap utils
 */

const u = () => "URL";

// template to bypass webpack source-map-loader errors
const sourceMappingURL = (data: string) =>
  `sourceMapping${u()}=data:application/json,${encodeURI(data)}`;

/**
 * Produce a snippet that will map to the specified name.
 */
export const nameCode = (script: string, code: string, useStrict = false) => {
  const mappings: string[] = [];

  const lines = code.split("\n");

  if (useStrict) mappings.push("");

  for (let i = 0; i < lines.length; i++) {
    if (i === 0) mappings.push("AAAA");
    else mappings.push("AACA");
  }

  const map = {
    version: 3,
    file: null,
    sources: [new URL("file:" + script).toString()],
    sourcesContent: [null],
    names: [],
    mappings: mappings.join(";"),
  };

  return (
    (useStrict ? '"use strict";\n' : "") +
    code +
    "//# " +
    sourceMappingURL(JSON.stringify(map))
  );
};

/**
 * Produce a mapped snippet that will evaluate to a callable block function.
 */
export const nameFunctionCode = (script: string, code: string) => {
  const mappings: string[] = [];

  const lines = code.split("\n");

  // first line is empty because we append ()=>{\n
  mappings.push("");

  for (let i = 0; i < lines.length; i++) {
    if (i === 0) mappings.push("AAAA");
    else mappings.push("AACA");
  }

  const map = {
    version: 3,
    file: null,
    sources: [new URL("file:" + script).toString()],
    sourcesContent: [null],
    names: [],
    mappings: mappings.join(";"),
  };

  return `()=>{\n${code}\n}//# ${sourceMappingURL(JSON.stringify(map))}`;
};
