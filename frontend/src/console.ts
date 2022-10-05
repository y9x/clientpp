/**
 * Protect global console and allow logging before devtools can hook the context's console.
 */

type consoleProp = "log" | "info" | "warn" | "error" | "debug" | "trace";

const methods: consoleProp[] = [
  "log",
  "info",
  "warn",
  "error",
  "debug",
  "trace",
];

const initial = Object.fromEntries(
  methods.map((method) => [method, console[method]])
) as unknown as { [K in consoleProp]: typeof console[K] };

const buffer = Object.fromEntries(
  methods.map((method) => [method, []])
) as unknown as { [K in consoleProp]: unknown[][] };

const cloneConsole = Object.fromEntries(
  methods.map((method) => [
    method,
    (...data: unknown[]) => buffer[method].push(data),
  ])
) as unknown as { [K in consoleProp]: typeof console[K] };

// devtools hooks after
setTimeout(() => {
  for (const method of methods) {
    cloneConsole[method] = initial[method];
    for (const data of buffer[method]) cloneConsole[method](...data);
    buffer[method].length = 0;
  }
});

export default cloneConsole;
