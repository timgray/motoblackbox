SET(CMAKE_SYSTEM_NAME Generic)

#SET(CMAKE_C_COMPILER /home/iwasz/local/share/raspi-gcc/arm-unknown-linux-gnueabi/bin/arm-unknown-linux-gnueabi-gcc)
#SET(CMAKE_CXX_COMPILER /home/iwasz/local/share/raspi-gcc/arm-unknown-linux-gnueabi/bin/arm-unknown-linux-gnueabi-g++)

#SET(CMAKE_C_COMPILER /home/iwasz/local/share/raspi-linaro/tools/arm-bcm2708/arm-bcm2708-linux-gnueabi/bin/arm-bcm2708-linux-gnueabi-gcc)
#SET(CMAKE_CXX_COMPILER /home/iwasz/local/share/raspi-linaro/tools/arm-bcm2708/arm-bcm2708-linux-gnueabi/bin/arm-bcm2708-linux-gnueabi-g++)

SET(CMAKE_C_COMPILER /home/iwasz/local/share/raspi-linaro/tools/arm-bcm2708/arm-bcm2708hardfp-linux-gnueabi/bin/arm-bcm2708hardfp-linux-gnueabi-gcc)
SET(CMAKE_CXX_COMPILER /home/iwasz/local/share/raspi-linaro/tools/arm-bcm2708/arm-bcm2708hardfp-linux-gnueabi/bin/arm-bcm2708hardfp-linux-gnueabi-g++)

#SET(CFLAGS "-mfpu=vfp -mfloat-abi=hard -march=armv6zk -mtune=arm1176jzf-s -pthread")
#SET(CXXFLAGS "-mfpu=vfp -mfloat-abi=hard -march=armv6zk -mtune=arm1176jzf-s -std=c++11 -pthread")
SET(CFLAGS "-mfpu=vfp -mfloat-abi=hard -march=armv6zk -mtune=arm1176jzf-s -static-libgcc -pthread")
SET(CXXFLAGS "-mfpu=vfp -mfloat-abi=hard -march=armv6zk -mtune=arm1176jzf-s -static-libstdc++ -static-libgcc -std=c++11 -pthread")

SET(CMAKE_C_FLAGS  ${CFLAGS})
SET(CMAKE_CXX_FLAGS ${CXXFLAGS})
