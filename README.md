# DirectX-Deconfused

## Building
1. Clone DXD
2. Download CMake https://cmake.org/download/
3. Install CMake and ensure it's in your PATH variable, i.e. you can write 'cmake' command in console
4. Run build.sh script
5. Go to build directory, double click the solution file

## Developing
1. Code formatting - ClangFormat, download extension for Visual Studio at https://marketplace.visualstudio.com/items?itemName=LLVMExtensions.ClangFormat You can currently opened file with Ctrl+R,D. Please try to format every file before committing.
2. Use spaces, not tabs
3. Do not include Microsoft headers (e.g. dxgi, d3d12, windows.h) directly, but rather use DXD/ExternalHeadersWrappers.
4. Inherit classes you create from NonCopyableOrMovable, unless copy/move semantics are really desired.
5. Put API classes in DXD namespaces. Leave internal classes in global namespace.
6. Do not include internal headers in API headers. That won't work on the library's client side
7. Use "camelCase" for variables, constants and methods and "CamelCase" for classes.

## Projects in solution
- CMakePredefinedTargets
    - ALL_BUILD - builds all projects upon build
    - ZERO_CHECK - reruns CMake scripts upon build
- Development
    - Application - WIN32 application linking to LibraryDX12. Building this does not automatically trigger LibraryDX12 build, so remember to do it manually.
    - LibraryDX12 - produces .dll and .lib files to link against
- Miscellaneous
    - Documentation - outputs documentation upon build
    - CopyResources - copies "Resources" directory into binary files location upon build. The resources are then inside the working directory during debugging in Visual Studio.

## Documentation
Documentation is generated directly from the source code with Doxygen. You have to download it frome here: http://www.doxygen.nl/download.html and add to your PATH. Then build "Documentation" project in the solution to invoke generation. Files will be located in build/doc directory.
