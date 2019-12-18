if(DISTRIBUTION_MODE STREQUAL "Production")
    return()
endif()

# Compile options
add_definitions(/MP)
include_directories(. ${DXD_INCLUDE_DIR})
set_output_directories()
set_link_directory_to_lib()

# Macro for adding a single app
macro(add_application TARGET_NAME DEFAULT_APP)
    # Get Sources
    set(common_sources "Common/FpsCounter.h")
    set(SOURCES ${ARGN})
    list(APPEND SOURCES ${common_sources})

    # Target definition
    add_executable (${TARGET_NAME} WIN32 ${SOURCES})
    set_working_directory_to_bin(${TARGET_NAME})
    target_link_libraries(${TARGET_NAME} ${DXD_DLL_NAME}.lib)
    add_dependencies(${TARGET_NAME} ${DXD_TARGET_DLL})
    source_group (TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

    # Visual studio settings
    set_target_properties(${TARGET_NAME} PROPERTIES FOLDER Applications)
    if (${DEFAULT_APP})
        set_directory_properties(PROPERTIES VS_STARTUP_PROJECT ${TARGET_NAME})
    endif()
endmacro()

# Apps creations
add_application(Application1 TRUE  "Application1.cpp")
add_application(Application2 FALSE "Application2.cpp")
