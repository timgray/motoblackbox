#include <cairo.h>
#include <cairo-ft.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#include FT_STROKER_H
#include FT_GLYPH_H
#include FT_TRUETYPE_IDS_H
#include <cassert>

/*
 * TODO : proper cleanup of resources (FreeType ad cairo).
 */
int main (int argc, char **argv)
{
        double ptSize = 50.0;
        int device_hdpi = 100;
        int device_vdpi = 100;

        /* Init freetype */
        FT_Library ft_library;
        assert(!FT_Init_FreeType(&ft_library));

        FT_Face ft_face;
        assert (!FT_New_Face (ft_library, "fonts/7_segment_display.ttf", 0, &ft_face));
        assert (!FT_Set_Char_Size (ft_face, 0, ptSize, device_hdpi, device_vdpi));

        cairo_font_face_t *cairo_ft_face = cairo_ft_font_face_create_for_ft_face (ft_face, 0);

        cairo_surface_t *surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 640, 480);
        cairo_t *cr = cairo_create(surface);

//        cairo_select_font_face (cr, "serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_font_face (cr, cairo_ft_face);
        cairo_set_font_size (cr, 32.0);
        cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 1.0);
        cairo_move_to(cr, 10.0, 150.0);
        cairo_show_text(cr, "123");


        cairo_surface_t *dashSurface = cairo_image_surface_create_from_png ("image/dashboard.png");
        cairo_set_source_surface (cr, dashSurface, 220, 80);
        cairo_paint (cr);

        cairo_destroy(cr);
        cairo_surface_write_to_png(surface, "hello.png");
        cairo_surface_destroy(surface);

        FT_Done_Face (ft_face);
        FT_Done_FreeType (ft_library);
        return 0;
}
