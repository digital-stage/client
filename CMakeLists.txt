cmake_minimum_required(VERSION 3.12)

project(DIGITALSTAGE_APP VERSION 0.0.1)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (APPLE)
        include(${CMAKE_CURRENT_LIST_DIR}/cmake/FixAppleArchitecture.cmake)
        set(CMAKE_OSX_DEPLOYMENT_TARGET 10.14)
endif (APPLE)

add_subdirectory(JUCE)
add_subdirectory(libds)

set(COMPILE_OV_WITH_CMAKE ON CACHE BOOL "Use cmake to configure and build OV")

add_compile_definitions(SIGNUP_URL="https://single.dstage.org/account/signup")
add_compile_definitions(STAGE_URL="https://single.dstage.org/stage")
add_compile_definitions(MIXER_URL="https://single.dstage.org/mixer")
add_compile_definitions(AUTH_URL="http://single.dstage.org/api/auth")

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
        ${CMAKE_CURRENT_SOURCE_DIR}/src/TaskbarComponent.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/LoginWindow.cpp
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

target_include_directories(DigitalStage
        PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/libds/src
    )

target_link_libraries(DigitalStage
    PRIVATE
        # GuiAppData            # If we'd created a binary data target, we'd link to it here
        juce::juce_gui_extra
        ds
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

if(UNIX)
    if(COMPILE_OV_WITH_CMAKE)
        message(STATUS "Using cmake to build OV")
        add_subdirectory(libov)
        target_include_directories(DigitalStage 
        PRIVATE 
            ov)
        target_link_libraries(DigitalStage
        PRIVATE
            ov)
    else()
        message(STATUS "Using Makefile to build OV")
        target_sources(DigitalStage
            PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src/OvController.cpp
        )
        # Add ov dependency only on unix like systems
        include(${CMAKE_CURRENT_LIST_DIR}/cmake/BuildLibOv.cmake)
        target_include_directories(DigitalStage 
        PRIVATE 
            ${OV_INCLUDE_DIRS})
        target_link_libraries(DigitalStage
        PRIVATE
            ${OV_LIBRARIES})
        add_dependencies(DigitalStage libov)
    endif()
endif(UNIX)

juce_add_bundle_resources_directory(DigitalStage ${CMAKE_CURRENT_LIST_DIR}/assets)