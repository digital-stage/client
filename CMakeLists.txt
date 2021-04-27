cmake_minimum_required(VERSION 3.12)

project(DIGITALSTAGE_ORLANDOVIOLS_APP VERSION 0.0.1)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
cmake_policy(SET CMP0087 NEW)
if (UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif ()
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake/modules)

# HOTFIXES
include(${CMAKE_CURRENT_LIST_DIR}/cmake/FixLinuxCpprest.cmake)
if (APPLE)
    include(${CMAKE_CURRENT_LIST_DIR}/cmake/FixAppleArchitecture.cmake)
    set(CMAKE_OSX_DEPLOYMENT_TARGET 10.13)
endif (APPLE)

set(COMPILE_OV_WITH_CMAKE ON CACHE BOOL "Use cmake to configure and build OV")

# GLOBAL DEFINTIONS
add_compile_definitions(SIGNUP_URL="https://single.dstage.org/account/signup")
add_compile_definitions(STAGE_URL="https://single.dstage.org/stage")
add_compile_definitions(MIXER_URL="https://single.dstage.org/mixer")
add_compile_definitions(AUTH_URL="https://single.dstage.org/api/auth")
add_compile_definitions(API_URL="ws://localhost:4000")

# DEPENDENCIES
add_subdirectory(lib/JUCE EXCLUDE_FROM_ALL)
add_subdirectory(lib/libds EXCLUDE_FROM_ALL)
if (UNIX)
    find_package(Jack REQUIRED)
endif (UNIX)

# ACTUAL APPS
juce_add_gui_app(DigitalStage
        BUNDLE_ID org.digital-stage.client
        ICON_BIG ${CMAKE_CURRENT_LIST_DIR}/assets/digitalstage/appicon.png
        NEEDS_CURL TRUE

        COMPANY_NAME "Digital Stage gGmbH"
        PRODUCT_NAME "DigitalStage")
juce_generate_juce_header(DigitalStage)
target_sources(DigitalStage
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/digitalstage/Main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/digitalstage/ApplicationController.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/digitalstage/LoginPane.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/digitalstage/TaskbarComponent.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/digitalstage/jammer/JammerHandler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/digitalstage/utils/SoundCardManager.cpp
        )
target_compile_definitions(DigitalStage
        PRIVATE
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
        JUCE_USE_CURL=1     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:DigitalStage,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:DigitalStage,JUCE_VERSION>")
juce_add_bundle_resources_directory(DigitalStage ${CMAKE_CURRENT_LIST_DIR}/assets)
target_include_directories(DigitalStage
        PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/lib/libds/include
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

if (UNIX)
    target_sources(DigitalStage
            PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src/digitalstage/ov/OvHandler.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/common/OvMixer.cpp
            )
    target_link_libraries(DigitalStage
            PUBLIC
            ${JACK_LIBRARIES})
    target_include_directories(DigitalStage
            SYSTEM PUBLIC
            ${JACK_INCLUDE_DIRS})

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

    ###
    # OV
    ###
    message(STATUS "Using cmake to build OV")
    add_subdirectory(lib/libov EXCLUDE_FROM_ALL)
    target_compile_options(Ov PRIVATE -Wno-all)
    get_property(TASCAR_PLUGIN_LIBRARIES GLOBAL PROPERTY TASCAR_PLUGIN_LIBRARIES)
    target_include_directories(DigitalStage
            SYSTEM PRIVATE
            ${CMAKE_CURRENT_LIST_DIR}/lib/libov/src
            )
    target_link_libraries(DigitalStage
            PRIVATE
            Ov::Ov
            ${TASCAR_PLUGIN_LIBRARIES}
            )
endif (UNIX)

if (UNIX)
    ###
    # OrlandoViols Client
    ###
    juce_add_gui_app(OrlandoViols
            BUNDLE_ID com.orlandoviols.client
            ICON_BIG ${CMAKE_CURRENT_LIST_DIR}/assets/orlandoviols/appicon.png
            NEEDS_CURL TRUE

            COMPANY_NAME "Digital Stage gGmbH"
            PRODUCT_NAME "OrlandoViols")
    juce_generate_juce_header(OrlandoViols)
    target_compile_definitions(OrlandoViols
            PRIVATE
            # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
            JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
            JUCE_USE_CURL=1     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
            JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:OrlandoViols,JUCE_PRODUCT_NAME>"
            JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:OrlandoViols,JUCE_VERSION>")
    juce_add_bundle_resources_directory(OrlandoViols ${CMAKE_CURRENT_LIST_DIR}/assets)
    target_include_directories(OrlandoViols
            SYSTEM PRIVATE
            ${CMAKE_CURRENT_LIST_DIR}/lib/libov/src
            SYSTEM PUBLIC
            ${JACK_INCLUDE_DIRS})
    target_link_libraries(OrlandoViols
            PRIVATE
            Ov::Ov
            ${TASCAR_PLUGIN_LIBRARIES}
            juce::juce_gui_extra
            PUBLIC
            ${JACK_LIBRARIES}
            juce::juce_recommended_config_flags
            juce::juce_recommended_lto_flags
            juce::juce_recommended_warning_flags)
    target_sources(OrlandoViols
            PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src/common/OvMixer.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/orlandoviols/Main.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/orlandoviols/ApplicationController.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/orlandoviols/TaskbarComponent.cpp
            )
    add_custom_command(TARGET OrlandoViols
            POST_BUILD
            COMMAND cp $<TARGET_FILE_DIR:zita-n2j>/zita-n2j
            $<TARGET_FILE_DIR:OrlandoViols>
            COMMENT "Copying zita-n2j" VERBATIM)
    add_custom_command(TARGET OrlandoViols
            POST_BUILD
            COMMAND cp $<TARGET_FILE_DIR:zita-j2n>/zita-j2n
            $<TARGET_FILE_DIR:OrlandoViols>
            COMMENT "Copying zita-j2n" VERBATIM)
endif (UNIX)


if (APPLE)
    set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR})
    INSTALL(TARGETS DigitalStage
            BUNDLE DESTINATION . COMPONENT Runtime)
    INSTALL(TARGETS OrlandoViols
            BUNDLE DESTINATION . COMPONENT Runtime)
    if (APPLE_CERT)
        message(STATUS "Signing app bundle is activated")
        install(CODE "
            include(BundleUtilities)
            fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/OrlandoViols.app\" \"\" \"$<TARGET_FILE_DIR:Tascar::Tascar>/../plugins\")
            execute_process(COMMAND codesign --force --deep --sign \"${APPLE_CERT}\" \"\${CMAKE_INSTALL_PREFIX}/OrlandoViols.app\")
        "
                DESTINATION .
                COMPONENT Runtime)
        install(CODE "
            include(BundleUtilities)
            fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/DigitalStage.app\" \"\" \"$<TARGET_FILE_DIR:Tascar::Tascar>/../plugins\")
            execute_process(COMMAND codesign --force --deep --sign \"${APPLE_CERT}\" \"\${CMAKE_INSTALL_PREFIX}/DigitalStage.app\")
        "
                DESTINATION .
                COMPONENT Runtime)
    else ()
        install(CODE "
            include(BundleUtilities)
            fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/OrlandoViols.app\" \"\" \"$<TARGET_FILE_DIR:Tascar::Tascar>/../plugins\")
        "
                DESTINATION .
                COMPONENT Runtime)
        install(CODE "
            include(BundleUtilities)
            fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/DigitalStage.app\" \"\" \"$<TARGET_FILE_DIR:Tascar::Tascar>/../plugins\")
        "
                DESTINATION .
                COMPONENT Runtime)
    endif ()
endif (APPLE)


set(CPACK_PACKAGE_HOMEPAGE_URL "https://www.digital-stage.org")
set(CPACK_PACKAGE_VENDOR "Digitale Bühne gGmbH")
set(CPACK_PACKAGE_CONTACT "Tobias Hegemann <tobias.hegemann@digital-stage.org")
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE)
set(CPACK_RESOURCE_FILE_README ${CMAKE_CURRENT_SOURCE_DIR}/README.md)

if (APPLE)
    set(CPACK_DMG_VOLUME_NAME "InstallDigitalStage")
    set(CPACK_PACKAGE_NAME "InstallDigitalStage")
    set(CPACK_GENERATOR "DragNDrop")
    set(CPACK_DMG_DS_STORE_SETUP_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/Packaging_DMGSetup.scpt)
    set(CPACK_DMG_BACKGROUND_IMAGE ${CMAKE_CURRENT_SOURCE_DIR}/assets/digitalstage/DMGBackground.tif)
    set(CPACK_SOURCE_STRIP_FILES "")
    set(CPACK_COMPONENTS_ALL Runtime)
    include(CPack)
endif (APPLE)

if (LINUX)
    include(GNUInstallDirs)
    set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR})
    install(TARGETS DigitalStage OrlandoViols Tascar ${TASCAR_PLUGIN_LIBRARIES}
            BUNDLE DESTINATION .
            RUNTIME DESTINATION .
            RESOURCE DESTINATION assets
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            PRIVATE_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
            PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    set(CPACK_GENERATOR "TGZ")
    set(CPACK_SOURCE_IGNORE_FILES
            \\.git/
            build/
            ".*~$"
            )
    set(CPACK_VERBATIM_VARIABLES YES)
    set(CPACK_ARCHIVE_FILE_NAME "DigitalStage-${CMAKE_PROJECT_VERSION}-linux")
    include(CPack)
endif (LINUX)