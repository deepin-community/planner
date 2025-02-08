/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2001-2002 CodeFactory AB
 * Copyright (C) 2001-2002 Richard Hult <richard@imendio.com>
 * Copyright (C) 2001-2002 Mikael Hallendal <micke@imendio.com>
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

#pragma once

#include <gtk/gtk.h>
#include <libgnomecanvas/gnome-canvas.h>
#include <libgnomecanvas/gnome-canvas-util.h>

#define PLANNER_TYPE_GANTT_ROW            (planner_gantt_row_get_type ())
#define PLANNER_GANTT_ROW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PLANNER_TYPE_GANTT_ROW, PlannerGanttRow))
#define PLANNER_GANTT_ROW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PLANNER_TYPE_GANTT_ROW, PlannerGanttRowClass))
#define PLANNER_IS_GANTT_ROW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PLANNER_TYPE_GANTT_ROW))
#define PLANNER_IS_GANTT_ROW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PLANNER_TYPE_GANTT_ROW))
#define PLANNER_GANTT_ROW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PLANNER_TYPE_GANTT_ROW, PlannerGanttRowClass))


typedef struct _PlannerGanttRow      PlannerGanttRow;
typedef struct _PlannerGanttRowClass PlannerGanttRowClass;
typedef struct _PlannerGanttRowPriv  PlannerGanttRowPriv;

struct _PlannerGanttRow {
	GnomeCanvasItem  parent;
	PlannerGanttRowPriv  *priv;
};

struct _PlannerGanttRowClass {
	GnomeCanvasItemClass parent_class;
};


GType planner_gantt_row_get_type     (void) G_GNUC_CONST;
void  planner_gantt_row_get_geometry (PlannerGanttRow *row,
				 gdouble    *x1,
				 gdouble    *y1,
				 gdouble    *x2,
				 gdouble    *y2);
void  planner_gantt_row_set_visible  (PlannerGanttRow *row,
				 gboolean    is_visible);
