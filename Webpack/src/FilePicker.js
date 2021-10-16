'use strict';

var Control = require('./libs/MenuUI/Control'),
	utils = require('./libs/Utils'),
	ipc = require('./IPC');

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
				click: () => {
					var id = Math.random().toString();
					
					ipc.once(id, (data, error) => {
						if(error)return;
						this.value = this.input.value = data;
					});
					
					// send entries instead of an object, c++ json parser removes the order
					ipc.send('browse file', id, this.data.title, Object.entries(this.data.filters));
				},
			},
		});
	}
};

module.exports = Control.Types.FilePicker = FilePicker;