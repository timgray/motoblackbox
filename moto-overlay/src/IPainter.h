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
#include "PainterDTO.h"

class IPainter {
public:
        virtual ~IPainter () {}
        virtual void paint (cairo_t *cr, PainterDTO const &dto) = 0;
};

#endif /* IPAINTER_H_ */
