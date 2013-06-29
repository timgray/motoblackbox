/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#ifndef PAINTERDTO_H_
#define PAINTERDTO_H_

#include <ostream>

struct Frame {

        Frame () {}
        Frame (float r, float v, float e) : rpm {r}, velocity {v}, engineTemp {e} {}

        uint32_t timestamp = 0;
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

std::ostream &operator<< (std::ostream &o, Frame const &f);

#endif /* PAINTERDTO_H_ */
