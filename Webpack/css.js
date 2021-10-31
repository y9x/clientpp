'use strict';

var fs = require('fs'),
	path = require('path');

module.exports = source => {
	return 'module.exports=' + JSON.stringify(source + '');
};