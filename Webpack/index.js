'use strict';

var production = false;

var path = require('path'),
	webpack = require('webpack'),
	TerserPlugin = require('terser-webpack-plugin'),
	dist = path.join(__dirname, 'dist'),
	serve = path.join(dist, 'serve'),
	{ ModifyPlugin, errors } = require('./ModifyPlugin'),
	terser = {
		optimization: {
			minimize: production,
			minimizer: [ new TerserPlugin({
				terserOptions: {
					mangle: {
						eval: true, 
					},
					format: {
						quote_style: 1,
					},
				},
			}) ],
		},
	},
	folder = path.join(__dirname, 'src');

var compiler = webpack({
	entry: folder,
	output: {
		path: dist,
		filename: 'Webpack.js',
	},
	context: __dirname,
	devtool: 'source-map',
	plugins: [
			new ModifyPlugin({
				file: 'Webpack.js',
				stage: 'result',
				replace: [
					[ '//# sourceMappingURL=Webpack.js.map', '' ],
				],
			}),
		],
	// false
	mode: production ? 'production' : 'development',
	...terser,
}, (err, stats) => {
	if(errors(err, stats))return console.error('Creating compiler bootstrapper failed');
	
	var callback = (err, stats) => {
		if(errors(err, stats))return console.error('Build of bootstrapper failed');
		else console.log('Build of bootstrapper success');
	};
	
	if(process.argv.includes('-once'))compiler.run(callback);
	else compiler.watch({}, callback);
});