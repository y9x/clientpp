if(location.host == 'krunker.io' || location.host.endsWith('.krunker.io')){
	window.chrome.webview.postMessage('["send webpack"]');
	chrome.webview.addEventListener('message', event => {
		var [event, ...data] = JSON.parse(event.data);

		if (event == 'eval webpack') {
			let [ webpack, runtime_data ] = data;

			// let map = URL.createObjectURL(new Blob([webpack_map], { type: 'application/json' }));

			try{
				new Function('_RUNTIME_DATA_', 'webpack', 'eval(webpack)')(runtime_data, webpack);
				console.log('Guru Client++ Webpack Initialized');
			 }catch(err){
				console.error('Error loading Guru Client++ Bootstrapper:\n', err);
			}
		} else console.error('Loading Webpack failure: Unknown event:', event);
	});
}