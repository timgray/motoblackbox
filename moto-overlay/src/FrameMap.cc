/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#include "FrameMap.h"
#include <fstream>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include "Frame.h"

/**
 * CSV : timestamp, velocity, rpm, engineTemp, airTemp, frontBrake, rearBrake, leftTurn, rightTurn, parkingLight
 */
FrameVector readFrames (std::string const &path)
{
        FrameVector frames;
        std::ifstream file (path);

        Frame frame;
        uint32_t time;

        typedef boost::tokenizer <boost::escaped_list_separator <char>> Tokenizer;
        std::string line;

        while (std::getline (file, line)) {
#if 0
                std::cerr << line << std::endl;
#endif

                Tokenizer tok (line);
                Tokenizer::const_iterator i = tok.begin ();
                frame.timestamp = boost::lexical_cast <uint32_t> (*i++);
                frame.velocity = boost::lexical_cast <float> (*i++);
                frame.rpm = boost::lexical_cast <float> (*i++);
                frame.engineTemp = boost::lexical_cast <float> (*i++);
                frame.airTemp = boost::lexical_cast <float> (*i++);
                frame.frontBrake = boost::lexical_cast <bool> (*i++);
                frame.rearBrake = boost::lexical_cast <bool> (*i++);
                frame.leftTurn = boost::lexical_cast <bool> (*i++);
                frame.rightTurn = boost::lexical_cast <bool> (*i++);
                frame.parkingLight = boost::lexical_cast <bool> (*i++);
                frames.push_back (frame);
        }

        return frames;
}

std::ostream &operator<< (std::ostream &o, FrameVector const &frames)
{
        for (Frame const &frame : frames) {
                o << frame << "\n";
        }

        return o;
}
