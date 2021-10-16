'use strict';

module.exports = code => {
	var enums = {};
	
	code.replace(/enum class (\w+) {([^]*?)}/g, (match, label, data) => {
		var add = enums[label] = [];
		
		console.log(data);
		data.replace(/\w+/g, match => add.push(match + ':' + add.length));
	});
	
	var output = [];
	
	for(let [ label, data ] of Object.entries(enums))output.push(`exports.${label}={${data.join(',')}}`);
	
	return output.join(';');
};