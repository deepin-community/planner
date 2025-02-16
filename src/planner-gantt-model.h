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

#include <libplanner/mrp-project.h>
#include <libplanner/mrp-task.h>
#include <gtk/gtk.h>

#define PLANNER_TYPE_GANTT_MODEL            (planner_gantt_model_get_type ())
#define PLANNER_GANTT_MODEL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PLANNER_TYPE_GANTT_MODEL, PlannerGanttModel))
#define PLANNER_GANTT_MODEL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PLANNER_TYPE_GANTT_MODEL, PlannerGanttModelClass))
#define PLANNER_IS_GANTT_MODEL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PLANNER_TYPE_GANTT_MODEL))
#define PLANNER_IS_GANTT_MODEL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((obj), PLANNER_TYPE_GANTT_MODEL))

typedef struct _PlannerGanttModel      PlannerGanttModel;
typedef struct _PlannerGanttModelClass PlannerGanttModelClass;
typedef struct _PlannerGanttModelPriv  PlannerGanttModelPriv;

struct _PlannerGanttModel {
	GObject                parent;
	gint                   stamp;
	PlannerGanttModelPriv *priv;
};

struct _PlannerGanttModelClass {
	GObjectClass parent_class;
};

enum {
	COL_WBS,
	COL_NAME,
	COL_START,
	COL_FINISH,
	COL_DURATION,
	COL_WORK,
	COL_SLACK,
	COL_WEIGHT,
	COL_EDITABLE,
	COL_TASK,
	COL_COST,
	COL_ASSIGNED_TO,
	COL_COMPLETE,
	NUM_COLS
};

GType              planner_gantt_model_get_type               (void) G_GNUC_CONST;
PlannerGanttModel *planner_gantt_model_new                    (MrpProject        *project);
GtkTreePath  *     planner_gantt_model_get_path_from_task     (PlannerGanttModel *model,
							       MrpTask           *task);
MrpTask      *     planner_gantt_model_get_indent_task_target (PlannerGanttModel *model,
							       MrpTask           *task);
MrpProject   *     planner_gantt_model_get_project            (PlannerGanttModel *model);
MrpTask      *     planner_gantt_model_get_task               (PlannerGanttModel *model,
							       GtkTreeIter       *iter);
MrpTask           *planner_gantt_model_get_task_from_path     (PlannerGanttModel *model,
							       GtkTreePath       *path);
