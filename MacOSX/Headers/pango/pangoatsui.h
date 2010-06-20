/* Pango
 * pangoatsui.h:
 *
 * Copyright (C) 2005 Imendio AB
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

#ifndef __PANGOATSUI_H__
#define __PANGOATSUI_H__

#include <pango/pango-context.h>
#include <pango/pango-font.h>
#include <ApplicationServices/ApplicationServices.h>

G_BEGIN_DECLS

#define PANGO_TYPE_ATSUI_FONT       (pango_atsui_font_get_type ())
#define PANGO_ATSUI_FONT(object)    (G_TYPE_CHECK_INSTANCE_CAST ((object), PANGO_TYPE_ATSUI_FONT, PangoATSUIFont))
#define PANGO_IS_ATSUI_FONT(object) (G_TYPE_CHECK_INSTANCE_TYPE ((object), PANGO_TYPE_ATSUI_FONT))

typedef struct _PangoATSUIFont         PangoATSUIFont;
typedef struct _PangoATSUIFontClass    PangoATSUIFontClass;

#if defined(PANGO_ENABLE_ENGINE) || defined(PANGO_ENABLE_BACKEND)

#define PANGO_RENDER_TYPE_ATSUI "PangoRenderATSUI"

#ifdef PANGO_ENABLE_BACKEND

#define PANGO_ATSUI_FONT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PANGO_TYPE_ATSUI_FONT, PangoATSUIFontClass))
#define PANGO_IS_ATSUI_FONT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PANGO_TYPE_ATSUI_FONT))
#define PANGO_ATSUI_FONT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PANGO_TYPE_ATSUI_FONT, PangoATSUIFontClass))

typedef struct _PangoATSUIFontPrivate  PangoATSUIFontPrivate;

struct _PangoATSUIFont
{
  PangoFont parent_instance;
  PangoATSUIFontPrivate *priv;
};

struct _PangoATSUIFontClass
{
  PangoFontClass parent_class;

  /*< private >*/

  /* Padding for future expansion */
  void (*_pango_reserved1) (void);
  void (*_pango_reserved2) (void);
  void (*_pango_reserved3) (void);
  void (*_pango_reserved4) (void);
};

#endif /* PANGO_ENABLE_BACKEND */

ATSUFontID pango_atsui_font_get_atsu_font_id (PangoATSUIFont *font);

#endif /* PANGO_ENABLE_ENGINE || PANGO_ENABLE_BACKEND */

GType      pango_atsui_font_get_type         (void) G_GNUC_CONST;

G_END_DECLS

#endif /* __PANGOATSUI_H__ */
