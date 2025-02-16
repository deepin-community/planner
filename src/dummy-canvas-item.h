/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 1997, 1998, 1999, 2000 Free Software Foundation
 * Copyright (C) 2009 Maurice van der Pot <griffon26@kfk4ever.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
/*
  @NOTATION@
 */

/* Dummy item type for GnomeCanvas widget
 *
 * This dummy item can be used to manipulate the bounding box returned by the
 * get_bounds function of the canvas root. It has no visible shape, but is
 * included in bounding box calculations.
 *
 * Author: Maurice van der Pot <griffon26@kfk4ever.com>
 */

#pragma once

#include <libgnomecanvas/gnome-canvas.h>


G_BEGIN_DECLS

/* Base class for rectangle and ellipse item types.  These are defined by their top-left and
 * bottom-right corners.  Rectangles and ellipses share the following arguments:
 *
 * name			type		read/write	description
 * ------------------------------------------------------------------------------------------
 * x1			double		RW		Leftmost coordinate of rectangle
 * y1			double		RW		Topmost coordinate of rectangle
 * x2			double		RW		Rightmost coordinate of rectangle
 * y2			double		RW		Bottommost coordinate of rectangle
 */

#define GNOME_TYPE_CANVAS_DUMMY            (gnome_canvas_dummy_get_type ())
#define GNOME_CANVAS_DUMMY(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GNOME_TYPE_CANVAS_DUMMY, GnomeCanvasDummy))
#define GNOME_CANVAS_DUMMY_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GNOME_TYPE_CANVAS_DUMMY, GnomeCanvasDummyClass))
#define GNOME_IS_CANVAS_DUMMY(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GNOME_TYPE_CANVAS_DUMMY))
#define GNOME_IS_CANVAS_DUMMY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GNOME_TYPE_CANVAS_DUMMY))
#define GNOME_CANVAS_DUMMY_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GNOME_TYPE_CANVAS_DUMMY, GnomeCanvasDummyClass))


typedef struct _GnomeCanvasDummy GnomeCanvasDummy;
typedef struct _GnomeCanvasDummyClass GnomeCanvasDummyClass;

struct _GnomeCanvasDummy {
	GnomeCanvasItem item;

        double x1;
        double x2;
        double y1;
        double y2;

};

struct _GnomeCanvasDummyClass {
	GnomeCanvasItemClass parent_class;
};


/* Standard Gtk function */
GType gnome_canvas_dummy_get_type (void) G_GNUC_CONST;


G_END_DECLS
