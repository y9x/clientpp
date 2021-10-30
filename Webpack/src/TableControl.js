'use strict';

var Control = require('./libs/MenuUI/Control'),
	utils = require('./libs/Utils');

class TableControl extends Control {
	static id = 'table';
	create(){
		this.node = utils.add_ele('table', this.content, {
			style: {
				display: 'block',
			},
		});
	}
	update(init){
		super.update(init);
		if(init)this.input.checked = this.value;
		this.label_text(this.name);
	}
};

module.exports = Control.Types.TableControl = TableControl;