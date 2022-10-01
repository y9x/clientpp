// script executes before devtools listens on logs

const methods = ["log", "info", "warn", "error", "debug", "trace"];
let initial = {};
let buffer = {};

const cloneConsole = {};

for (let method of methods) {
  initial[method] = console[method].bind(console);
  cloneConsole[method] = (...data) => {
    if (!buffer[method]) buffer[method] = [];
    buffer[method].push(data);
  };
}

// devtools hooks after

setTimeout(() => {
  for (let method of methods) {
    cloneConsole[method] = initial[method];
    if (buffer[method])
      for (let data of buffer[method]) cloneConsole[method](...data);
  }

  buffer = null;
  initial = null;
});

export default cloneConsole;
