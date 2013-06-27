/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#ifndef YAMAHAPAINTER_H_
#define YAMAHAPAINTER_H_

#include "IPainter.h"
#include <cairo.h>
#include "PainterDTO.h"

class YamahaPainter : public IPainter {
public:
        YamahaPainter ();
        virtual ~YamahaPainter ();

        virtual void paint (cairo_t *cr, PainterDTO const &dto);

private:

        struct Impl;
        Impl *impl = 0;
};

#endif /* YAMAHAPAINTER_H_ */
