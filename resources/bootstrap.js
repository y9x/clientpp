console.log("Initializing Chief Client++...")

try {
  const run = new Function(
    "webview",
    "script",
    "_RUNTIME_DATA_",
    "eval(script)"
  );
  run(chrome.webview, $FRONTEND, $RUNTIME);
  console.log("Initialized Chief Client++");
} catch (err) {
  console.error("Unable to initialize Chief Client++:");
  console.error(err);
}
