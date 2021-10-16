'use strict';

module.exports = {
	'/': 'game',
	'/social.html': 'social',
	'/editor.html': 'editor',
}[location.pathname];