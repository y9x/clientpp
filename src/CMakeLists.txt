# Create a library called "Hello" which includes the source file "hello.cxx".
# The extension is already found. Any number of sources could be listed here.
# add_library (Hello hello.cxx)

# Make sure the compiler can find include files for our Hello library
# when other libraries or executables link to Hello

file(GLOB src
    "*.cpp"
    "../utils/*.cpp"
)

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
