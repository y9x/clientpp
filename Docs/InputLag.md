# Krunker input lag

Mouse feeling unresponsive in Krunker.

## Steps to reproduce

- Decrease monitor refresh rate to 60hz (NVIDIA control panel)
- Use a version of WebView2 running Chromium >86
- Open Krunker in a WebView2 Window
- Game will run at 60 ticks and report 60 FPS. Despite this, mouse movement feels unresponsive.

## Notes

- `WH_MOUSE_LL` hook is not responsible
- Doesn't apply to keyboard input
- Doesn't occur in Microsoft Edge or Google Chrome; Only in WebView2