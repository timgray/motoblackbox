cmake_minimum_required(VERSION 2.6)

SET (CMAKE_VERBOSE_MAKEFILE 0) 
SET (UPLOAD_AFTER_BUILD 0) 

# Name of thos project and excecutable file as well.
PROJECT (moto-raspberry)

# Definicje per płytka/procesor etc.
include (raspberrypi.cmake)

ADD_DEFINITIONS(-std=c++11)

SET(Boost_ADDITIONAL_VERSIONS "1.41" "1.41.0")
find_package( Boost 1.41.0 )
include_directories(${Boost_INCLUDE_DIRS})

AUX_SOURCE_DIRECTORY (../src/ APP_SOURCES)
add_executable (${PROJECT_NAME} ${APP_SOURCES})
