/* Pango
 * pangox.h:
 *
 * Copyright (C) 1999 Red Hat Software
 * Copyright (C) 2000 SuSE Linux Ltd
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __PANGOX_H__
#define __PANGOX_H__

#include <glib.h>
#include <pango/pango-layout.h>

G_BEGIN_DECLS

#include <X11/Xlib.h>

#ifndef PANGO_DISABLE_DEPRECATED

#define PANGO_RENDER_TYPE_X "PangoRenderX"

typedef GC (*PangoGetGCFunc) (PangoContext *context, PangoColor *color, GC base_gc);
typedef void (*PangoFreeGCFunc) (PangoContext *context, GC gc);

/* Calls for applications
 */
PangoContext * pango_x_get_context        (Display          *display);
void           pango_x_context_set_funcs  (PangoContext     *context,
					   PangoGetGCFunc    get_gc_func,
					   PangoFreeGCFunc   free_gc_func);

PangoFont *    pango_x_load_font          (Display          *display,
					   const gchar      *spec);
void           pango_x_render             (Display          *display,
					   Drawable          d,
					   GC                gc,
					   PangoFont        *font,
					   PangoGlyphString *glyphs,
					   gint              x,
					   gint              y);
void           pango_x_render_layout_line (Display          *display,
					   Drawable          drawable,
					   GC                gc,
					   PangoLayoutLine  *line,
					   int               x,
					   int               y);
void           pango_x_render_layout      (Display          *display,
					   Drawable          drawable,
					   GC                gc,
					   PangoLayout      *layout,
					   int               x,
					   int               y);

/* API for rendering modules
 */
typedef guint16 PangoXSubfont;

#define PANGO_X_MAKE_GLYPH(subfont,index_) ((subfont)<<16 | (index_))
#define PANGO_X_GLYPH_SUBFONT(glyph) ((glyph)>>16)
#define PANGO_X_GLYPH_INDEX(glyph) ((glyph) & 0xffff)

int        pango_x_list_subfonts     (PangoFont      *font,
				      char          **charsets,
				      int             n_charsets,
				      PangoXSubfont **subfont_ids,
				      int           **subfont_charsets);
gboolean   pango_x_has_glyph         (PangoFont      *font,
				      PangoGlyph      glyph);
PangoGlyph pango_x_get_unknown_glyph (PangoFont      *font);

#ifdef PANGO_ENABLE_ENGINE
PangoGlyph pango_x_font_get_unknown_glyph (PangoFont    *font,
					   gunichar      wc);
#endif /* PANGO_ENABLE_ENGINE */

/* API for libraries that want to use PangoX mixed with classic X fonts.
 */
typedef struct _PangoXFontCache PangoXFontCache;

PangoXFontCache *pango_x_font_cache_new     (Display         *display);
void             pango_x_font_cache_free    (PangoXFontCache *cache);

XFontStruct * pango_x_font_cache_load      (PangoXFontCache *cache,
					    const char      *xlfd);
void          pango_x_font_cache_unload    (PangoXFontCache *cache,
					    XFontStruct     *fs);

PangoFontMap *   pango_x_font_map_for_display  (Display     *display);
void             pango_x_shutdown_display      (Display     *display);
PangoXFontCache *pango_x_font_map_get_font_cache (PangoFontMap *font_map);

char *pango_x_font_subfont_xlfd (PangoFont     *font,
				 PangoXSubfont  subfont_id);


gboolean pango_x_find_first_subfont (PangoFont     *font,
				     char         **charsets,
				     int            n_charsets,
				     PangoXSubfont *rfont);

void pango_x_fallback_shape (PangoFont        *font,
			     PangoGlyphString *glyphs,
			     const char       *text,
			     int               n_chars);

gboolean pango_x_apply_ligatures (PangoFont     *font,
				  PangoXSubfont  subfont,
				  gunichar     **glyphs,
				  int           *n_glyphs,
				  int          **clusters);

#endif /* PANGO_DISABLE_DEPRECATED */

G_END_DECLS

#endif /* __PANGOX_H__ */
