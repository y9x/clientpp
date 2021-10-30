'use strict';

var Control = require('./libs/MenuUI/Control'),
	utils = require('./libs/Utils'),
	{ IM, ipc } = require('./IPC');

class FilePicker extends Control.Types.TextBoxControl {
	static id = 'filepicker';
	create(...args){
		super.create(...args);
		this.browse = utils.add_ele('div', this.content, {
			className: 'settingsBtn',
			textContent: 'Browse',
			style: {
				width: '100px',
			},
			events: {
				click: async () => {
					// send entries instead of an object, c++ json parser removes the order
					var data = await ipc.post(IM.browse_file, this.data.title, Object.entries(this.data.filters));
					
					this.value = this.input.value = data;
				},
			},
		});
	}
};

module.exports = Control.Types.FilePicker = FilePicker;