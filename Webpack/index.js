'use strict';

var production = false;

var path = require('path'),
	webpack = require('webpack'),
	TerserPlugin = require('terser-webpack-plugin'),
	dist = path.join(__dirname, 'dist'),
	serve = path.join(dist, 'serve'),
	{ ModifyPlugin, errors } = require('./ModifyPlugin'),
	folder = path.join(__dirname, 'src'),
	callback = (err, stats) => {
		if(errors(err, stats))return console.error('Build of bootstrapper failed');
		else console.log('Build of bootstrapper success');
	},
	compiler = webpack({
		entry: folder,
		output: {
			path: dist,
			filename: 'Webpack.js',
		},
		context: __dirname,
		// inline work can be done from the client
		// base64 strings are 3x larger
		devtool: 'source-map',
		mode: production ? 'production' : 'development',
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
		module: {
			rules: [
				{ test: /IPCMessages\.h$/, use: path.join(__dirname, 'EnumLoader.js') },
			],
		},
	});

if(process.argv.includes('-once'))compiler.run(callback);
else compiler.watch({}, callback);