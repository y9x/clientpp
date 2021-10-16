'use strict';

module.exports = code => {
	var enums = {};
	
	code.replace(/\/\/.*?$/gm, '').replace(/enum class (\w+) {([^]*?)}/g, (match, label, data) => {
		// console.log(data);
		var add = enums[label] = [];
		
		data.replace(/\w+/g, match => add.push(match + ':' + add.length));
	});
	
	var output = [];
	
	for(let [ label, data ] of Object.entries(enums))output.push(`exports.${label}={${data.join(',')}}`);
	
	return output.join(';');
};