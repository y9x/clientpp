# Create a library called "Hello" which includes the source file "hello.cxx".
# The extension is already found. Any number of sources could be listed here.
# add_library (Hello hello.cxx)

# Make sure the compiler can find include files for our Hello library
# when other libraries or executables link to Hello

file(GLOB src
    "*.cpp"
    "../utils/*.cpp"
)

set(WEBVIEW2 ${PROJECT_SOURCE_DIR}/packages/Microsoft.Web.WebView2.1.0.992.28)
set(WIL ${PROJECT_SOURCE_DIR}/packages/Microsoft.Windows.ImplementationLibrary.1.0.211019.2)

add_executable(Client WIN32 ${src})
target_sources(Client PRIVATE Resource.rc)
target_include_directories(Client PRIVATE ${PROJECT_SOURCE_DIR}/include/ ${WEBVIEW2}/build/native/include/ ${WIL}/include)
target_link_libraries(Client crypt32 winhttp discord-rpc ${WEBVIEW2}/build/native/x86/WebView2LoaderStatic.lib)
target_compile_features(Client PUBLIC cxx_std_20)
set_target_properties(Client PROPERTIES CXX_EXTENSIONS OFF)