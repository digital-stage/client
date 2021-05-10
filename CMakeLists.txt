cmake_minimum_required(VERSION 3.12)

project(DIGITALSTAGE_ORLANDOVIOLS_APP VERSION 0.0.1)


##
## CONFIGURATION
##
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
cmake_policy(SET CMP0087 NEW)
if (UNIX AND NOT APPLE)
    add_compile_options(-Wno-cast-align)
    set(LINUX TRUE)
endif ()
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules)
add_compile_definitions(SIGNUP_URL="https://live.dstage.org/account/signup")
add_compile_definitions(STAGE_URL="https://live.dstage.org/stage")
add_compile_definitions(MIXER_URL="https://live.dstage.org/mixer")
add_compile_definitions(AUTH_URL="https://auth.dstage.org")
add_compile_definitions(API_URL="wss://api.dstage.org")


##
## DEPENDENCIES
##
# Hotfixes
if (UNIX AND NOT APPLE)
    include(${PROJECT_SOURCE_DIR}/cmake/FixLinuxCpprest.cmake)
endif (UNIX AND NOT APPLE)
if (APPLE)
    include(${PROJECT_SOURCE_DIR}/cmake/FixAppleArchitecture.cmake)
endif (APPLE)
add_subdirectory(${PROJECT_SOURCE_DIR}/JUCE EXCLUDE_FROM_ALL)
if (UNIX)
    find_package(Jack REQUIRED)
    add_subdirectory(${PROJECT_SOURCE_DIR}/libov EXCLUDE_FROM_ALL)
    target_compile_definitions(Ov PUBLIC GUI)
    target_compile_definitions(Ov PUBLIC SHOWDEBUG)
    get_property(OV_PUBLIC_PLUGIN_LIBRARIES GLOBAL PROPERTY OV_PUBLIC_PLUGIN_LIBRARIES)
    add_subdirectory(zita-njbridge EXCLUDE_FROM_ALL)
endif (UNIX)


##
## TARGET
## create target and add include path
##
# DigitalStage Client (with ov optional)
juce_add_gui_app(DigitalStage
        BUNDLE_ID org.digital-stage.client
        ICON_BIG ${PROJECT_SOURCE_DIR}/assets/digitalstage/appicon.png
        NEEDS_CURL TRUE
        COMPANY_NAME "Digital Stage gGmbH"
        PRODUCT_NAME "DigitalStage")
juce_generate_juce_header(DigitalStage)
target_sources(DigitalStage
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/common/MacAddress.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/digitalstage/Main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/digitalstage/ApplicationController.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/digitalstage/LoginPane.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/digitalstage/TaskbarComponent.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/digitalstage/jammer/JammerHandler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/digitalstage/utils/SoundCardManager.cpp)
if (UNIX)
    target_sources(DigitalStage
            PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src/common/OvMixer.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/common/JackAudioController.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/digitalstage/ov/OvHandler.cpp
            )
endif (UNIX)
target_compile_definitions(DigitalStage
        PRIVATE
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
        JUCE_USE_CURL=1     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:DigitalStage,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:DigitalStage,JUCE_VERSION>")
juce_add_bundle_resources_directory(DigitalStage ${PROJECT_SOURCE_DIR}/assets)
target_link_libraries(DigitalStage
        PRIVATE
        # GuiAppData            # If we'd created a binary data target, we'd link to it here
        juce::juce_gui_extra
        juce::juce_audio_utils
        DigitalStage::Api
        DigitalStage::Auth
        PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
if (UNIX)
    target_link_libraries(DigitalStage
            PUBLIC
            Ov::Ov
            ${OV_PUBLIC_PLUGIN_LIBRARIES}
            ${JACK_LIBRARIES})
    target_include_directories(DigitalStage
            SYSTEM PUBLIC
            ${JACK_INCLUDE_DIRS})
endif (UNIX)

# OrlandoViols Client (only on unix systems)
if (UNIX)
    juce_add_gui_app(OrlandoViols
            BUNDLE_ID com.orlandoviols.client
            ICON_BIG ${PROJECT_SOURCE_DIR}/assets/orlandoviols/appicon.png
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
    juce_add_bundle_resources_directory(OrlandoViols ${PROJECT_SOURCE_DIR}/assets)
    target_include_directories(OrlandoViols
            SYSTEM PUBLIC
            ${JACK_INCLUDE_DIRS})
    target_link_libraries(OrlandoViols
            PRIVATE
            juce::juce_gui_extra
            PUBLIC
            Ov::Ov
            ${OV_PUBLIC_PLUGIN_LIBRARIES}
            ${JACK_LIBRARIES}
            juce::juce_recommended_config_flags
            juce::juce_recommended_lto_flags
            juce::juce_recommended_warning_flags)
    target_sources(OrlandoViols
            PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src/common/OvMixer.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/common/JackAudioController.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/orlandoviols/Main.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/orlandoviols/ApplicationController.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/orlandoviols/TaskbarComponent.cpp
            )
endif (UNIX)

if (UNIX)
    # ov CLI - analogoue to ovclient (only on unix systems)
    add_executable(ovclient ${CMAKE_CURRENT_SOURCE_DIR}/src/cli.cpp)
    target_include_directories(ovclient
            SYSTEM PUBLIC
            ${JACK_INCLUDE_DIRS})
    target_link_libraries(ovclient
            PUBLIC
            DigitalStageApi
            DigitalStageAuth
            Ov::Ov
            ${OV_PUBLIC_PLUGIN_LIBRARIES}
            ${JACK_LIBRARIES})
endif (UNIX)


##
## INSTALL
## install header files, generate and install cmake config files for find_package()
##
if (UNIX)
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
            RESOURCE DESTINATION . COMPONENT Assets
            BUNDLE DESTINATION . COMPONENT Runtime)
    INSTALL(TARGETS OrlandoViols
            RESOURCE DESTINATION . COMPONENT Assets
            BUNDLE DESTINATION . COMPONENT Runtime)
    if (APPLE_CERT)
        message(STATUS "Signing app bundle is activated")
        install(CODE "
            include(BundleUtilities)
            fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/OrlandoViols.app\" \"\" \"$<TARGET_FILE_DIR:Tascar::Tascar>/../plugins\")
            set(PLUGINS ${OV_PUBLIC_PLUGIN_LIBRARIES})
            foreach (PLUGIN IN LISTS PLUGINS)
                message(STATUS \"Fixing \${PLUGIN}\")
                execute_process(COMMAND ${CMAKE_INSTALL_NAME_TOOL} -id \${PLUGIN}.dylib \"\${CMAKE_INSTALL_PREFIX}/OrlandoViols.app/Contents/Frameworks/\${PLUGIN}.dylib\")
            endforeach()
            execute_process(COMMAND codesign --force --deep --sign \"${APPLE_CERT}\" \"\${CMAKE_INSTALL_PREFIX}/OrlandoViols.app\")"
                DESTINATION .
                COMPONENT Runtime)
        install(CODE "
            include(BundleUtilities)
            fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/DigitalStage.app\" \"\" \"$<TARGET_FILE_DIR:Tascar::Tascar>/../plugins\")
            set(PLUGINS ${OV_PUBLIC_PLUGIN_LIBRARIES})
            foreach (PLUGIN IN LISTS PLUGINS)
                message(STATUS \"Fixing \${PLUGIN}\")
                execute_process(COMMAND ${CMAKE_INSTALL_NAME_TOOL} -id \${PLUGIN}.dylib \"\${CMAKE_INSTALL_PREFIX}/DigitalStage.app/Contents/Frameworks/\${PLUGIN}.dylib\")
            endforeach()
            execute_process(COMMAND codesign --force --deep --sign \"${APPLE_CERT}\" \"\${CMAKE_INSTALL_PREFIX}/DigitalStage.app\")"
                DESTINATION .
                COMPONENT Runtime)
    else ()
        install(CODE "
            include(BundleUtilities)
            fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/OrlandoViols.app\" \"\" \"$<TARGET_FILE_DIR:Tascar::Tascar>/../plugins\")
            set(PLUGINS ${OV_PUBLIC_PLUGIN_LIBRARIES})
            foreach (PLUGIN IN LISTS PLUGINS)
                message(STATUS \"Fixing \${PLUGIN}\")
                execute_process(COMMAND ${CMAKE_INSTALL_NAME_TOOL} -id \${PLUGIN}.dylib \"\${CMAKE_INSTALL_PREFIX}/OrlandoViols.app/Contents/Frameworks/\${PLUGIN}.dylib\")
            endforeach()"
                DESTINATION .
                COMPONENT Runtime)
        install(CODE "
            include(BundleUtilities)
            fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/DigitalStage.app\" \"\" \"$<TARGET_FILE_DIR:Tascar::Tascar>/../plugins\")
            set(PLUGINS ${OV_PUBLIC_PLUGIN_LIBRARIES})
            foreach (PLUGIN IN LISTS PLUGINS)
                message(STATUS \"Fixing \${PLUGIN}\")
                execute_process(COMMAND ${CMAKE_INSTALL_NAME_TOOL} -id \${PLUGIN}.dylib \"\${CMAKE_INSTALL_PREFIX}/DigitalStage.app/Contents/Frameworks/\${PLUGIN}.dylib\")
            endforeach()"
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
    install(TARGETS DigitalStage OrlandoViols Tascar ${OV_PUBLIC_PLUGIN_LIBRARIES}
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
    set(CPACK_PACKAGE_FILE_NAME "DigitalStage-${CMAKE_PROJECT_VERSION}-linux")
    set(CPACK_ARCHIVE_FILE_NAME "DigitalStage-${CMAKE_PROJECT_VERSION}-linux")
    include(CPack)
endif (LINUX)