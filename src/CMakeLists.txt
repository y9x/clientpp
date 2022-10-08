
file(GLOB src
    "*.cpp"
    "../utils/*.cpp"
)

add_definitions(-DCLIENT_VERSION_STRING="${CLIENT_VERSION_STRING}")
add_executable(Client WIN32 ${src})
target_sources(Client PRIVATE Resource.rc)
target_include_directories(Client PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${DISCORD_RPC}/include
    ${RAPIDJSON}/include
    ${URIPARSER}/include
    ${SEMVER}/include
    ${WEBVIEW2}/build/native/include
    ${WIL}/include
)
target_link_libraries(Client crypt32 winhttp discord-rpc uriparser ${WEBVIEW2}/build/native/x86/WebView2LoaderStatic.lib)
target_compile_features(Client PUBLIC cxx_std_20)
set_target_properties(Client PROPERTIES CXX_EXTENSIONS OFF)
