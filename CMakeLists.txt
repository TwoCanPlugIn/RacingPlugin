##---------------------------------------------------------------------------
## Author:      Steven Adler (based on standard OpenCPN Plug-In CMAKE commands)
## Copyright:   2020
## License:     GPL v3+
##---------------------------------------------------------------------------

# define minimum cmake version
CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

PROJECT(racing_pi)

SET(PACKAGE_NAME racing_pi)
SET(VERBOSE_NAME racing)
SET(TITLE_NAME racing)
SET(CPACK_PACKAGE_CONTACT "TwoCanPlugin@hotmail.com")
SET(CMAKE_BUILD_TYPE Release)
SET(CMAKE_CXX_STANDARD 11)

SET(PLUGIN_VERSION_MAJOR 1)
SET(PLUGIN_VERSION_MINOR 0)
SET(PLUGIN_VERSION_PATCH 0)
SET(PLUGIN_VERSION_DATE "2020-03-20")
SET(PLUGIN_NAME_SUFFIX "ov50")


INCLUDE("cmake/PluginConfigure.cmake")

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/inc ${CMAKE_SOURCE_DIR}/src)

SET(SRC_RACING
            src/racing_icons.cpp
            inc/racing_icons.h
            src/racing_plugin.cpp
            inc/racing_plugin.h
            src/racing_dialog.cpp
            inc/racing_dialog.h
            src/racing_dialogbase.cpp
            inc/racing_dialogbase.h
            src/racing_window.cpp
            inc/racing_window.h
            src/racing_windowbase.cpp
            inc/racing_windowbase.h
            inc/version.h
 	)

ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRC_RACING} )

INCLUDE("cmake/PluginInstall.cmake")
INCLUDE("cmake/PluginLocalization.cmake")
INCLUDE("cmake/PluginPackage.cmake")

