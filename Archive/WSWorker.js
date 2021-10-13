'use strict';

var // client events
	SOCKET_OPEN = 0,
	SOCKET_SEND = 1,
	SOCKET_CLOSE = 2,
	SOCKET_SET_BINARY_TYPE = 3,
	SOCKET_GET_PROTOCOL = 4,
	SOCKET_GET_READYSTATE = 5,
	SOCKET_GET_URL = 6,
	SOCKET_GET_BUFFERED_AMOUNT = 7,
	// worker events
	SOCKET_MESSAGE = 8,
	SOCKET_OPENED = 9,
	SOCKET_CLOSED = 10,
	SOCKET_ERROR = 11;

// id -> ws
var sockets = {};

var total_ws = 0;

var send = (event, ...data) => postMessage([ event, ...data ]);

var verify_socket = (event, id) => {
	let socket = sockets[id];
	
	if(socket)return socket;
	else if(event != SOCKET_OPEN)throw RangeError(`Unknown socket ${id} on event ${event}`);
};

send();

var text_decoder = new TextDecoder();

addEventListener('message', ({ data: message }) => {
	var [ event, ...data ] = message;
	
	switch(event){
		case SOCKET_SET_BINARY_TYPE:
			
			verify_socket(event, data[0]).binaryType = data[1];
			
			break;
		case SOCKET_SEND:
			
			verify_socket(event, data[0]).send(data[1]);
			
			break;
		case SOCKET_OPEN:
			
			let id = total_ws++,
				socket = sockets[id] = new WebSocket(data[1], data[2]);
			
			send(data[0], id);
			
			socket.addEventListener('error', event => {
				send(id, SOCKET_ERROR, socket.readyState);
			});
			
			socket.addEventListener('open', event => {
				send(id, SOCKET_OPENED, socket.readyState);
			});
			
			socket.addEventListener('message', event => {
				send(id, SOCKET_MESSAGE, event.data);
			});
			
			break;
	}
});