/*
 * Copyright (C) 1997, 1998, 1999, 2000 Free Software Foundation
 * All rights reserved.
 *
 * This file is part of the Gnome Library.
 *
 * The Gnome Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * The Gnome Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with the Gnome Library; see the file COPYING.LIB.  If not,
 * write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
/*
  @NOTATION@
 */

/* Line/curve item type for GnomeCanvas widget
 *
 * GnomeCanvas is basically a port of the Tk toolkit's most excellent canvas widget.  Tk is
 * copyrighted by the Regents of the University of California, Sun Microsystems, and other parties.
 *
 *
 * Author: Federico Mena <federico@nuclecu.unam.mx>
 */

#include <config.h>
#include <math.h>
#include <string.h>
#include <cairo-gobject.h>
#include "libgnomecanvas/gnome-canvas-line.h"
#include "libgnomecanvas.h"

#define noVERBOSE

#define DEFAULT_SPLINE_STEPS 12		/* this is what Tk uses */
#define NUM_ARROW_POINTS     6		/* number of points in an arrowhead */
#define NUM_STATIC_POINTS    256	/* number of static points to use to avoid allocating arrays */

#define GROW_BOUNDS(bx1, by1, bx2, by2, x, y) {	\
	if (x < bx1)				\
		bx1 = x;			\
						\
	if (x > bx2)				\
		bx2 = x;			\
						\
	if (y < by1)				\
		by1 = y;			\
						\
	if (y > by2)				\
		by2 = y;			\
}

enum {
	PROP_0,
	PROP_POINTS,
	PROP_FILL_COLOR,
	PROP_FILL_COLOR_RGBA,
	PROP_WIDTH_PIXELS,
	PROP_WIDTH_UNITS,
	PROP_CAP_STYLE,
	PROP_JOIN_STYLE,
	PROP_FIRST_ARROWHEAD,
	PROP_LAST_ARROWHEAD,
	PROP_SMOOTH,
	PROP_SPLINE_STEPS,
	PROP_ARROW_SHAPE_A,
	PROP_ARROW_SHAPE_B,
	PROP_ARROW_SHAPE_C
};

static void gnome_canvas_line_class_init   (GnomeCanvasLineClass *class);
static void gnome_canvas_line_init         (GnomeCanvasLine      *line);
static void gnome_canvas_line_dispose      (GnomeCanvasItem      *object);
static void gnome_canvas_line_set_property (GObject              *object,
					    guint                 param_id,
					    const GValue         *value,
					    GParamSpec           *pspec);
static void gnome_canvas_line_get_property (GObject              *object,
					    guint                 param_id,
					    GValue               *value,
					    GParamSpec           *pspec);

static void   gnome_canvas_line_update      (GnomeCanvasItem *item, const cairo_matrix_t *i2c, gint flags);
static void   gnome_canvas_line_draw        (GnomeCanvasItem *item, cairo_t *cr,
					     gint x, gint y, gint width, gint height);
static GnomeCanvasItem *gnome_canvas_line_point  (GnomeCanvasItem *item, gdouble x, gdouble y,
					     gint cx, gint cy);
static void   gnome_canvas_line_bounds      (GnomeCanvasItem *item, gdouble *x1, gdouble *y1, gdouble *x2, gdouble *y2);

static GnomeCanvasItemClass *parent_class;

GType
gnome_canvas_line_get_type (void)
{
	static GType line_type;

	if (!line_type) {
		const GTypeInfo object_info = {
			sizeof (GnomeCanvasLineClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) gnome_canvas_line_class_init,
			(GClassFinalizeFunc) NULL,
			NULL,			/* class_data */
			sizeof (GnomeCanvasLine),
			0,			/* n_preallocs */
			(GInstanceInitFunc) gnome_canvas_line_init,
			NULL			/* value_table */
		};

		line_type = g_type_register_static (GNOME_TYPE_CANVAS_ITEM, "GnomeCanvasLine",
						    &object_info, 0);
	}

	return line_type;
}

static void
gnome_canvas_line_class_init (GnomeCanvasLineClass *class)
{
	GObjectClass *gobject_class;
	GnomeCanvasItemClass *item_class;

	gobject_class = (GObjectClass *) class;
	item_class = (GnomeCanvasItemClass *) class;

	parent_class = g_type_class_peek_parent (class);

	gobject_class->set_property = gnome_canvas_line_set_property;
	gobject_class->get_property = gnome_canvas_line_get_property;

        g_object_class_install_property
                (gobject_class,
                 PROP_POINTS,
                 g_param_spec_boxed ("points", NULL, NULL,
				     GNOME_TYPE_CANVAS_POINTS,
				     (G_PARAM_READABLE | G_PARAM_WRITABLE)));
        g_object_class_install_property
                (gobject_class,
                 PROP_FILL_COLOR,
                 g_param_spec_string ("fill_color", NULL, NULL,
                                      NULL,
                                      (G_PARAM_READABLE | G_PARAM_WRITABLE)));
        g_object_class_install_property
                (gobject_class,
                 PROP_FILL_COLOR_RGBA,
                 g_param_spec_uint ("fill_color_rgba", NULL, NULL,
				    0, G_MAXUINT, 0,
				    (G_PARAM_READABLE | G_PARAM_WRITABLE)));
        g_object_class_install_property
                (gobject_class,
                 PROP_WIDTH_PIXELS,
                 g_param_spec_uint ("width_pixels", NULL, NULL,
				    0, G_MAXUINT, 0,
				    (G_PARAM_READABLE | G_PARAM_WRITABLE)));
        g_object_class_install_property
                (gobject_class,
                 PROP_WIDTH_UNITS,
                 g_param_spec_double ("width_units", NULL, NULL,
				      0.0, G_MAXDOUBLE, 0.0,
				      (G_PARAM_READABLE | G_PARAM_WRITABLE)));
        g_object_class_install_property
                (gobject_class,
                 PROP_CAP_STYLE,
                 g_param_spec_enum ("cap_style", NULL, NULL,
                                    CAIRO_GOBJECT_TYPE_LINE_CAP,
                                    CAIRO_LINE_CAP_BUTT,
                                    (G_PARAM_READABLE | G_PARAM_WRITABLE)));
        g_object_class_install_property
                (gobject_class,
                 PROP_JOIN_STYLE,
                 g_param_spec_enum ("join_style", NULL, NULL,
                                    CAIRO_GOBJECT_TYPE_LINE_JOIN,
                                    CAIRO_LINE_JOIN_MITER,
                                    (G_PARAM_READABLE | G_PARAM_WRITABLE)));
        g_object_class_install_property
                (gobject_class,
                 PROP_FIRST_ARROWHEAD,
                 g_param_spec_boolean ("first_arrowhead", NULL, NULL,
				       FALSE,
				       (G_PARAM_READABLE | G_PARAM_WRITABLE)));
        g_object_class_install_property
                (gobject_class,
                 PROP_LAST_ARROWHEAD,
                 g_param_spec_boolean ("last_arrowhead", NULL, NULL,
				       FALSE,
				       (G_PARAM_READABLE | G_PARAM_WRITABLE)));
        g_object_class_install_property
                (gobject_class,
                 PROP_SMOOTH,
                 g_param_spec_boolean ("smooth", NULL, NULL,
				       FALSE,
				       (G_PARAM_READABLE | G_PARAM_WRITABLE)));
        g_object_class_install_property
                (gobject_class,
                 PROP_SPLINE_STEPS,
                 g_param_spec_uint ("spline_steps", NULL, NULL,
				    0, G_MAXUINT, DEFAULT_SPLINE_STEPS,
				    (G_PARAM_READABLE | G_PARAM_WRITABLE)));
        g_object_class_install_property
                (gobject_class,
                 PROP_ARROW_SHAPE_A,
                 g_param_spec_double ("arrow_shape_a", NULL, NULL,
				      -G_MAXDOUBLE, G_MAXDOUBLE, 0,
				      (G_PARAM_READABLE | G_PARAM_WRITABLE)));
        g_object_class_install_property
                (gobject_class,
                 PROP_ARROW_SHAPE_B,
                 g_param_spec_double ("arrow_shape_b", NULL, NULL,
				      -G_MAXDOUBLE, G_MAXDOUBLE, 0,
				      (G_PARAM_READABLE | G_PARAM_WRITABLE)));
        g_object_class_install_property
                (gobject_class,
                 PROP_ARROW_SHAPE_C,
                 g_param_spec_double ("arrow_shape_c", NULL, NULL,
				      -G_MAXDOUBLE, G_MAXDOUBLE, 0,
				      (G_PARAM_READABLE | G_PARAM_WRITABLE)));

	item_class->dispose = gnome_canvas_line_dispose;
	item_class->update = gnome_canvas_line_update;
	item_class->draw = gnome_canvas_line_draw;
	item_class->point = gnome_canvas_line_point;
	item_class->bounds = gnome_canvas_line_bounds;
}

static void
gnome_canvas_line_init (GnomeCanvasLine *line)
{
	line->width = 0.0;
	line->cap = CAIRO_LINE_CAP_BUTT;
	line->join = CAIRO_LINE_JOIN_MITER;
	line->shape_a = 0.0;
	line->shape_b = 0.0;
	line->shape_c = 0.0;
	line->spline_steps = DEFAULT_SPLINE_STEPS;
}

static void
gnome_canvas_line_dispose (GnomeCanvasItem *object)
{
	GnomeCanvasLine *line;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GNOME_IS_CANVAS_LINE (object));

	line = GNOME_CANVAS_LINE (object);

	/* remember, dispose can be run multiple times! */

	if (line->coords)
		g_free (line->coords);
	line->coords = NULL;

	if (line->first_coords)
		g_free (line->first_coords);
	line->first_coords = NULL;

	if (line->last_coords)
		g_free (line->last_coords);
	line->last_coords = NULL;

	if (GNOME_CANVAS_ITEM_CLASS (parent_class)->dispose)
		GNOME_CANVAS_ITEM_CLASS (parent_class)->dispose (object);
}

/* Computes the bounding box of the line, including its arrow points.  Assumes that the number of
 * points in the line is not zero.
 */
static void
get_bounds (GnomeCanvasLine *line, gdouble *bx1, gdouble *by1, gdouble *bx2, gdouble *by2)
{
	gdouble *coords;
	gdouble x1, y1, x2, y2;
	gdouble width;
	gint i;

	if (!line->coords) {
	    *bx1 = *by1 = *bx2 = *by2 = 0.0;
	    return;
	}

	/* Find bounding box of line's points */

	x1 = x2 = line->coords[0];
	y1 = y2 = line->coords[1];

	for (i = 1, coords = line->coords + 2; i < line->num_points; i++, coords += 2)
		GROW_BOUNDS (x1, y1, x2, y2, coords[0], coords[1]);

	/* Add possible over-estimate for wide lines */

	width = line->width;

	x1 -= width;
	y1 -= width;
	x2 += width;
	y2 += width;

	/* For mitered lines, make a second pass through all the points.  Compute the location of
	 * the two miter vertex points and add them to the bounding box.
	 */

	if (line->join == CAIRO_LINE_JOIN_MITER)
		for (i = line->num_points, coords = line->coords; i >= 3; i--, coords += 2) {
			gdouble mx1, my1, mx2, my2;

			if (gnome_canvas_get_miter_points (coords[0], coords[1],
							   coords[2], coords[3],
							   coords[4], coords[5],
							   width,
							   &mx1, &my1, &mx2, &my2)) {
				GROW_BOUNDS (x1, y1, x2, y2, mx1, my1);
				GROW_BOUNDS (x1, y1, x2, y2, mx2, my2);
			}
		}

	/* Add the arrow points, if any */

	if (line->first_arrow && line->first_coords)
		for (i = 0, coords = line->first_coords; i < NUM_ARROW_POINTS; i++, coords += 2)
			GROW_BOUNDS (x1, y1, x2, y2, coords[0], coords[1]);

	if (line->last_arrow && line->last_coords)
		for (i = 0, coords = line->last_coords; i < NUM_ARROW_POINTS; i++, coords += 2)
			GROW_BOUNDS (x1, y1, x2, y2, coords[0], coords[1]);

	/* Done */

	*bx1 = x1;
	*by1 = y1;
	*bx2 = x2;
	*by2 = y2;
}

/* Computes the bounding box of the line, in canvas coordinates.  Assumes that the number of points in the polygon is
 * not zero. Affine is the i2c transformation.
 */
static void
get_bounds_canvas (GnomeCanvasLine      *line,
                   gdouble              *bx1,
                   gdouble              *by1,
                   gdouble              *bx2,
                   gdouble              *by2,
                   const cairo_matrix_t *i2c)
{
	/* It would be possible to tighten the bounds somewhat by transforming the individual points before
	   aggregating them into the bbox. But it hardly seems worth it. */
	gdouble x1, x2, y1, y2;

	get_bounds (line, &x1, &y1, &x2, &y2);
	gnome_canvas_matrix_transform_rect (i2c, &x1, &y1, &x2, &y2);

	/* include 1 pixel of fudge */
	*bx1 = floor (x1) - 1;
	*by1 = floor (y1) - 1;
	*bx2 = ceil (x2) + 1;
	*by2 = ceil (y2) + 1;
}

/* Recalculates the arrow polygons for the line */
static void
reconfigure_arrows (GnomeCanvasLine *line)
{
	gdouble *poly, *coords;
	gdouble dx, dy, length;
	gdouble sin_theta, cos_theta, tmp;
	gdouble frac_height;	/* Line width as fraction of arrowhead width */
	gdouble backup;		/* Distance to backup end points so the line ends in the middle of the arrowhead */
	gdouble vx, vy;		/* Position of arrowhead vertex */
	gdouble shape_a, shape_b, shape_c;
	gdouble width;
	gint i;

	if (line->num_points == 0)
		return;

	/* Set up things */

	if (line->first_arrow) {
		if (line->first_coords) {
			line->coords[0] = line->first_coords[0];
			line->coords[1] = line->first_coords[1];
		} else
			line->first_coords = g_new (double, 2 * NUM_ARROW_POINTS);
	} else if (line->first_coords) {
		line->coords[0] = line->first_coords[0];
		line->coords[1] = line->first_coords[1];

		g_free (line->first_coords);
		line->first_coords = NULL;
	}

	i = 2 * (line->num_points - 1);

	if (line->last_arrow) {
		if (line->last_coords) {
			line->coords[i] = line->last_coords[0];
			line->coords[i + 1] = line->last_coords[1];
		} else
			line->last_coords = g_new (double, 2 * NUM_ARROW_POINTS);
	} else if (line->last_coords) {
		line->coords[i] = line->last_coords[0];
		line->coords[i + 1] = line->last_coords[1];

		g_free (line->last_coords);
		line->last_coords = NULL;
	}

	if (!line->first_arrow && !line->last_arrow)
		return;

	width = line->width;

	/* Add fudge value for better-looking results */

	shape_a = line->shape_a;
	shape_b = line->shape_b;
	shape_c = line->shape_c + width / 2.0;

	shape_a += 0.001;
	shape_b += 0.001;
	shape_c += 0.001;

	/* Compute the polygon for the first arrowhead and adjust the first point in the line so
	 * that the line does not stick out past the leading edge of the arrowhead.
	 */

	frac_height = (line->width / 2.0) / shape_c;
	backup = frac_height * shape_b + shape_a * (1.0 - frac_height) / 2.0;

	if (line->first_arrow) {
		poly = line->first_coords;
		poly[0] = poly[10] = line->coords[0];
		poly[1] = poly[11] = line->coords[1];

		dx = poly[0] - line->coords[2];
		dy = poly[1] - line->coords[3];
		length = sqrt (dx * dx + dy * dy);
		if (length < GNOME_CANVAS_EPSILON)
			sin_theta = cos_theta = 0.0;
		else {
			sin_theta = dy / length;
			cos_theta = dx / length;
		}

		vx = poly[0] - shape_a * cos_theta;
		vy = poly[1] - shape_a * sin_theta;

		tmp = shape_c * sin_theta;

		poly[2] = poly[0] - shape_b * cos_theta + tmp;
		poly[8] = poly[2] - 2.0 * tmp;

		tmp = shape_c * cos_theta;

		poly[3] = poly[1] - shape_b * sin_theta - tmp;
		poly[9] = poly[3] + 2.0 * tmp;

		poly[4] = poly[2] * frac_height + vx * (1.0 - frac_height);
		poly[5] = poly[3] * frac_height + vy * (1.0 - frac_height);
		poly[6] = poly[8] * frac_height + vx * (1.0 - frac_height);
		poly[7] = poly[9] * frac_height + vy * (1.0 - frac_height);

		/* Move the first point towards the second so that the corners at the end of the
		 * line are inside the arrowhead.
		 */

		line->coords[0] = poly[0] - backup * cos_theta;
		line->coords[1] = poly[1] - backup * sin_theta;
	}

	/* Same process for last arrowhead */

	if (line->last_arrow) {
		coords = line->coords + 2 * (line->num_points - 2);
		poly = line->last_coords;
		poly[0] = poly[10] = coords[2];
		poly[1] = poly[11] = coords[3];

		dx = poly[0] - coords[0];
		dy = poly[1] - coords[1];
		length = sqrt (dx * dx + dy * dy);
		if (length < GNOME_CANVAS_EPSILON)
			sin_theta = cos_theta = 0.0;
		else {
			sin_theta = dy / length;
			cos_theta = dx / length;
		}

		vx = poly[0] - shape_a * cos_theta;
		vy = poly[1] - shape_a * sin_theta;

		tmp = shape_c * sin_theta;

		poly[2] = poly[0] - shape_b * cos_theta + tmp;
		poly[8] = poly[2] - 2.0 * tmp;

		tmp = shape_c * cos_theta;

		poly[3] = poly[1] - shape_b * sin_theta - tmp;
		poly[9] = poly[3] + 2.0 * tmp;

		poly[4] = poly[2] * frac_height + vx * (1.0 - frac_height);
		poly[5] = poly[3] * frac_height + vy * (1.0 - frac_height);
		poly[6] = poly[8] * frac_height + vx * (1.0 - frac_height);
		poly[7] = poly[9] * frac_height + vy * (1.0 - frac_height);

		coords[2] = poly[0] - backup * cos_theta;
		coords[3] = poly[1] - backup * sin_theta;
	}
}

static void
gnome_canvas_line_set_property (GObject              *object,
				guint                 param_id,
				const GValue         *value,
				GParamSpec           *pspec)
{
	GnomeCanvasItem *item;
	GnomeCanvasLine *line;
	GnomeCanvasPoints *points;
	GdkColor color = { 0, 0, 0, 0, };
	gboolean color_changed;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GNOME_IS_CANVAS_LINE (object));

	item = GNOME_CANVAS_ITEM (object);
	line = GNOME_CANVAS_LINE (object);

	color_changed = FALSE;

	switch (param_id) {
	case PROP_POINTS:
		points = g_value_get_boxed (value);

		if (line->coords) {
			g_free (line->coords);
			line->coords = NULL;
		}

		if (!points)
			line->num_points = 0;
		else {
			line->num_points = points->num_points;
			line->coords = g_new (double, 2 * line->num_points);
			memcpy (line->coords, points->coords, 2 * line->num_points * sizeof (gdouble));
		}

		/* Drop the arrowhead polygons if they exist -- they will be regenerated */

		if (line->first_coords) {
			g_free (line->first_coords);
			line->first_coords = NULL;
		}

		if (line->last_coords) {
			g_free (line->last_coords);
			line->last_coords = NULL;
		}

		/* Since the line's points have changed, we need to re-generate arrowheads in
		 * addition to recalculating the bounds.
		 */
		gnome_canvas_item_request_update (item);
		break;

	case PROP_FILL_COLOR:
		if (g_value_get_string (value))
			gdk_color_parse (g_value_get_string (value), &color);
		line->fill_rgba = ((color.red & 0xff00) << 16 |
				   (color.green & 0xff00) << 8 |
				   (color.blue & 0xff00) |
				   0xff);
		color_changed = TRUE;
		break;

	case PROP_FILL_COLOR_RGBA:
		line->fill_rgba = g_value_get_uint (value);
		color_changed = TRUE;
		break;

	case PROP_WIDTH_PIXELS:
		line->width = g_value_get_uint (value);
		line->width_pixels = TRUE;
		gnome_canvas_item_request_update (item);
		break;

	case PROP_WIDTH_UNITS:
		line->width = fabs (g_value_get_double (value));
		line->width_pixels = FALSE;
		gnome_canvas_item_request_update (item);
		break;

	case PROP_CAP_STYLE:
		line->cap = g_value_get_enum (value);
		gnome_canvas_item_request_update (item);
		break;

	case PROP_JOIN_STYLE:
		line->join = g_value_get_enum (value);
		gnome_canvas_item_request_update (item);
		break;

	case PROP_FIRST_ARROWHEAD:
		line->first_arrow = g_value_get_boolean (value);
		gnome_canvas_item_request_update (item);
		break;

	case PROP_LAST_ARROWHEAD:
		line->last_arrow = g_value_get_boolean (value);
		gnome_canvas_item_request_update (item);
		break;

	case PROP_SMOOTH:
		/* FIXME */
		break;

	case PROP_SPLINE_STEPS:
		/* FIXME */
		break;

	case PROP_ARROW_SHAPE_A:
		line->shape_a = fabs (g_value_get_double (value));
		gnome_canvas_item_request_update (item);
		break;

	case PROP_ARROW_SHAPE_B:
		line->shape_b = fabs (g_value_get_double (value));
		gnome_canvas_item_request_update (item);
		break;

	case PROP_ARROW_SHAPE_C:
		line->shape_c = fabs (g_value_get_double (value));
		gnome_canvas_item_request_update (item);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;
	}

	if (color_changed)
		gnome_canvas_request_redraw (item->canvas, item->x1, item->y1, item->x2, item->y2);
}

/* Returns a copy of the line's points without the endpoint adjustments for
 * arrowheads.
 */
static GnomeCanvasPoints *
get_points (GnomeCanvasLine *line)
{
	GnomeCanvasPoints *points;
	gint start_ofs, end_ofs;

	if (line->num_points == 0)
		return NULL;

	start_ofs = end_ofs = 0;

	points = gnome_canvas_points_new (line->num_points);

	/* Invariant:  if first_coords or last_coords exist, then the line's
	 * endpoints have been adjusted.
	 */

	if (line->first_coords) {
		start_ofs = 1;

		points->coords[0] = line->first_coords[0];
		points->coords[1] = line->first_coords[1];
	}

	if (line->last_coords) {
		end_ofs = 1;

		points->coords[2 * (line->num_points - 1)] = line->last_coords[0];
		points->coords[2 * (line->num_points - 1) + 1] = line->last_coords[1];
	}

	memcpy (points->coords + 2 * start_ofs,
		line->coords + 2 * start_ofs,
		2 * (line->num_points - (start_ofs + end_ofs)) * sizeof (gdouble));

	return points;
}

static void
gnome_canvas_line_get_property (GObject              *object,
				guint                 param_id,
				GValue               *value,
				GParamSpec           *pspec)
{
	GnomeCanvasLine *line;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GNOME_IS_CANVAS_LINE (object));

	line = GNOME_CANVAS_LINE (object);

	switch (param_id) {
	case PROP_POINTS:
		/* get_points returns a copy */
		g_value_take_boxed (value, get_points (line));
		break;

	case PROP_FILL_COLOR:
		g_value_take_string (value,
				     g_strdup_printf ("#%02x%02x%02x",
				     line->fill_rgba >> 24,
				     (line->fill_rgba >> 16) & 0xff,
				     (line->fill_rgba >> 8) & 0xff));
		break;

	case PROP_FILL_COLOR_RGBA:
		g_value_set_uint (value, line->fill_rgba);
		break;

	case PROP_WIDTH_PIXELS:
		g_value_set_uint (value, line->width);
		break;

	case PROP_WIDTH_UNITS:
		g_value_set_double (value, line->width);
		break;

	case PROP_CAP_STYLE:
		g_value_set_enum (value, line->cap);
		break;

	case PROP_JOIN_STYLE:
		g_value_set_enum (value, line->join);
		break;

	case PROP_FIRST_ARROWHEAD:
		g_value_set_boolean (value, line->first_arrow);
		break;

	case PROP_LAST_ARROWHEAD:
		g_value_set_boolean (value, line->last_arrow);
		break;

	case PROP_SMOOTH:
		g_value_set_boolean (value, line->smooth);
		break;

	case PROP_SPLINE_STEPS:
		g_value_set_uint (value, line->spline_steps);
		break;

	case PROP_ARROW_SHAPE_A:
		g_value_set_double (value, line->shape_a);
		break;

	case PROP_ARROW_SHAPE_B:
		g_value_set_double (value, line->shape_b);
		break;

	case PROP_ARROW_SHAPE_C:
		g_value_set_double (value, line->shape_c);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;
	}
}

static void
gnome_canvas_line_update (GnomeCanvasItem *item, const cairo_matrix_t *i2c, gint flags)
{
	GnomeCanvasLine *line;
	gdouble x1, y1, x2, y2;

	line = GNOME_CANVAS_LINE (item);

	if (parent_class->update)
		(* parent_class->update) (item, i2c, flags);

	reconfigure_arrows (line);

	get_bounds_canvas (line, &x1, &y1, &x2, &y2, i2c);
	gnome_canvas_update_bbox (item, x1, y1, x2, y2);
}

static void
item_to_canvas (GnomeCanvas          *canvas,
                gdouble              *item_coords,
                GdkPoint             *canvas_coords,
                gint                  num_points,
                gint                 *num_drawn_points,
                const cairo_matrix_t *matrix,
                gint                  x,
                gint                  y)
{
	gint i;
	gint old_cx, old_cy;
	gint cx, cy;
	double px, py;

	/* the first point is always drawn */

	px = item_coords[0];
	py = item_coords[1];
	cairo_matrix_transform_point (matrix, &px, &py);
	cx = floor (px + 0.5);
	cy = floor (py + 0.5);
	canvas_coords->x = cx - x;
	canvas_coords->y = cy - y;
	canvas_coords++;
	old_cx = cx;
	old_cy = cy;
	*num_drawn_points = 1;

	for (i = 1; i < num_points; i++) {
		px = item_coords[i * 2];
		py = item_coords[i * 2 + 1];
		cairo_matrix_transform_point (matrix, &px, &py);
		cx = floor (px + 0.5);
		cy = floor (py + 0.5);
		if (old_cx != cx || old_cy != cy) {
			canvas_coords->x = cx - x;
			canvas_coords->y = cy - y;
			old_cx = cx;
			old_cy = cy;
			canvas_coords++;
			(*num_drawn_points)++;
		}
	}
}

static void
gnome_canvas_line_draw (GnomeCanvasItem *item, cairo_t *cr,
			gint x, gint y, gint width, gint height)
{
	GnomeCanvasLine *line;
	GdkPoint static_points[NUM_STATIC_POINTS];
	GdkPoint *points;
	gint actual_num_points_drawn;
	cairo_matrix_t matrix;
        int i;

	line = GNOME_CANVAS_LINE (item);

	if (line->num_points == 0)
		return;

        cairo_save (cr);

        /* points are always centered */
        cairo_translate (cr, 0.5, 0.5);

        cairo_set_source_rgba (cr,
                               ((line->fill_rgba >> 24) & 0xff) / 255.0,
                               ((line->fill_rgba >> 16) & 0xff) / 255.0,
                               ((line->fill_rgba >>  8) & 0xff) / 255.0,
                               ((line->fill_rgba >>  0) & 0xff) / 255.0);
	if (line->width_pixels)
                cairo_set_line_width (cr, (gint) line->width);
        else
		cairo_set_line_width (cr, (gint) (line->width + 0.5));
        if (line->first_arrow || line->last_arrow)
                cairo_set_line_cap (cr, CAIRO_LINE_CAP_BUTT);
        else
                cairo_set_line_cap (cr, line->cap);
        cairo_set_line_join (cr, line->join);

        gnome_canvas_item_i2c_matrix (item, &matrix);
	/* Build array of canvas pixel coordinates */

	if (line->num_points <= NUM_STATIC_POINTS)
		points = static_points;
	else
		points = g_new (GdkPoint, line->num_points);

	item_to_canvas (item->canvas, line->coords, points, line->num_points,
			&actual_num_points_drawn, &matrix, x, y);

        cairo_move_to (cr, points[0].x, points[0].y);
        for (i = 1; i < actual_num_points_drawn; i++)
                cairo_line_to (cr, points[i].x, points[i].y);
        cairo_stroke (cr);

	if (points != static_points)
		g_free (points);

	/* Draw arrowheads */

	points = static_points;

	if (line->first_arrow) {
		item_to_canvas (item->canvas, line->first_coords, points, NUM_ARROW_POINTS,
				&actual_num_points_drawn, &matrix, x, y);
                cairo_move_to (cr, points[0].x, points[0].y);
                for (i = 1; i < actual_num_points_drawn; i++)
                        cairo_line_to (cr, points[i].x, points[i].y);
                cairo_fill (cr);
	}

	if (line->last_arrow) {
		item_to_canvas (item->canvas, line->last_coords, points, NUM_ARROW_POINTS,
				&actual_num_points_drawn, &matrix, x, y);
                cairo_move_to (cr, points[0].x, points[0].y);
                for (i = 1; i < actual_num_points_drawn; i++)
                        cairo_line_to (cr, points[i].x, points[i].y);
                cairo_fill (cr);
	}

  cairo_restore (cr);
}

static GnomeCanvasItem *
gnome_canvas_line_point (GnomeCanvasItem *item, gdouble x, gdouble y,
			 gint cx, gint cy)
{
	GnomeCanvasLine *line;
	gdouble *line_points = NULL, *coords;
	gdouble static_points[2 * NUM_STATIC_POINTS];
	gdouble poly[10];
	gdouble best, dist;
	gdouble dx, dy;
	gdouble width;
	gint num_points = 0, i;
	gint changed_miter_to_bevel;

#ifdef VERBOSE
	g_print ("gnome_canvas_line_point x, y = (%g, %g); cx, cy = (%d, %d)\n", x, y, cx, cy);
#endif

	line = GNOME_CANVAS_LINE (item);

	best = 1.0e36;

	/* Handle smoothed lines by generating an expanded set ot points */

	if (line->smooth && (line->num_points > 2)) {
		/* FIXME */
	} else {
		num_points = line->num_points;
		line_points = line->coords;
	}

	/* Compute a polygon for each edge of the line and test the point against it.  The effective
	 * width of the line is adjusted so that it will be at least one pixel thick (so that zero
	 * pixel-wide lines can be pickedup as well).
	 */

	width = MAX(line->width, 1.0);

	changed_miter_to_bevel = 0;

	for (i = num_points, coords = line_points; i >= 2; i--, coords += 2) {
		/* If rounding is done around the first point, then compute distance between the
		 * point and the first point.
		 */

		if (((line->cap == CAIRO_LINE_CAP_ROUND) && (i == num_points))
		    || ((line->join == CAIRO_LINE_JOIN_ROUND) && (i != num_points))) {
			dx = coords[0] - x;
			dy = coords[1] - y;
			dist = sqrt (dx * dx + dy * dy) - width / 2.0;
			if (dist < GNOME_CANVAS_EPSILON) {
				best = 0.0;
				goto done;
			} else if (dist < best)
				best = dist;
		}

		/* Compute the polygonal shape corresponding to this edge, with two points for the
		 * first point of the edge and two points for the last point of the edge.
		 */

		if (i == num_points)
			gnome_canvas_get_butt_points (coords[2], coords[3], coords[0], coords[1],
						      width, (line->cap == CAIRO_LINE_CAP_SQUARE),
						      poly, poly + 1, poly + 2, poly + 3);
		else if ((line->join == CAIRO_LINE_JOIN_MITER) && !changed_miter_to_bevel) {
			poly[0] = poly[6];
			poly[1] = poly[7];
			poly[2] = poly[4];
			poly[3] = poly[5];
		} else {
			gnome_canvas_get_butt_points (coords[2], coords[3], coords[0], coords[1],
						      width, FALSE,
						      poly, poly + 1, poly + 2, poly + 3);

			/* If this line uses beveled joints, then check the distance to a polygon
			 * comprising the last two points of the previous polygon and the first two
			 * from this polygon; this checks the wedges that fill the mitered point.
			 */

			if ((line->join == CAIRO_LINE_JOIN_BEVEL) || changed_miter_to_bevel) {
				poly[8] = poly[0];
				poly[9] = poly[1];

				dist = gnome_canvas_polygon_to_point (poly, 5, x, y);
				if (dist < GNOME_CANVAS_EPSILON) {
					best = 0.0;
					goto done;
				} else if (dist < best)
					best = dist;

				changed_miter_to_bevel = FALSE;
			}
		}

		if (i == 2)
			gnome_canvas_get_butt_points (coords[0], coords[1], coords[2], coords[3],
						      width, (line->cap == CAIRO_LINE_CAP_SQUARE),
						      poly + 4, poly + 5, poly + 6, poly + 7);
		else if (line->join == CAIRO_LINE_JOIN_MITER) {
			if (!gnome_canvas_get_miter_points (coords[0], coords[1],
							    coords[2], coords[3],
							    coords[4], coords[5],
							    width,
							    poly + 4, poly + 5, poly + 6, poly + 7)) {
				changed_miter_to_bevel = TRUE;
				gnome_canvas_get_butt_points (coords[0], coords[1], coords[2], coords[3],
							      width, FALSE,
							      poly + 4, poly + 5, poly + 6, poly + 7);
			}
		} else
			gnome_canvas_get_butt_points (coords[0], coords[1], coords[2], coords[3],
						      width, FALSE,
						      poly + 4, poly + 5, poly + 6, poly + 7);

		poly[8] = poly[0];
		poly[9] = poly[1];

		dist = gnome_canvas_polygon_to_point (poly, 5, x, y);
		if (dist < GNOME_CANVAS_EPSILON) {
			best = 0.0;
			goto done;
		} else if (dist < best)
			best = dist;
	}

	/* If caps are rounded, check the distance to the cap around the final end point of the line */

	if (line->cap == CAIRO_LINE_CAP_ROUND) {
		dx = coords[0] - x;
		dy = coords[1] - y;
		dist = sqrt (dx * dx + dy * dy) - width / 2.0;
		if (dist < GNOME_CANVAS_EPSILON) {
			best = 0.0;
			goto done;
		} else
			best = dist;
	}

	/* sometimes the GnomeCanvasItem::update signal will not have
           been processed between deleting the arrow points and a call
           to this routine -- this can cause a segfault here */
	if ((line->first_arrow && !line->first_coords) ||
	    (line->last_arrow && !line->last_coords))
		reconfigure_arrows (line);

	/* If there are arrowheads, check the distance to them */

	if (line->first_arrow) {
		dist = gnome_canvas_polygon_to_point (line->first_coords, NUM_ARROW_POINTS, x, y);
		if (dist < GNOME_CANVAS_EPSILON) {
			best = 0.0;
			goto done;
		} else
			best = dist;
	}

	if (line->last_arrow) {
		dist = gnome_canvas_polygon_to_point (line->last_coords, NUM_ARROW_POINTS, x, y);
		if (dist < GNOME_CANVAS_EPSILON) {
			best = 0.0;
			goto done;
		} else
			best = dist;
	}

done:

	if ((line_points != static_points) && (line_points != line->coords))
		g_free (line_points);

	return best == 0.0 ? item : NULL;
}

static void
gnome_canvas_line_bounds (GnomeCanvasItem *item, gdouble *x1, gdouble *y1, gdouble *x2, gdouble *y2)
{
	GnomeCanvasLine *line;

	line = GNOME_CANVAS_LINE (item);

	if (line->num_points == 0) {
		*x1 = *y1 = *x2 = *y2 = 0.0;
		return;
	}

	get_bounds (line, x1, y1, x2, y2);
}
