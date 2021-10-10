# <img src="./Client/Icon.ico" style="height:1em"> Guru Client++

Modern Krunker Client written in C++ and Powered by WebView2

DISCLAIMER: This client does not contain cheats unless installed by the user.

[License](./LICENSE)

[Discord](https://y9x.github.io/discord)

## Features:

- Higher FPS potential: No NodeJS and Electron overhead unlike IDKR and Official Client
- Lightweight
- Fast Loading
- Resource Swapper
- User Styles
- User Scripts
- Blocks Ads

## Installation:

1. [Download the Latest Release](https://github.com/y9x/clientpp/releases)
2. Run Client.exe

## Requirements:

- Windows 7-10
- [Visual C++ Runtime](https://docs.microsoft.com/en-US/cpp/windows/latest-supported-vc-redist?view=msvc-160#visual-studio-2015-2017-2019-and-2022)

## Why you shouldn't uncap FPS

- Aim freeze no longer occurs
- Bhopping and movement is no longer dependent on FPS
- Less resource usage

## Finding the Client Folder

1. Open Krunker Settings
2. Select the Client tab
3. Find the Folder label, select Open

## Installing a User Script

1. Open Krunker Settings
2. Select the Client tab
3. Find the Scripts label, select Open
4. Drag your User Script into the folder
5. Press <kbd>F4</kbd> to seek a new game or <kbd>F5</kbd> to refresh

## Installing a User Style

1. Open Krunker Settings
2. Select the Client tab
3. Find the Styles label, select Open
4. Drag your User Style into the folder
5. Press <kbd>F4</kbd> to seek a new game or <kbd>F5</kbd> to refresh

## Installing the Krunker Cheat Loader

DISCLAIMER: This client does not explicitly condone the use of cheats.

1. Download the [Krunker Cheat Loader](https://api.sys32.dev/v2/cheat-loader)
2. Follow the [Userscript installation steps](#installing-a-user-script)

## Build Requirements

- [Visual Studio](https://visualstudio.microsoft.com/downloads/)
- Desktop Development with C++
- C++ ATL for latest v142 build tools
- NodeJS

## Preparing repository

- Clone the repo
- Enter `libs/Release/`
- Enter `libcrypto-1_1-static.zip`
- Extract `libcrypto-1_1-static.lib` to `libs/Release/`

This is because libcrypto is >100 mb, if it were left unextracted then the repo can't be committed 

**Remove conflicting OpenSSL and zlib packages**

```sh
vcpkg remove openssl:x86-windows --recurse
vcpkg remove zlib:x86-windows --recurse
```

## Build Webpack

1. Extract and open the Client repo
2. CD into the Webpack folder
3. Install modules: `npm install`
4. Build: `npm run build-once`

## Build VS

1. Open `Guru Client++.sln`
2. Press Build in the toolbar
3. Select Build Solution
4. Find output in `Client/Debug` or `Client/Release`