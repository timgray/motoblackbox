/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#ifndef SHIELD_H_
#define SHIELD_H_

#include <ostream>
#include <boost/circular_buffer.hpp>

extern const char *PORT;

/**
 * Data frame from AVR shield.
 */
struct Frame {
        float velocity = 0;
        float rpm = 0;
        float engineTemp = 0;
        float airTemp = 0;
        bool frontBrake = false;
        bool rearBrake = false;
        bool leftTurn = false;
        bool rightTurn = false;
        bool parkingLight = false;
};

extern std::ostream &operator<< (std::ostream &o, Frame const &f);

/**
 * AVR shield on top of the RasPI.
 */
class Shield {
public:

        Shield (std::string const &port);
        virtual ~Shield ();

        Frame read ();

private:

        typedef boost::circular_buffer<uint8_t> InputData;
        bool validateBuffer (InputData const &d);
        float computeTemp (uint8_t temp);

private:

        int ttyFd = 0;
        const unsigned int FRAME_SIZE = 8; // Start (command) byte, 6 data bytes and 1 checksum byte.

        const unsigned int BUF_VELOCITY_MSB = 1;
        const unsigned int BUF_VELOCITY_LSB = 2;
        const unsigned int BUF_RPM = 3;
        const unsigned int BUF_ENGINE_TEMP = 4;
        const unsigned int BUF_GPIO = 5;
        const unsigned int BUF_AIR_TEMP = 6;

        const unsigned int GPIO_LEFT_TURN = 0;
        const unsigned int GPIO_RIGHT_TURN = 1;
        const unsigned int GPIO_FRONT_BRAKE = 2;
        const unsigned int GPIO_REAR_BRAKE = 3;
        const unsigned int GPIO_PARKING_LIGHT = 4;

        const float ENGINE_TEMP_FACTOR = 0.5;
        const float RPM_FACTOR = 50;
        const float VELOCITY_FACTOR = 0.4; // Found empirically
        const uint8_t SHIELD_COMMAND_BYTE = 0x01;
};

#endif /* SHIELD_H_ */
