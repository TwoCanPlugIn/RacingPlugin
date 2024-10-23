# ---------------------------------------------------------------------------
# Author:      Steven Adler based on (jongough aka. Jon Gough) Copyright: License:     wxWidgets License
# ---------------------------------------------------------------------------

## ----- When changing this file do NOT change the order in which sections occur        ----- ##
## ----- Changes should only be made between the section blocks that identify where     ----- ##
## ----- these changes should be. The whole configuration process relies on this        ----- ##
## ----- sequence to be successful                                                      ----- ##
##

##
## ----- Modify section below to include all the details for your plugin ----- ##
##

set(CMLOC "CMakeLists: ")

# define minimum cmake version
cmake_minimum_required(VERSION 3.1.1)
if(COMMAND cmake_policy)
    if(POLICY CMP0043)
        cmake_policy(SET CMP0043 NEW)
    endif(POLICY CMP0043)
    cmake_policy(SET CMP0048 NEW)
    if(POLICY CMP0077)
        cmake_policy(SET CMP0077 NEW)
    endif(POLICY CMP0077)
endif(COMMAND cmake_policy)


# define plugin name, owner and versions
set(VERBOSE_NAME "Race_Start_Display")
set(COMMON_NAME "Race Start Display")
set(TITLE_NAME "RACESTARTDISPLAY")
set(PACKAGE_CONTACT "twocanplugin@hotmail.com")
set(PACKAGE "race_start_display")
set(SHORT_DESCRIPTION "Race Start Display, A simple tool for yacht racing")
set(LONG_DESCRIPTION "Race Start Display, A simple tool for yacht racing, Displays countdown timer, distance and time to start line") 
set(VERSION_MAJOR "1")
set(VERSION_MINOR "3")
set(VERSION_PATCH "0")
set(VERSION_TWEAK "0")
set(VERSION_DATE "21/05/2023")
set(OCPN_MIN_VERSION "ov50")
set(OCPN_API_VERSION_MAJOR "1")
set(OCPN_API_VERSION_MINOR "16")
set(TP_COMMENT "Release for O5 using CI, FrontEnd v2")

set(PARENT "opencpn")

# The next line allows for setting up your own local webserver with git on it for testing purposes, the default is github.com
set(GIT_REPOSITORY_SERVER "github.com")

# Specifies cloudsmith user and repository names (note suffixes for repositories production, beta & alpha)
set(PROD "prod") # Standard Repos
set(BETA "beta") # Standard Repos
set(ALPHA "alpha") # Standard Repos

set(CLOUDSMITH_USER "steven-adler")
# set this if your cloudsmith repository name does not match your git repository name 
set(CLOUDSMITH_BASE_REPOSITORY "racingplugin")

set(XML_INFO_URL "https://opencpn.org/wiki/dokuwiki/doku.php?id=opencpn:developer_manual:plugins:beta_plugins:race-start")
set(XML_SUMMARY ${SHORT_DESCRIPTION})
set(XML_DESCRIPTION ${LONG_DESCRIPTION})

##
## ----- Modify section below if there are special requirements for the plugin ----- ##
##

## Racing Plugin (I think) uses SVG graphics for the toolbar icons
##option(PLUGIN_USE_SVG "Use SVG graphics" ON)

set(CMAKE_CXX_STANDARD 11)

# Prefer libGL.so to libOpenGL.so, see CMP0072
set(OpenGL_GL_PREFERENCE "LEGACY")
# Don't use local version of GLU library
set(USE_LOCAL_GLU FALSE)
option(USE_GL "Enable OpenGL support" OFF)
message(STATUS "${CMLOC}USE_GL: ${USE_GL}")

## Define the build type
if("${CMAKE_BUILD_TYPE}" STREQUAL "")
    set(CMAKE_BUILD_TYPE
        "Release"
        CACHE STRING "Choose the type of build" FORCE)
endif("${CMAKE_BUILD_TYPE}" STREQUAL "")
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "" "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
message(STATUS "${CMLOC}Build type: ${CMAKE_BUILD_TYPE}")

##
## ----- The statements below are used to setup standard variables that are required by the CMAKE process - do not remove ----- ##
##

project(${PACKAGE})

include("cmake/PluginSetup.cmake")

##
## ----- Modify section below if there are special requirements for the plugin ----- ##
##

set(CMAKE_VERBOSE_MAKEFILE ON)

option(Plugin_CXX11 "Use c++11" OFF)


##
## ----- do not change next section - needed to configure build process ----- ##
##

include("cmake/PluginConfigure.cmake")

##
## ----- Change below to match project requirements for source, headers, etc. ----- ##
##

## Racing Plugin does not understand/use this definition
##add_definitions(-DUSE_S57)

##Racing plugin uses SVG for toolbar icons
add_definitions(-DPLUGIN_USE_SVG)

## We don't use JSON
##if(APPLE)
##    option(PI_JSON_SCHEMA_VALIDATOR "Use JSON Schema validator" OFF)
##else(APPLE)
##    option(PI_JSON_SCHEMA_VALIDATOR "Use JSON Schema validator" ON)
##endif(APPLE)

## Need api-16 for ocpn_plugin.h
INCLUDE_DIRECTORIES(BEFORE ${PROJECT_SOURCE_DIR}/libs/ocpn-api)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/inc ${CMAKE_SOURCE_DIR}/src)

SET(SOURCES src/racing_plugin.cpp
            src/racing_gauge.cpp
            src/racing_dialog.cpp
            src/racing_dialogbase.cpp
            src/racing_window.cpp
            src/racing_windowbase.cpp)

SET(HEADERS inc/racing_plugin.h
            inc/racing_gauge.h
            inc/racing_dialog.h
            inc/racing_dialogbase.h
            inc/racing_window.h
            inc/racing_windowbase.h)


TARGET_SOURCES(${PACKAGE_NAME} PUBLIC ${SOURCES} )

##
## ----- do not change next section - needed to configure build process ----- ##
##

include("cmake/PluginInstall.cmake")
include("cmake/PluginLocalization.cmake")
include("cmake/PluginPackage.cmake")

