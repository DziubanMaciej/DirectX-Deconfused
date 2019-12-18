function getParameters() {
    function isInArray() (
        for item in $1; do
            if [ "$item" == "$2" ]; then
                echo 1
                return
            fi
        done
        echo 0
    )

    function printHelp() (
        echo "Usage: build.sh [-d <Development|Production>] [-a <x64|x86>] [-c <Debug|Release>]"
    )

    distribution_modes=(Development Production)
    distribution_mode=${distribution_modes[0]}
    architectures=(x64 x86)
    architecture=${architectures[0]}
    configurations=(Debug Release)
    configuration=${configurations[0]}
    while getopts "a:d:c:h" opt; do
      case ${opt} in
        a ) architecture="$OPTARG"
            if [ `isInArray "$architectures" $OPTARG` == 0 ]; then
                printHelp
                exit 1
            fi ;;
        d ) distribution_mode="$OPTARG"
            if [ `isInArray "${distribution_modes[*]}" $OPTARG` == 0 ]; then
                printHelp
                exit 1
            fi ;;
        c ) configuration="$OPTARG"
            if [ `isInArray "${configurations[*]}" $OPTARG` == 0 ]; then
                printHelp
                exit 1
            fi ;;
        h ) printHelp; exit 0 ;;
        \?) printHelp; exit 1 ;;
      esac
    done
}

# Initialize
getParameters $@
root_dir=`pwd`
build_path=`realpath .build`

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

# Flags
external_libs_flags="-DDXD_BIN_PATH=${build_path}/DXD -DDXD_MACROS_PATH=${root_dir}/CMakeMacros.cmake"
dxd_flags="-DEXTERNAL_LIBS_BIN_PATH=${build_path} -DDISTRIBUTION_MODE=${distribution_mode}"
arch_flags="$architecture $configuration"

# Build targets
build_and_compile "ExternalLibraries/DirectXTex" "${build_path}/DirectXTex" $arch_flags $external_libs_flags
build_and_compile "ExternalLibraries/gtest"      "${build_path}/gtest"      $arch_flags $external_libs_flags "-Dgtest_force_shared_crt=ON"
build             "."                            "${build_path}/DXD"        $arch_flags $dxd_flags
