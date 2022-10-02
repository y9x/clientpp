{
  let { log, error } = console;

  try {
    let run = new Function(
      "webview",
      "script",
      "_RUNTIME_DATA_",
      "eval(script)"
    );
    run(chrome.webview, $FRONTEND, $RUNTIME);
    log("Initialized Chief Client++");
  } catch (err) {
    error("Unable to initialize Chief Client++:\n", err);
  }
}
