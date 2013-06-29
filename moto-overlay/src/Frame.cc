/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#include "Frame.h"

std::ostream &operator<< (std::ostream &o, Frame const &f)
{
        o << "T=" << f.timestamp <<
             ", SPEED=" << f.velocity <<
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
