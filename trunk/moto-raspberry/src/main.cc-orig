#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <inttypes.h>
#include <iostream>
#include <algorithm>
#include <boost/circular_buffer.hpp>

const char *PORT = "/dev/ttyAMA0";

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

/**
 *
 */
int main (int argc, char** argv)
{
        Shield port (PORT);
        uint8_t cnt = 0;
        char mark[] = { '/', '-', '\\', '|' };

        while (true) {
                std::cout << "\033[A\033[2K" << port.read () << " [" << mark[++cnt % 4] << "]" << std::endl;
        }

        return EXIT_SUCCESS;
}
