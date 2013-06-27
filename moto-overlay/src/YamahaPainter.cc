/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#include "YamahaPainter.h"
#include <cairo.h>
#include <cairo-gobject.h>
#include <cairo-ft.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#include FT_STROKER_H
#include FT_GLYPH_H
#include FT_TRUETYPE_IDS_H
#include <cassert>

struct YamahaPainter::Impl {
        FT_Library ft_library;
        FT_Face ft_face;
        cairo_font_face_t *cairo_ft_face = 0;
        cairo_surface_t *dashSurface = 0;
        cairo_surface_t *pointerSurface = 0;
};

YamahaPainter::YamahaPainter ()
{
        impl = new Impl ();
        double ptSize = 50.0;
        int device_hdpi = 100;
        int device_vdpi = 100;

        /* Init freetype */
        assert(!FT_Init_FreeType(&impl->ft_library));

        assert (!FT_New_Face (impl->ft_library, "fonts/7_segment_display.ttf", 0, &impl->ft_face));
        assert (!FT_Set_Char_Size (impl->ft_face, 0, ptSize, device_hdpi, device_vdpi));

        impl->cairo_ft_face = cairo_ft_font_face_create_for_ft_face (impl->ft_face, 0);
        impl->dashSurface = cairo_image_surface_create_from_png ("image/gauge.png");
        impl->pointerSurface = cairo_image_surface_create_from_png ("image/pointer.png");
}

YamahaPainter::~YamahaPainter ()
{
        FT_Done_Face (impl->ft_face);
        FT_Done_FreeType (impl->ft_library);
        delete impl;
}

void YamahaPainter::paint (cairo_t *cr, PainterDTO const &dto)
{
        // Velocity
        cairo_set_font_face (cr, impl->cairo_ft_face);
        cairo_set_font_size (cr, 18.0);
        cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 1.0);
        cairo_move_to(cr, 1006, 605);
        cairo_show_text(cr, "222");

        // Temp
        cairo_set_font_size (cr, 10.0);
        cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 1.0);
        cairo_move_to(cr, 950, 595);
        cairo_show_text(cr, "78");

        // Dash
        cairo_save (cr);
        cairo_translate (cr, 880, 520);
        cairo_scale (cr, 0.2, 0.2);
        cairo_set_source_surface (cr, impl->dashSurface, 0, 0);
        cairo_paint (cr);
        cairo_restore (cr);

        // Pointer
        cairo_translate (cr, 1115.5, 611);
        cairo_scale (cr, 0.2, 0.2);
        cairo_translate (cr, 308, 71);
        cairo_rotate (cr, dto.rpm);
        cairo_translate (cr, -308, -71);
        cairo_set_source_surface (cr, impl->pointerSurface, 0, 0);
        cairo_paint (cr);
}
