list(APPEND TASCAR_PLUGINS 
        ${CMAKE_CURRENT_SOURCE_DIR}/libov/tascar/plugins/build/ovclienttascar_ap_delay.dylib    
        ${CMAKE_CURRENT_SOURCE_DIR}/libov/tascar/plugins/build/ovclienttascar_ap_metronome.dylib
        ${CMAKE_CURRENT_SOURCE_DIR}/libov/tascar/plugins/build/ovclienttascar_ap_sndfile.dylib
        ${CMAKE_CURRENT_SOURCE_DIR}/libov/tascar/plugins/build/ovclienttascar_jackrec.dylib
        ${CMAKE_CURRENT_SOURCE_DIR}/libov/tascar/plugins/build/ovclienttascar_route.dylib
        ${CMAKE_CURRENT_SOURCE_DIR}/libov/tascar/plugins/build/ovclienttascar_system.dylib
        ${CMAKE_CURRENT_SOURCE_DIR}/libov/tascar/plugins/build/ovclienttascar_touchosc.dylib
        ${CMAKE_CURRENT_SOURCE_DIR}/libov/tascar/plugins/build/ovclienttascar_waitforjackport.dylib
        ${CMAKE_CURRENT_SOURCE_DIR}/libov/tascar/plugins/build/ovclienttascarreceiver_hrtf.dylib
        ${CMAKE_CURRENT_SOURCE_DIR}/libov/tascar/plugins/build/ovclienttascarreceiver_omni.dylib
        ${CMAKE_CURRENT_SOURCE_DIR}/libov/tascar/plugins/build/ovclienttascarreceiver_ortf.dylib
        ${CMAKE_CURRENT_SOURCE_DIR}/libov/tascar/plugins/build/ovclienttascarreceiver_simplefdn.dylib
        ${CMAKE_CURRENT_SOURCE_DIR}/libov/tascar/plugins/build/ovclienttascarsource_cardioidmod.dylib
        ${CMAKE_CURRENT_SOURCE_DIR}/libov/tascar/plugins/build/ovclienttascarsource_omni.dylib)

message(STATUS "Will compile OV for ${CMAKE_SYSTEM_PROCESSOR}")

add_custom_command(
    OUTPUT 
        ${CMAKE_CURRENT_SOURCE_DIR}/libov/build/libov.a
        ${TASCAR_PLUGINS}
    COMMAND make VERBOSE=1 ARCH=${CMAKE_SYSTEM_PROCESSOR} -j${CMAKE_BUILD_PARALLEL_LEVEL}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/libov
    COMMENT "Building OV"
    VERBATIM
)
add_custom_target(libov
    DEPENDS 
        ${CMAKE_CURRENT_SOURCE_DIR}/libov/build/libov.a
        ${TASCAR_PLUGINS}
    VERBATIM
    )

if(APPLE)
    foreach (TASCAR_PLUGIN IN LISTS TASCAR_PLUGINS)
        string(REGEX MATCH "([^\/]+\.dylib)$" TASCAR_PLUGIN_NAME ${TASCAR_PLUGIN})
        add_custom_command(TARGET libov POST_BUILD
            COMMAND
            ${CMAKE_INSTALL_NAME_TOOL}
            -id
            "${TASCAR_PLUGIN}"
            ${TASCAR_PLUGIN}
            COMMENT "Fixing ${TASCAR_PLUGIN_NAME}..."
            DEPENDS
            ${TASCAR_PLUGIN}
        )
    endforeach()
endif(APPLE)

set(OV_INCLUDE_DIRS 
    ${CMAKE_CURRENT_SOURCE_DIR}/libov/src
    ${CMAKE_CURRENT_SOURCE_DIR}/libov/tascar/libtascar/build)
set(OV_LIBRARIES 
    ${CMAKE_CURRENT_SOURCE_DIR}/libov/build/libov.a
    ${TASCAR_PLUGINS}
    )