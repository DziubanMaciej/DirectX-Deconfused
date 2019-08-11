# ---------------------------------------------- Automatically adding all subdirectories with CMakeLists.txt
macro(add_subdirectories)
    file(GLOB subdirectories RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*)
    foreach(subdir ${subdirectories})
        file(RELATIVE_PATH subdir_relative ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/${subdir})
        if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${subdir}/CMakeLists.txt AND NOT ${subdir_relative}_hidden})
            add_subdirectory(${subdir})
        endif()
    endforeach()
endmacro()

# ---------------------------------------------- Accumulating sources in global properties and collecting them in one command

macro(add_sources TARGET_NAME)
    string(CONCAT PROPERTY_NAME "SourceFilesFor_" ${TARGET_NAME})
    set_property(GLOBAL APPEND PROPERTY ${PROPERTY_NAME} ${ARGN})
endmacro()

macro(add_sources_and_cmake_file TARGET_NAME)
    add_sources(${TARGET_NAME} ${ARGN} ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt)
endmacro()

macro(collect_sources VAR TARGET_NAME)
    string(CONCAT PROPERTY_NAME "SourceFilesFor_" ${TARGET_NAME})
    get_property(${VAR} GLOBAL PROPERTY ${PROPERTY_NAME})
endmacro()

# ---------------------------------------------- Helpers for precompiled shaders

function(add_shaders_and_cmake_file TARGET_NAME)
    add_sources(${TARGET_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt)

    string(CONCAT PATH_PROPERTY_NAME "ShaderPathsFor_" ${TARGET_NAME})
    string(CONCAT TYPE_PROPERTY_NAME "ShaderTypesFor_" ${TARGET_NAME})
    math(EXPR maxIndex "${ARGC} - 1")
    foreach(index RANGE 1 ${maxIndex} 2)
        set(path "${ARGV${index}}")
        math(EXPR index2 "${index} + 1")
        set(type "${ARGV${index2}}")

        add_sources(${TARGET_NAME} ${path})
        set_property(GLOBAL APPEND PROPERTY ${PATH_PROPERTY_NAME} ${path})
        set_property(GLOBAL APPEND PROPERTY ${TYPE_PROPERTY_NAME} ${type})
    endforeach()
endfunction()

macro(set_shaders_properties TARGET_NAME COMPILATION_ENABLED)
    string(CONCAT PATH_PROPERTY_NAME "ShaderPathsFor_" ${TARGET_NAME})
    string(CONCAT TYPE_PROPERTY_NAME "ShaderTypesFor_" ${TARGET_NAME})
    get_property(paths GLOBAL PROPERTY ${PATH_PROPERTY_NAME})
    get_property(types GLOBAL PROPERTY ${TYPE_PROPERTY_NAME})

    list(LENGTH paths length)
    math(EXPR length "${length} - 1")
    foreach(index RANGE ${length})
        list(GET paths ${index} path)
        list(GET types ${index} type)
        if(${COMPILATION_ENABLED})
            set_source_files_properties(${path} PROPERTIES VS_SHADER_TYPE ${type} VS_SHADER_MODEL 5.1)
        else()
            set_source_files_properties(${path} PROPERTIES HEADER_FILE_ONLY TRUE)
        endif()
    endforeach()
endmacro()

# ---------------------------------------------- Helpers for configuring paths

macro(set_output_directories)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib) # .lib
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin) # .dll
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin) # .exe
endmacro()

macro(set_working_directory_to_bin TARGET_NAME)
    set_property(TARGET ${TARGET_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE})
endmacro()

macro(set_link_directory_to_lib)
    link_directories(${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
endmacro()

macro(add_definitions_for_paths ${TARGET_NAME})
    add_definitions(-DSHADERS_PATH=L"../../../${TARGET_NAME}/Shaders/")
    add_definitions(-DRESOURCES_PATH=L"../../../")
endmacro()

set(DXD_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/LibraryDX12/Include)
