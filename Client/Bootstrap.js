if(location.host == 'krunker.io' || location.host.endsWith('.krunker.io')){
	let { log } = console,
		{ webview } = chrome;

	webview.postMessage('["send webpack"]');
	// first message should ALWAYS be evaluate data
	webview.addEventListener('message', ({ data }) => {
		var [event] = data.slice(0, 1);

		if (event != 'eval webpack') throw Error('Invalid message recieved: ' + JSON.stringify(data));

		var [, webpack, runtime_data] = data;
		new Function('webview', 'webpack', '_RUNTIME_DATA_', 'eval(webpack)')(webview, webpack, runtime_data);

		log('Guru Client++ Webpack Initialized');
	}, { once: true });
}