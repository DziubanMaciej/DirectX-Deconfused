# DirectX-Deconfused

## How to build and develop
### Building
1. Clone DXD
2. Download CMake https://cmake.org/download/
3. Install CMake and ensure it's in your PATH variable, i.e. you can write 'cmake' command in console
4. Run build.sh script
5. Go to build directory, double click the solution file
### Developing
1. Code formatting - ClangFormat, download extension for Visual Studio at https://marketplace.visualstudio.com/items?itemName=LLVMExtensions.ClangFormat You can currently opened file with Ctrl+R,D.
2. Use spaces
3. There are three projects in the solution
    - ApplicationDX12 - standalone WIN32 application linking to libraries necessary for DirectX12
    - LibraryDX12 - produces .dll and .lib files to link against
    - Application - WIN32 application linking to LibraryDX12. Building this does not automatically trigger LibraryDX12 build, so remember to do it manually.
