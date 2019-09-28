root_dir=`pwd`
build_path=`realpath .build`

function build() {
    # Get args
    src_dir="$1"
    build_dir="$2"
    shift; shift # rest of arguments go to CMake

    # Go to destination directory
    mkdir -p "$build_dir"
    pushd "$build_dir" > /dev/null

    # Run CMake
    src="$root_dir/$src_dir"
    echo "Building \"$src\"..."
    cmake "$src" $@
    if [ $? != 0 ]; then
        exit 1
    fi
    echo

    # Return to previous directory
    popd > /dev/null
}

function build_and_compile() {
    # Run CMake
    build $@

    # Compile
    build_dir="$2"
    cmake --build "$build_dir"
    if [ $? != 0 ]; then
        exit 1
    fi
    echo
}

assimp_flags="-DASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT=OFF -DASSIMP_BUILD_OBJ_IMPORTER=ON"
arch_flags="-A x64 -DCMAKE_BUILD_TYPE=Debug"
external_libs_flags="-DDXD_BIN_PATH=${build_path}/DXD -DDXD_MACROS_PATH=${root_dir}/CMakeMacros.cmake"
dxd_flags="-DEXTERNAL_LIBS_BIN_PATH=${build_path}"

build_and_compile "ExternalLibraries/DirectXTex" "${build_path}/DirectXTex" ${arch_flags} ${external_libs_flags}
build_and_compile "ExternalLibraries/assimp"     "${build_path}/assimp"     ${arch_flags} ${external_libs_flags} ${assimp_flags}
build             "."                            "${build_path}/DXD"        ${arch_flags} ${dxd_flags}
