cmake_minimum_required(VERSION 3.16)
project (CLIENT VERSION 0.18 LANGUAGES CXX)

set(CLIENT_MAJOR_VERSION "0")
set(CLIENT_MINOR_VERSION "18")
set(CLIENT_PATCH_VERSION "0")
set(CLIENT_VERSION_STRING "${CLIENT_MAJOR_VERSION}.${CLIENT_MINOR_VERSION}.${CLIENT_PATCH_VERSION}")

set(RAPIDJSON ${PROJECT_SOURCE_DIR}/rapidjson)
set(SEMVER ${PROJECT_SOURCE_DIR}/semver)
set(JSON ${PROJECT_SOURCE_DIR}/json)
set(WEBVIEW2 ${PROJECT_SOURCE_DIR}/packages/Microsoft.Web.WebView2.1.0.992.28)
set(WIL ${PROJECT_SOURCE_DIR}/packages/Microsoft.Windows.ImplementationLibrary.1.0.211019.2)
set(DISCORD_RPC ${PROJECT_SOURCE_DIR}/discord-rpc)

add_definitions(-DUNICODE -DNOMINMAX -DCLIENT_VERSION_STRING="${CLIENT_VERSION_STRING}")

message("Setting CONTROL FLOW GUARD") 
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /guard:cf")
set(CMAKE_EXE_LINKER_FLAGS  "/guard:cf /DYNAMICBASE")

message("Setting QSPECTRE")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Qspectre")

add_subdirectory (src)
add_subdirectory (discord-rpc-lib)
