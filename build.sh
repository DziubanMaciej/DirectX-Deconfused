root_dir=`pwd`
build_path=`realpath .build`
architecture="x64"
build_type="Debug"

function build() {
    # Get args
    src_dir="$1"
    build_dir="$2"
    architecture="$3"
    build_type="$4"
    shift; shift; shift; shift # rest of arguments go to CMake

    # Go to destination directory
    mkdir -p "$build_dir"
    pushd "$build_dir" > /dev/null

    # Run CMake
    src="$root_dir/$src_dir"
    echo "Building \"$src\"..."
    cmake "$src" -A $architecture -DCMAKE_BUILD_TYPE=$build_type $@
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
    build_type="$4"
    cmake --build "$build_dir" --config "$build_type"
    if [ $? != 0 ]; then
        exit 1
    fi
    echo
}

external_libs_flags="-DDXD_BIN_PATH=${build_path}/DXD -DDXD_MACROS_PATH=${root_dir}/CMakeMacros.cmake"
dxd_flags="-DEXTERNAL_LIBS_BIN_PATH=${build_path}"

build_and_compile "ExternalLibraries/DirectXTex" "${build_path}/DirectXTex" ${architecture} $build_type ${external_libs_flags}
build             "."                            "${build_path}/DXD"        ${architecture} $build_type ${dxd_flags}
