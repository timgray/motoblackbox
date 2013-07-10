#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <inttypes.h>
#include <iostream>
#include <algorithm>
#include "Shield.h"

const char *PORT = "/dev/ttyAMA0";

std::ostream &operator<< (std::ostream &o, Frame const &f)
{
        o << "SPEED=" << f.velocity <<
             " km/h, RPM=" << f.rpm <<
             " rpm, ENGINE=" << f. engineTemp <<
             " \u2103, AIR=" << f.airTemp <<
             " \u2103, FB=" << f.frontBrake <<
             ", RB=" << f.rearBrake <<
             ", LEFT=" << f.leftTurn <<
             ", RIGHT=" << f.rightTurn <<
             ", PARKL=" << f.parkingLight;
        return o;
}

Shield::Shield (std::string const &port)
{
#if 0
        std::cerr << "Shield::Shield : starting serial port communication..." << std::endl;
#endif
        struct termios tio;

        memset(&tio, 0, sizeof(tio));
        tio.c_iflag = 0;
        tio.c_oflag = 0;
        tio.c_cflag = CS8 | CREAD | CLOCAL; // 8n1, see termios.h for more information
        tio.c_lflag = 0;
        tio.c_cc[VMIN] = 1;
        tio.c_cc[VTIME] = 2;

        ttyFd = open(PORT, O_RDONLY | O_SYNC);
        cfsetispeed(&tio, B38400);

        tcsetattr(ttyFd, TCSANOW, &tio);
}

Shield::~Shield ()
{
        close(ttyFd);
}

Frame Shield::read ()
{
        uint8_t c;
        boost::circular_buffer<uint8_t> buffer (FRAME_SIZE);
        Frame frame;

        do {
                if (::read (ttyFd, &c, 1) > 0) {
                        buffer.push_back (c);
                }
        }
        while (!validateBuffer (buffer));

        frame.velocity = ((buffer[BUF_VELOCITY_MSB] << 8) | (buffer[BUF_VELOCITY_LSB])) * VELOCITY_FACTOR;
        frame.rpm = buffer[BUF_RPM] * RPM_FACTOR;
        frame.engineTemp = computeTemp (buffer[BUF_ENGINE_TEMP]);
        frame.airTemp = buffer[BUF_AIR_TEMP];
        frame.frontBrake = buffer[BUF_GPIO] & (1 << GPIO_FRONT_BRAKE);
        frame.rearBrake = buffer[BUF_GPIO] & (1 << GPIO_REAR_BRAKE);
        frame.leftTurn = buffer[BUF_GPIO] & (1 << GPIO_LEFT_TURN);
        frame.rightTurn = buffer[BUF_GPIO] & (1 << GPIO_RIGHT_TURN);
        frame.parkingLight = buffer[BUF_GPIO] & (1 << GPIO_PARKING_LIGHT);

        return frame;
}

bool Shield::validateBuffer (InputData const &d)
{
#if 0
        std::cerr << "Shield::validateBuffer ";

        for (uint8_t b : d) {
                std::cerr << std::hex << (int)b << ", ";
        }

        std::cerr << "F=" << d.full () << ", FR=" << std::hex << (int)d.front ();

        if (d.full ()) {
                std::cerr << ", AC=" << std::accumulate (d.begin () + 1, d.end () - 1, 0) << ", B=" << std::hex << (int)d.back ();
        }

        std::cerr << std::endl;
#endif

        // Computer sum of buffer bytes ommiting the first and last ones (COMMAND, and checksum respectively).
        return d.full () && d.front () == SHIELD_COMMAND_BYTE & static_cast <uint8_t> (std::accumulate (d.begin () + 1, d.end () - 1, 0)) == d.back ();
}

float Shield::computeTemp (uint8_t temp)
{
        // Empirical equation. Found by my wife with excel.
        return 0.95515 * temp - 25.724;
}
