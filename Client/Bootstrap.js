if(location.host == 'krunker.io' || location.host.endsWith('.krunker.io')){
	let { log } = console,
		{ webview } = chrome;

	webview.postMessage("[0]");

	// first message should ALWAYS be evaluate data
	webview.addEventListener('message', ({ data }) => {
		var [ event, ...args ] = data;

		if (event != 1) throw Error('Invalid message recieved: ' + JSON.stringify(data));

		var [webpack, runtime_data] = args;
		new Function('webview', 'webpack', '_RUNTIME_DATA_', 'eval(webpack)')(webview, webpack, runtime_data);

		log('Chief Client++ Webpack Initialized');
	}, { once: true });
}