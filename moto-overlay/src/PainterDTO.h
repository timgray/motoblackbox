/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#ifndef PAINTERDTO_H_
#define PAINTERDTO_H_

struct PainterDTO {

        PainterDTO (float r, float v, float e) : rpm {r}, velocity {v}, engineTemp {e} {}

        float rpm = 0;
        float velocity = 0;
        float engineTemp = 0;
};

#endif /* PAINTERDTO_H_ */
