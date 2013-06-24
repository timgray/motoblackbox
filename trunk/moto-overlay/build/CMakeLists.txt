# Wymagaj tej wersji - dopiero w niej jest nowe makro FindPkgConfig
CMAKE_MINIMUM_REQUIRED (VERSION 2.6.0 FATAL_ERROR)

# Name of thos project and excecutable file as well.
PROJECT (moto-overlay)

SET (CMAKE_VERBOSE_MAKEFILE 0) 
ADD_DEFINITIONS (-O0)
ADD_DEFINITIONS(-std=c++11)

SET(Boost_ADDITIONAL_VERSIONS "1.41" "1.41.0")
find_package( Boost 1.41.0 )
include_directories(${Boost_INCLUDE_DIRS})

include (FindPkgConfig)

pkg_check_modules (CAIRO REQUIRED "cairo")
INCLUDE_DIRECTORIES (${CAIRO_INCLUDE_DIRS})
link_directories(${CAIRO_LIBRARY_DIRS})

pkg_check_modules (CAIRO_PNG REQUIRED "cairo-png")
INCLUDE_DIRECTORIES (${CAIRO_PNG_INCLUDE_DIRS})
link_directories(${CAIRO_PNG_LIBRARY_DIRS})

pkg_check_modules (FREETYPE REQUIRED "freetype2")
INCLUDE_DIRECTORIES (${FREETYPE_INCLUDE_DIRS})
link_directories(${FREETYPE_LIBRARY_DIRS})

AUX_SOURCE_DIRECTORY (../src/ APP_SOURCES)
add_executable (${PROJECT_NAME} ${APP_SOURCES})
     
TARGET_LINK_LIBRARIES (${PROJECT_NAME} ${CAIRO_LIBRARIES})
TARGET_LINK_LIBRARIES (${PROJECT_NAME} ${CAIRO_PNG_LIBRARIES})
TARGET_LINK_LIBRARIES (${PROJECT_NAME} ${FREETYPE_LIBRARIES})
