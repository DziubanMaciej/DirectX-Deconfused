# DirectX-Deconfused

## Building
Requirements:
- CMake
- Visual Studio 2017 along with Game Development with C++
- Doxygen (optional
- Bash interpreter (WSL or Git bash)

Steps:
1. Clone DXD
2. Ensure CMake is in your PATH
3. Run build.sh (optional parameter: Debug/Release)
4. Open .build/DXD/DXD.sln in Visual Studio
5. Build DXD_dll project
6. Generated binaries are under .build/DXD/bin/Debug

## Coding guidelines
1. Code formatting - ClangFormat, download extension for Visual Studio at https://marketplace.visualstudio.com/items?itemName=LLVMExtensions.ClangFormat Currently opened file  can be formatted with with Ctrl+R,D.
2. Use spaces, not tabs
3. Do not include Microsoft headers (e.g. dxgi, d3d12, windows.h) directly, but rather use DXD/ExternalHeadersWrappers.
4. Inherit classes you create from NonCopyableOrMovable, unless copy/move semantics are really desired.
5. Put API classes in DXD namespaces. Leave internal classes in global namespace.
6. API classes should only contain a protected constructor, pure virtual methods and factory method declaration implemented inside DXD
7. Do not include internal headers in API headers. That won't work on the library's client side
8. Use "camelCase" for variables, constants and methods and "CamelCase" for classes.

## Projects in solution
- Applications - example applications using DXD library.
    - Application1 - application providing camera controls and presenting all the effects implemented along with configuration controls displayed on screen
    - Application2 - minimal (Hello world) application rendering a cube
- CMakePredefinedTargets
    - ALL_BUILD - builds all projects upon build
    - ZERO_CHECK - reruns CMake scripts upon build
- Documentation - projects generating documentation upon build. You have to have Doxygen installed and added to your PATH for the projects to work
    - ApiDocumentation - generates documentation of DXD public headers
    - InternalDocumentation - generates documentation of all DXD files, both public and internal (note that not all internal code is documented)
- DXD
    - DXD_core - object library used by both DXD_dll and DXD_lib. Its purpose is to enable producing both dynamic and static version without compiling the whole engine twice
    - DXD_dll - dynamic DXD library, used by example application
    - DXD_lib - static DXD library, used by unit tests.
- Tests
    - UnitTests - unit level tests of some parts of the engine
    
## Linking your own applications to DXD
Preferred way is to link to a dynamic library.
1. Build DXD_dll project. 
2. Link your application to ./build/DXD/lib/DXD.lib
3. Copy DXD.dll to the same directory as your executable
It is also possible to link against DXD statically, but it's a bit more complex. 
1. Build DXD_lib project. 
2. Link your application to ./build/DXD/lib/DXD_static.lib.
3. Link your application to all of DXD's dependendies (see CMakeValues.cmake file)
4. #define DXD_STATIC_LINK in your application to enable static DXD linkage (to make it project-wide, you can set Project Properties -> C++ -> Preprocessor -> Preprocessor definitions).
