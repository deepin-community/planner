/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2002 CodeFactory AB
 * Copyright (C) 2002 Richard Hult <richard@imendio.com>
 * Copyright (C) 2002 Mikael Hallendal <micke@imendio.com>
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

#define PLANNER_TYPE_GANTT_HEADER		(planner_gantt_header_get_type ())
#define PLANNER_GANTT_HEADER(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), PLANNER_TYPE_GANTT_HEADER, PlannerGanttHeader))
#define PLANNER_GANTT_HEADER_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), PLANNER_TYPE_GANTT_HEADER, PlannerGanttHeaderClass))
#define PLANNER_IS_GANTT_HEADER(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), PLANNER_TYPE_GANTT_HEADER))
#define PLANNER_IS_GANTT_HEADER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((obj), PLANNER_TYPE_GANTT_HEADER))
#define PLANNER_GANTT_HEADER_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), PLANNER_TYPE_GANTT_HEADER, PlannerGanttHeaderClass))

typedef struct _PlannerGanttHeader           PlannerGanttHeader;
typedef struct _PlannerGanttHeaderClass      PlannerGanttHeaderClass;
typedef struct _PlannerGanttHeaderPriv       PlannerGanttHeaderPriv;

struct _PlannerGanttHeader
{
	GtkWidget          parent;
	PlannerGanttHeaderPriv *priv;
};

struct _PlannerGanttHeaderClass
{
	GtkWidgetClass parent_class;

	void  (* set_scroll_adjustments) (PlannerGanttHeader  *header,
					  GtkAdjustment *hadjustment,
					  GtkAdjustment *vadjustment);
};


GType                planner_gantt_header_get_type        (void) G_GNUC_CONST;
GtkWidget             *planner_gantt_header_new             (void);
