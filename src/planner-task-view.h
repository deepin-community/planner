/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2005 Imendio AB
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
#include "planner-view.h"

#define PLANNER_TYPE_TASK_VIEW	          (planner_task_view_get_type ())
#define PLANNER_TASK_VIEW(obj)	          (G_TYPE_CHECK_INSTANCE_CAST ((obj), PLANNER_TYPE_TASK_VIEW, PlannerTaskView))
#define PLANNER_TASK_VIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), PLANNER_TYPE_TASK_VIEW, PlannerTaskViewClass))
#define PLANNER_IS_TASK_VIEW(obj)	  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PLANNER_TYPE_TASK_VIEW))
#define PLANNER_IS_TASK_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PLANNER_TYPE_TASK_VIEW))
#define PLANNER_TASK_VIEW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PLANNER_TYPE_TASK_VIEW, PlannerTaskViewClass))

typedef struct _PlannerTaskView       PlannerTaskView;
typedef struct _PlannerTaskViewClass  PlannerTaskViewClass;
typedef struct _PlannerTaskViewPriv   PlannerTaskViewPriv;

struct _PlannerTaskView {
	PlannerView           parent;
	PlannerTaskViewPriv *priv;
};

struct _PlannerTaskViewClass {
	PlannerViewClass parent_class;
};

GType        planner_task_view_get_type     (void) G_GNUC_CONST;
PlannerView *planner_task_view_new          (void);
