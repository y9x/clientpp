'use strict';

module.exports = code => {
	var ns = [];
	
	code.replace(/\/\/.*?$/gm, '').replace(/namespace\s+(\w+)\s*?{([^]*?)}/g, (match, namespace, data) => {
		var add = [];
		data.replace(/(\w+)\s*?=\s*?(\d+)/g, (match, label, value) => add.push(label + ':' + value));
		ns.push(`${namespace}:{${add}}`);
	});
	
	console.log(`module.exports={${ns}}`);
	
	return `module.exports={${ns}}`;
};