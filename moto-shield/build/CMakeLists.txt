cmake_minimum_required(VERSION 2.6)

SET (CMAKE_VERBOSE_MAKEFILE 0) 
SET (WITH_ARDUINO_CORE 0) 
SET (UPLOAD_AFTER_BUILD 0) 
SET (DUDE_PROGRAMMER "usbasp")
#SET (DUDE_PORT "/dev/ttyACM1")

# Name of thos project and excecutable file as well.
PROJECT (moto-shield)

# Definicje per płytka/procesor etc.
include (arduino_mega.cmake)
#include (attiny13.cmake)
#include (atmega8.cmake)
#include (atmega32u2.cmake)

# Czy budować bibliotekę arduino? Zwykłe projekty AVR nie potrzebują jej.
IF (WITH_ARDUINO_CORE)
    
    # ArduinoCore library.
    SET (CORE_LIB_SOURCES)
    AUX_SOURCE_DIRECTORY (../src/libarduinocore CORE_LIB_SOURCES)
    ADD_LIBRARY (arduinocore ${CORE_LIB_SOURCES})
    
    # Actual program
    include_directories (../src/libarduinocore)
    LINK_DIRECTORIES (../src/libarduinocore)
    link_libraries (arduinocore)
ENDIF ()

AUX_SOURCE_DIRECTORY (../src/ APP_SOURCES)
add_executable (${PROJECT_NAME} ${APP_SOURCES})

# AVR Create Extended Listing
ADD_CUSTOM_TARGET ("${PROJECT_NAME}.lss" ALL COMMAND avr-objdump -h -S ${PROJECT_NAME} > "${PROJECT_NAME}.lss"  DEPENDS ${PROJECT_NAME})

# Create Flas200h image (ihex format)
ADD_CUSTOM_TARGET ("${PROJECT_NAME}.hex" ALL COMMAND avr-objcopy -R .eeprom -O ihex ${PROJECT_NAME} "${PROJECT_NAME}.hex" DEPENDS ${PROJECT_NAME})

# Upload to chip.
IF (UPLOAD_AFTER_BUILD)
    ADD_CUSTOM_TARGET (up ALL COMMAND avrdude -b 19200 -D -V -C /etc/avrdude.conf -p${DUDE_CHIP_NAME} -P${DUDE_PORT} -c${DUDE_PROGRAMMER} -U "flash:w:${PROJECT_NAME}.hex:i" DEPENDS "${PROJECT_NAME}.hex")
ELSE ()    
    ADD_CUSTOM_TARGET (up COMMAND avrdude -b 19200 -D -V -C /etc/avrdude.conf -p${DUDE_CHIP_NAME} -P${DUDE_PORT} -c${DUDE_PROGRAMMER} -U "flash:w:${PROJECT_NAME}.hex:i" DEPENDS "${PROJECT_NAME}.hex")
ENDIF ()
