if(location.host == 'krunker.io' || location.host.endsWith('.krunker.io')){
	window.chrome.webview.postMessage('["send webpack"]');
	chrome.webview.addEventListener('message', ({ data }) => {
		if (data[0] == 'eval webpack') {
			let [event, webpack, runtime_data] = data;

			// try{
			new Function('_RUNTIME_DATA_', 'webpack', 'eval(webpack)')(runtime_data, webpack);
			console.log('Guru Client++ Webpack Initialized');
			// }catch(err){
			// 	console.error('Error loading Guru Client++ Bootstrapper:\n', err);
			//}
		} else console.error('Failure bootstrapping GC++: Unknown event:', event);
	}, { once: true });
}