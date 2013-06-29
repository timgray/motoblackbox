/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#ifndef FRAMEMAP_H_
#define FRAMEMAP_H_

#include "Frame.h"
#include <vector>
#include <string>
#include <ostream>

typedef std::vector <Frame> FrameVector;

FrameVector readFrames (std::string const &path);

std::ostream &operator<< (std::ostream &o, FrameVector const &map);

#endif /* FRAMEMAP_H_ */
