cmake_minimum_required(VERSION 2.6)

SET (CMAKE_VERBOSE_MAKEFILE 0) 
SET (UPLOAD_AFTER_BUILD 0) 

# Name of thos project and excecutable file as well.
PROJECT (moto-raspberry)

# Definicje per płytka/procesor etc.
include (raspberrypi.cmake)

ADD_DEFINITIONS(-std=c++11)

# cd /home/iwasz/Downloads/
# git clone https://github.com/raspberrypi/userland.git
# you get the idea...
SET(USERLAND_DIR "/home/iwasz/Downloads/userland")
include_directories(${USERLAND_DIR})
include_directories("${USERLAND_DIR}/interface/vcos")
include_directories("${USERLAND_DIR}/interface/vcos/pthreads")
include_directories("${USERLAND_DIR}/host_applications/linux/libs/bcm_host/include")
include_directories("${USERLAND_DIR}/interface/vmcs_host/linux")
link_directories("${USERLAND_DIR}/build/lib")

SET(Boost_ADDITIONAL_VERSIONS "1.41" "1.41.0")
find_package( Boost 1.41.0 )
include_directories(${Boost_INCLUDE_DIRS})

AUX_SOURCE_DIRECTORY (../src/ APP_SOURCES)
add_executable (${PROJECT_NAME} ${APP_SOURCES})

target_link_libraries(${PROJECT_NAME} mmal_core) 
target_link_libraries(${PROJECT_NAME} mmal_util)
target_link_libraries(${PROJECT_NAME} mmal_vc_client)
target_link_libraries(${PROJECT_NAME} vcos)
target_link_libraries(${PROJECT_NAME} bcm_host)
