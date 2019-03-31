# DirectX-Deconfused

## Building
1. Clone DXD
2. Download CMake https://cmake.org/download/
3. Install CMake and ensure it's in your PATH variable, i.e. you can write 'cmake' command in console
4. Run build.sh script
5. Go to build directory, double click the solution file

## Developing
1. Code formatting - ClangFormat, download extension for Visual Studio at https://marketplace.visualstudio.com/items?itemName=LLVMExtensions.ClangFormat You can currently opened file with Ctrl+R,D.
2. Use spaces

# Projects in soluction
    - ApplicationDX12 - standalone WIN32 application linking to libraries necessary for DirectX12
	- Documentation - outputs documentation upon build
    - LibraryDX12 - produces .dll and .lib files to link against
    - Application - WIN32 application linking to LibraryDX12. Building this does not automatically trigger LibraryDX12 build, so remember to do it manually.

# Documentation
Documentation is generated directly from the source code with Doxygen. You have to download it frome here: http://www.doxygen.nl/download.html and add to your PATH. Then build "Documentation" project in the solution to invoke generation. Files will be located in build/doc directory.
