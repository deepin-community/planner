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
/* Miscellaneous utility functions for the GnomeCanvas widget
 *
 * GnomeCanvas is basically a port of the Tk toolkit's most excellent canvas widget.  Tk is
 * copyrighted by the Regents of the University of California, Sun Microsystems, and other parties.
 *
 * Author: Federico Mena <federico@nuclecu.unam.mx>
 */

#ifndef GNOME_CANVAS_UTIL_H
#define GNOME_CANVAS_UTIL_H

#include <libgnomecanvas/gnome-canvas.h>

G_BEGIN_DECLS

typedef struct _GnomeCanvasPoints GnomeCanvasPoints;

/* This structure defines an array of points.  X coordinates are stored in the even-numbered
 * indices, and Y coordinates are stored in the odd-numbered indices.  num_points indicates the
 * number of points, so the array is 2*num_points elements big.
 */
struct _GnomeCanvasPoints {
	gdouble *coords;
	gint num_points;
	gint ref_count;
};

/* Allocate a new GnomeCanvasPoints structure with enough space for the specified number of points */
GnomeCanvasPoints *gnome_canvas_points_new (gint num_points);

/* Increate ref count */
GnomeCanvasPoints *gnome_canvas_points_ref (GnomeCanvasPoints *points);
#define gnome_canvas_points_unref gnome_canvas_points_free

/* Decrease ref count and free structure if it has reached zero */
void gnome_canvas_points_free (GnomeCanvasPoints *points);

/* Given three points forming an angle, compute the coordinates of the inside and outside points of
 * the mitered corner formed by a line of a given width at that angle.
 *
 * If the angle is less than 11 degrees, then FALSE is returned and the return points are not
 * modified.  Otherwise, TRUE is returned.
 */
gint gnome_canvas_get_miter_points (gdouble x1, gdouble y1, gdouble x2, gdouble y2, gdouble x3, gdouble y3,
				   gdouble width,
				   gdouble *mx1, gdouble *my1, gdouble *mx2, gdouble *my2);

/* Compute the butt points of a line segment.  If project is FALSE, then the results are as follows:
 *
 *            -------------------* (bx1, by1)
 *                               |
 *   (x1, y1) *------------------* (x2, y2)
 *                               |
 *            -------------------* (bx2, by2)
 *
 * that is, the line is not projected beyond (x2, y2).  If project is TRUE, then the results are as
 * follows:
 *
 *            -------------------* (bx1, by1)
 *                      (x2, y2) |
 *   (x1, y1) *-------------*    |
 *                               |
 *            -------------------* (bx2, by2)
 */
void gnome_canvas_get_butt_points (gdouble x1, gdouble y1, gdouble x2, gdouble y2,
				   gdouble width, gint project,
				   gdouble *bx1, gdouble *by1, gdouble *bx2, gdouble *by2);

/* Calculate the distance from a polygon to a point.  The polygon's X coordinates are in the even
 * indices of the poly array, and the Y coordinates are in the odd indices.
 */
gdouble gnome_canvas_polygon_to_point (gdouble *poly, gint num_points, gdouble x, gdouble y);

/* Sets the svp to the new value, requesting repaint on what's changed. This
 * function takes responsibility for freeing new_svp. This routine also adds the
 * svp's bbox to the item's.
 */
void gnome_canvas_item_reset_bounds (GnomeCanvasItem *item);

/* Sets the bbox to the new value, requesting full repaint. */
void gnome_canvas_update_bbox (GnomeCanvasItem *item, gint x1, gint y1, gint x2, gint y2);

/* Create a scratch cairo_t for measuring purposes */
cairo_t *gnome_canvas_cairo_create_scratch (void);

void gnome_canvas_matrix_transform_rect (const cairo_matrix_t *matrix, double *x1, double *y1, double *x2, double *y2);

G_END_DECLS

#endif
