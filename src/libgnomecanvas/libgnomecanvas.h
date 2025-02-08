/*
 * Copyright (C) 1997, 1998, 1999, 2000 Free Software Foundation
 * All rights reserved.
 *
 * This file is part of the Gnome Library.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * published by the Free Software Foundation; either the
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * License along with the Gnome Library; see the file COPYING.LIB.  If not,
 */
/*
  @NOTATION@
 */

#ifndef LIBGNOMECANVAS_H
#define LIBGNOMECANVAS_H

#include <libgnomecanvas/gnome-canvas.h>
#include <libgnomecanvas/gnome-canvas-text.h>
#include <libgnomecanvas/gnome-canvas-pixbuf.h>
#include <libgnomecanvas/gnome-canvas-widget.h>
#include <libgnomecanvas/gnome-canvas-rect.h>
#include <libgnomecanvas/gnome-canvas-util.h>

G_BEGIN_DECLS

GType gnome_canvas_points_get_type (void);
#define GNOME_TYPE_CANVAS_POINTS gnome_canvas_points_get_type()

G_END_DECLS

#endif /* LIBGNOMECANVAS_H */
