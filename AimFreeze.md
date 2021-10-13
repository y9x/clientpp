# WebSocket lag with high frame rate
AKA "Krunker Aim Freeze"

On Chromium versions after 86.0.4231.0 with high frame rate, WebSocket responses are delayed when the pointer is locked, left mouse button is held, and the cursor is being moved.

## Steps to reproduce

1. Launch Chromium > 86 with the `--disable-frame-rate-limit` flag
2. Visit https://krunker.io
3. Verify you have over 244 FPS. Open the game's settings and search for "Show FPS"
3. In DevTools, open the network tab then search for the websocket `/ws?gameId=`. Select this then open the messages tab
5. In the game, press "Click to play" to trigger pointer lock and move your mouse cursor while holding the left mouse button
6. Notice the unusual lack of responses compared to when the left mouse button wasn't held

## Notes

- ~~This is NOT an issue with the game servers; Modifying all variables relevant to reporting the frames (delta time) will result in the same behavior~~
After attempting to recreate this, I've concluded this is an issue unique to Krunker's frame and event handling
- WebSocket messages aren't skipped
- Only occurs when moving mouse on pointer locked elements:
```js
window.addEventListener('click', event => event.target.requestPointerLock());
```
- JS handlers for `mousemove` aren't involved:
```js
// Run before game loads:
var { getContext } = HTMLCanvasElement.prototype,
	calls = 0;

HTMLCanvasElement.prototype.getContext = function(type, options){
	var ctx = getContext.call(this, type, options);
    this.addEventListener('mousemove', event => {
        event.stopImmediatePropagation(); // RESPONSES STILL LAGGING
    });
    return ctx;
};
```

## Possible causes

- Chromium thread prioritizes emitting `mousemove` events over WebSocket messages
(Noticable when both events are frequently emitted)

- Krunker servers are overwhelmed by the possibility of high camera changes per frame