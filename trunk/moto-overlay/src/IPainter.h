/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#ifndef IPAINTER_H_
#define IPAINTER_H_

#include <cairo.h>
#include "Frame.h"

class IPainter {
public:
        virtual ~IPainter () {}
        virtual void paint (cairo_t *cr, Frame const &dto) = 0;
};

#endif /* IPAINTER_H_ */
