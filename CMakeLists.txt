cmake_minimum_required(VERSION 3.16)
project (CLIENT VERSION 0.18 LANGUAGES CXX)

set(CLIENT_MAJOR_VERSION "0")
set(CLIENT_MINOR_VERSION "18")
set(CLIENT_PATCH_VERSION "0")
set(CLIENT_VERSION_STRING "${CLIENT_MAJOR_VERSION}.${CLIENT_MINOR_VERSION}.${CLIENT_PATCH_VERSION}-rc.1")

set(RAPIDJSON ${PROJECT_SOURCE_DIR}/rapidjson)
set(SEMVER ${PROJECT_SOURCE_DIR}/semver)
set(JSON ${PROJECT_SOURCE_DIR}/json)
set(WEBVIEW2 ${PROJECT_SOURCE_DIR}/packages/Microsoft.Web.WebView2.1.0.1343.22)
set(WIL ${PROJECT_SOURCE_DIR}/packages/Microsoft.Windows.ImplementationLibrary.1.0.220914.1)
set(DISCORD_RPC ${PROJECT_SOURCE_DIR}/discord-rpc)
set(URIPARSER ${PROJECT_SOURCE_DIR}/uriparser)

add_definitions(-DUNICODE -DNOMINMAX)

add_subdirectory (uriparser-lib)
add_subdirectory (discord-rpc-lib)
add_subdirectory (src)
