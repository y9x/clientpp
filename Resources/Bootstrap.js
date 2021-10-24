{
	let { log, error } = console;
	
	try {
		let run = new Function('webview', 'webpack', '_RUNTIME_DATA_', 'eval(webpack)');
		run(chrome.webview, $WEBPACK, $RUNTIME);
		log('Initialized Chief Client++');
	} catch (err) {
		error('Unable to initialize Chief Client++:\n', err);
	}
}