{
	let { log, error } = console,
		run = new Function('webview', 'webpack', '_RUNTIME_DATA_', 'eval(webpack)');

	try {
		run(chrome.webview, $WEBPACK, $RUNTIME);
		log('Chief Client++ Webpack Initialized');
	} catch (err) {
		error('Unable to initialize Chief Client++ Webpack:');
		error(err);
	}
}