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
	SOCKET_ERROR = 11,
	// types
	SOCKET_MESSAGE_TEXT = 0,
	SOCKET_MESSAGE_BINARY = 1;

var { default: code } = require('./WSWorker.js?raw'),
	url = URL.createObjectURL(new Blob([ code ], { type: 'application/json' })),
	worker = new Worker(url),
	Events = require('./Libs/Events');

class WorkerIPC extends Events {
	constructor(worker){
		super();
		this.worker = worker;
		this.worker.addEventListener('message', ({ data }) => {
			this.emit(...data);
		});
		
		this.ready = new Promise(resolve => {
			// first message indicates ready
			worker.addEventListener('message', event => {
				event.stopPropagation();
				URL.revokeObjectURL(url);
				resolve();
			}, { once: true });
		});
	}
	send(event, ...data){
		this.worker.postMessage([ event, ...data ]);
		return true;
	}
	post(event, ...data){
		var id = Math.random();
			
		this.worker.postMessage([ event, id, ...data ]);
		
		return new Promise(resolve => this.once(id, data => resolve(data)));
	}
};


worker.ipc = new WorkerIPC(worker);

var text_decoder = new TextDecoder(),
	text_encoder = new TextEncoder();

window.WebSocket = class ThreadedWebSocket extends EventTarget {
	static CLOSED = 3;
	static CLOSING = 2;
	static CONNECTING = 0;
	static OPEN = 1;
	CLOSED = 3;
	CLOSING = 2;
	CONNECTING = 0;
	OPEN = 1;
	#wait = worker.ipc.ready
	#id
	#propv = {}
	#setpropv(event, value){
		if(typeof value == 'function'){
			this.addEventListener(event, value);
			this.#propv[event] = value;
		}else{
			this.removeEventListener(event, this.#propv[event]);
			this.#propv[event] = null;
		}
		
		return this.#propv[event];
	}
	get onopen(){ return this.#propv.open }
	set onopen(value){ return this.#setpropv('open', value) }
	get onclose(){ return this.#propv.close }
	set onclose(value){ return this.#setpropv('close', value) }
	get onerror(){ return this.#propv.error }
	set onerror(value){ return this.#setpropv('error', value) }
	get onmessage(){ return this.#propv.message }
	set onmessage(value){ return this.#setpropv('message', value) }
	#readyState = this.CLOSING
	#bufferedAmount = 0
	#binaryType = 'blob';
	#url
	get url(){
		return this.#url;
	}
	get readyState(){
		return this.#readyState;
	}
	get bufferedAmount(){
		return this.#bufferedAmount;
	}
	get binaryType(){
		return this.#binaryType;
	}
	set binaryType(value){
		this.#wait.then(() => worker.ipc.send(SOCKET_SET_BINARY_TYPE, this.#id, value));
		if(['blob','arrayBuffer'].includes(value))this.#binaryType = value;
		return value;
	}
	#event_listener
	async #create(url, protocol){
		await this.#wait;
		
		try{
			this.#url = new URL(url);
			if(!(['ws:','wss:'].includes(this.#url.protocol)))throw new DOMException(`Failed to construct 'WebSocket': The URL's scheme must be either 'ws' or 'wss'. '${this.#url.protocol.slice(0,-1)}' is not allowed.`);
		}catch(err){
			throw new DOMException(`Failed to construct 'WebSocket': The URL '${url}' is invalid.`);
		}
		
		var id = await worker.ipc.post(SOCKET_OPEN, url, protocol);
		
		this.#id = id;
		
		this.#event_listener = (type, ...data) => {
			switch(type){
				case SOCKET_CLOSED:
					this.dispatchEvent(new Event('close'));
					worker.ipc.off(this.#id, this.#event_listener);
					break;
				case SOCKET_ERROR:
					this.#readyState = data[0];
					this.dispatchEvent(new Event('error'));
					break;
				case SOCKET_OPENED:
					this.#readyState = data[0];
					this.dispatchEvent(new Event('open'));
					break;
				case SOCKET_MESSAGE:
					this.dispatchEvent(new MessageEvent('message', { data: data[0] }));
					break;
			}
		};
		
		worker.ipc.on(this.#id, this.#event_listener);
	}
	constructor(url, protocol){
		super();
		this.#wait = this.#create(url, protocol);
	}
	close(code, reason){
		this.#readyState = this.CLOSED;
		worker.ipc.send(SOCKET_CLOSE, code, reason);
	}
	send(data){
		// doesnt throw?
		if(this.#readyState != this.OPEN)return console.error('WebSocket is already in CLOSING or CLOSED state.');
		worker.ipc.send(SOCKET_SEND, this.#id, data);
	}
};