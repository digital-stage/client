cmake_minimum_required(VERSION 3.12)

project(DIGITALSTAGE_APP VERSION 0.0.1)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake/modules)

if (APPLE)
        include(${CMAKE_CURRENT_LIST_DIR}/cmake/FixAppleArchitecture.cmake)
        set(CMAKE_OSX_DEPLOYMENT_TARGET 10.13)
endif (APPLE)

set(COMPILE_OV_WITH_CMAKE ON CACHE BOOL "Use cmake to configure and build OV")

add_compile_definitions(SIGNUP_URL="https://single.dstage.org/account/signup")
add_compile_definitions(STAGE_URL="https://single.dstage.org/stage")
add_compile_definitions(MIXER_URL="https://single.dstage.org/mixer")
add_compile_definitions(AUTH_URL="https://single.dstage.org/api/auth")
add_compile_definitions(API_URL="ws://localhost:4000")


add_subdirectory(lib/JUCE EXCLUDE_FROM_ALL)
add_subdirectory(lib/libds EXCLUDE_FROM_ALL)

juce_add_gui_app(DigitalStage
    BUNDLE_ID   org.digital-stage.client
    ICON_BIG    ${CMAKE_CURRENT_LIST_DIR}/assets/appicon.png
    NEEDS_CURL  TRUE
    
    COMPANY_NAME "Digital Stage gGmbH"
    PRODUCT_NAME "DigitalStage")

# `juce_generate_juce_header` will create a JuceHeader.h for a given target, which will be generated
# into your build tree. This should be included with `#include <JuceHeader.h>`. The include path for
# this header will be automatically added to the target. The main function of the JuceHeader is to
# include all your JUCE module headers; if you're happy to include module headers directly, you
# probably don't need to call this.

juce_generate_juce_header(DigitalStage)

# `target_sources` adds source files to a target. We pass the target that needs the sources as the
# first argument, then a visibility parameter for the sources which should normally be PRIVATE.
# Finally, we supply a list of source files that will be built into the target. This is a standard
# CMake command.

target_sources(DigitalStage
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ApplicationController.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/LoginPane.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/TaskbarComponent.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/jammer/JammerHandler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/SoundCardManager.cpp
        )

# `target_compile_definitions` adds some preprocessor definitions to our target. In a Projucer
# project, these might be passed in the 'Preprocessor Definitions' field. JUCE modules also make use
# of compile definitions to switch certain features on/off, so if there's a particular feature you
# need that's not on by default, check the module header for the correct flag to set here. These
# definitions will be visible both to your code, and also the JUCE module code, so for new
# definitions, pick unique names that are unlikely to collide! This is a standard CMake command.

target_compile_definitions(DigitalStage
    PRIVATE
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
        JUCE_USE_CURL=1     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:DigitalStage,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:DigitalStage,JUCE_VERSION>")

# If your target needs extra binary assets, you can add them here. The first argument is the name of
# a new static library target that will include all the binary resources. There is an optional
# `NAMESPACE` argument that can specify the namespace of the generated binary data class. Finally,
# the SOURCES argument should be followed by a list of source files that should be built into the
# static library. These source files can be of any kind (wav data, images, fonts, icons etc.).
# Conversion to binary-data will happen when your target is built.

# juce_add_binary_data(GuiAppData SOURCES ...)

# `target_link_libraries` links libraries and JUCE modules to other libraries or executables. Here,
# we're linking our executable target to the `juce::juce_gui_extra` module. Inter-module
# dependencies are resolved automatically, so `juce_core`, `juce_events` and so on will also be
# linked automatically. If we'd generated a binary data target above, we would need to link to it
# here too. This is a standard CMake command.

juce_add_bundle_resources_directory(DigitalStage ${CMAKE_CURRENT_LIST_DIR}/assets)


target_include_directories(DigitalStage
        PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/lib/libds/include
        #${CMAKE_CURRENT_LIST_DIR}/lib/libds/third_party/eventpp/include/eventpp
    )

target_link_libraries(DigitalStage
    PRIVATE
        # GuiAppData            # If we'd created a binary data target, we'd link to it here
        juce::juce_gui_extra
        juce::juce_audio_utils
        ds
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

if(UNIX)
    target_sources(DigitalStage
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ov/OvHandler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ov/OvMixer.cpp
    )
    
    ###
    # ZITA
    ###
    add_subdirectory(zita-njbridge ${PROJECT_BINARY_DIR}/zita EXCLUDE_FROM_ALL)
    # Copy zita binaries into the application bundle
    add_custom_command(TARGET DigitalStage
        POST_BUILD
        COMMAND cp $<TARGET_FILE_DIR:zita-n2j>/zita-n2j
        $<TARGET_FILE_DIR:DigitalStage>
        COMMENT "Copying zita-n2j" VERBATIM)
    add_custom_command(TARGET DigitalStage
        POST_BUILD
        COMMAND cp $<TARGET_FILE_DIR:zita-j2n>/zita-j2n
        $<TARGET_FILE_DIR:DigitalStage>
        COMMENT "Copying zita-j2n" VERBATIM)
        if(false)
            add_custom_command(TARGET DigitalStage POST_BUILD
                    COMMAND
                    ${CMAKE_INSTALL_NAME_TOOL}
                    -change
                    "/usr/local/lib/libjack.0.1.0.dylib"
                    "@executable_path/../Frameworks/libjack.0.1.0.dylib"
                    "$<TARGET_FILE_DIR:DigitalStage>/zita-n2j"
                    COMMENT "Fixing zita-n2j paths..."
                    )
            add_custom_command(TARGET DigitalStage POST_BUILD
                    COMMAND
                    ${CMAKE_INSTALL_NAME_TOOL}
                    -change
                    "/usr/local/lib/libjack.0.1.0.dylib"
                    "@executable_path/../Frameworks/libjack.0.1.0.dylib"
                    "$<TARGET_FILE_DIR:DigitalStage>/zita-j2n"
                    COMMENT "Fixing zita-j2n paths..."
                    )
        endif(false)

    ###
    # OV
    ###
    if(COMPILE_OV_WITH_CMAKE)
        message(STATUS "Using cmake to build OV")
        add_subdirectory(lib/libov EXCLUDE_FROM_ALL)
        get_property(TASCAR_PLUGIN_LIBRARIES GLOBAL PROPERTY TASCAR_PLUGIN_LIBRARIES)
        target_link_libraries(DigitalStage
        PRIVATE
            Ov::Ov
            ${TASCAR_PLUGIN_LIBRARIES}
            )
    else()
        message(STATUS "Using Makefile to build OV")
        # Add ov dependency only on unix like systems
        include(${CMAKE_CURRENT_LIST_DIR}/cmake/BuildLibOv.cmake)
        target_include_directories(DigitalStage 
        PUBLIC 
            ${OV_INCLUDE_DIRS})
        target_link_libraries(DigitalStage
        PUBLIC
            ${OV_LIBRARIES})
        add_dependencies(DigitalStage libov)

        # Special routine to fix dynlib imports inside macos
        if(APPLE)
            foreach (TASCAR_PLUGIN IN LISTS TASCAR_PLUGINS)
                string(REGEX MATCH "([^\/]+\.dylib)$" TASCAR_PLUGIN_NAME ${TASCAR_PLUGIN})
                message(STATUS "Will copy ${TASCAR_PLUGIN} into app bundle")
                if(false)
                    add_custom_command(TARGET DigitalStage POST_BUILD
                        COMMAND 
                        ${CMAKE_COMMAND} -E copy_if_different
                        ${TASCAR_PLUGIN}
                        "$<TARGET_FILE_DIR:DigitalStage>/../Frameworks/${TASCAR_PLUGIN_NAME}"
                        COMMAND
                        ${CMAKE_INSTALL_NAME_TOOL}
                        -id
                        "@executable_path/../Frameworks/${TASCAR_PLUGIN_NAME}"
                        "$<TARGET_FILE_DIR:DigitalStage>/../Frameworks/${TASCAR_PLUGIN_NAME}"
                        COMMAND
                        ${CMAKE_INSTALL_NAME_TOOL}
                        -change
                        "${TASCAR_PLUGIN}"
                        "@executable_path/../Frameworks/${TASCAR_PLUGIN_NAME}"
                        "$<TARGET_FILE:DigitalStage>"
                        COMMENT 
                        "Copying and fixing ${TASCAR_PLUGIN_NAME} into the application bundle"
                        VERBATIM
                    )
                endif(false)
            endforeach()
            add_custom_command(TARGET DigitalStage POST_BUILD
                    COMMAND
                    ${CMAKE_INSTALL_NAME_TOOL}
                    -add_rpath 
                    @executable_path/../Frameworks
                    "$<TARGET_FILE:DigitalStage>"
                    COMMENT 
                    "Adding rpath to target")
        endif(APPLE)
    endif()
endif(UNIX)

if(APPLE)
    cmake_policy(SET CMP0087 NEW)
    if(APPLE_CERT)
        message(STATUS "Signing app bundle is activated")
        install(CODE "
            INCLUDE(BundleUtilities)
            FIXUP_BUNDLE(\"$<TARGET_BUNDLE_DIR:DigitalStage>\" \"\" \"\")
            set(PLUGINS ${TASCAR_PLUGIN_LIBRARIES})
            foreach (PLUGIN IN LISTS PLUGINS)
                message(STATUS \"Fixing \${PLUGIN}\")
                execute_process(COMMAND ${CMAKE_INSTALL_NAME_TOOL} -id \${PLUGIN}.dylib \"$<TARGET_FILE_DIR:DigitalStage>/../Frameworks/\${PLUGIN}.dylib\")
            endforeach()
            execute_process(COMMAND codesign --force --deep --sign \"${APPLE_CERT}\" \"$<TARGET_FILE:DigitalStage>\")
        ")
    else(APPLE_CERT)
        install(CODE "
            INCLUDE(BundleUtilities)
            FIXUP_BUNDLE(\"$<TARGET_BUNDLE_DIR:DigitalStage>\" \"\" \"\")
            set(PLUGINS ${TASCAR_PLUGIN_LIBRARIES})
            foreach (PLUGIN IN LISTS PLUGINS)
                message(STATUS \"Fixing \${PLUGIN}\")
                execute_process(COMMAND ${CMAKE_INSTALL_NAME_TOOL} -id \${PLUGIN}.dylib \"$<TARGET_FILE_DIR:DigitalStage>/../Frameworks/\${PLUGIN}.dylib\")
            endforeach()
        ")
    endif(APPLE_CERT)
endif(APPLE)


set(CPACK_PACKAGE_HOMEPAGE_URL "https://www.digital-stage.org")
set(CPACK_PACKAGE_VENDOR "Digitale Bühne gGmbH")
set(CPACK_PACKAGE_CONTACT "Tobias Hegemann <tobias.hegemann@digital-stage.org")
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE)
set(CPACK_RESOURCE_FILE_README ${CMAKE_CURRENT_SOURCE_DIR}/README.md)

if(APPLE)
    INSTALL(TARGETS DigitalStage
        BUNDLE DESTINATION . COMPONENT Runtime
        RUNTIME DESTINATION bin COMPONENT Runtime
        RESOURCE DESTINATION assets COMPONENT Runtime
    )
    set(CPACK_DMG_VOLUME_NAME "InstallDigitalStage")
    set(CPACK_PACKAGE_NAME "InstallDigitalStage")
    set(CPACK_GENERATOR "DragNDrop")
    set(CPACK_DMG_DS_STORE_SETUP_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/Packaging_DMGSetup.scpt)
    set(CPACK_DMG_BACKGROUND_IMAGE ${CMAKE_CURRENT_SOURCE_DIR}/assets/DMGBackground.tif)
    set(CPACK_SOURCE_STRIP_FILES "")
    set(CPACK_STRIP_FILES "${CMAKE_CURRENT_BINARY_DIR}/DigitalStage_artefacts/${CMAKE_BUILD_TYPE}/DigitalStage.app")

    #set(CPACK_COMPONENTS_ALL DIGITALSTAGE_APP)
    include(CPack)
endif(APPLE)