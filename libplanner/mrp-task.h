/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2004-2005 Imendio AB
 * Copyright (C) 2001-2002 CodeFactory AB
 * Copyright (C) 2001-2002 Richard Hult <richard@imendio.com>
 * Copyright (C) 2001-2002 Mikael Hallendal <micke@imendio.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#pragma once

#include <libplanner/mrp-object.h>
#include <libplanner/mrp-types.h>
#include <libplanner/mrp-time.h>
#include <libplanner/mrp-assignment.h>

G_BEGIN_DECLS

#define MRP_TYPE_TASK                   (mrp_task_get_type ())

G_DECLARE_FINAL_TYPE (MrpTask, mrp_task, MRP, TASK, MrpObject)

#define MRP_TYPE_CONSTRAINT             (mrp_constraint_get_type ())
#define MRP_TYPE_RELATION               (mrp_relation_get_type ())

/**
 * MrpUnitsInterval:
 * @is_start: is start.
 * @start: start time.
 * @end: end time.
 * @units: worked units in the interval.
 * @units_full: expected worked units in the interval
 * all resources that are working in the
 * interval in the right percentage.
 * @res_n: number of expected resources working
 * at the task in the interval.
 *
 * [2006-04-11T12:42:44Z]
 * NOTE: moved from libplanner/mrp-task-manager.c to use
 * the structure in the src/planner-gantt-row.c
 * new fields are: units,  units_full, res_n.
 */
typedef struct {
	gboolean is_start;
	mrptime  start;
	mrptime  end;
	gint     units;
	gint     units_full;
	gint     res_n;
} MrpUnitsInterval;

/**
 * UNIT_IVAL_GET_TIME:
 * @R: an interval
 *
 * Get start or end time of an interval.
 */
#define UNIT_IVAL_GET_TIME(R) ((R->is_start?R->start:R->end))

#ifdef WITH_SIMPLE_PRIORITY_SCHEDULING

/**
 * MRP_DOMINANT_PRIORITY:
 *
 * Value of the magic priority of a dominant task.
 */
#define MRP_DOMINANT_PRIORITY           9999
#endif

typedef struct _MrpRelation MrpRelation;

GType            mrp_constraint_get_type            (void) G_GNUC_CONST;
MrpTask         *mrp_task_new                       (void);
const gchar     *mrp_task_get_name                  (MrpTask          *task);
void             mrp_task_set_name                  (MrpTask          *task,
						     const gchar      *name);
MrpRelation     *mrp_task_add_predecessor           (MrpTask          *task,
						     MrpTask          *predecessor,
						     MrpRelationType   type,
						     glong             lag,
						     GError          **error);
void             mrp_task_remove_predecessor        (MrpTask          *task,
						     MrpTask          *predecessor);
MrpRelation     *mrp_task_get_relation              (MrpTask          *task_a,
						     MrpTask          *task_b);
MrpRelation     *mrp_task_get_predecessor_relation  (MrpTask          *task,
						     MrpTask          *predecessor);
MrpRelation     *mrp_task_get_successor_relation    (MrpTask          *task,
						     MrpTask          *successor);
GList           *mrp_task_get_predecessor_relations (MrpTask          *task);
GList           *mrp_task_get_successor_relations   (MrpTask          *task);
gboolean         mrp_task_has_relation_to           (MrpTask          *task_a,
						     MrpTask          *task_b);
gboolean         mrp_task_has_relation              (MrpTask          *task);
MrpTask         *mrp_task_get_parent                (MrpTask          *task);
MrpTask         *mrp_task_get_first_child           (MrpTask          *task);
MrpTask         *mrp_task_get_next_sibling          (MrpTask          *task);
MrpTask         *mrp_task_get_prev_sibling          (MrpTask          *task);
guint            mrp_task_get_n_children            (MrpTask          *task);
MrpTask         *mrp_task_get_nth_child             (MrpTask          *task,
						     guint             n);
gint             mrp_task_get_position              (MrpTask          *task);
mrptime          mrp_task_get_start                 (MrpTask          *task);
mrptime          mrp_task_get_work_start            (MrpTask          *task);
mrptime          mrp_task_get_finish                (MrpTask          *task);
mrptime          mrp_task_get_latest_start          (MrpTask          *task);
mrptime          mrp_task_get_latest_finish         (MrpTask          *task);
gint             mrp_task_get_duration              (MrpTask          *task);
gint             mrp_task_get_work                  (MrpTask          *task);
gint             mrp_task_get_priority              (MrpTask          *task);
#ifdef WITH_SIMPLE_PRIORITY_SCHEDULING
gboolean         mrp_task_is_dominant               (MrpTask          *task);
#endif
GList *          mrp_task_get_unit_ivals            (MrpTask          *task);
GList *          mrp_task_set_unit_ivals            (MrpTask          *task,
						     GList *ivals);
GList           *mrp_task_get_assignments           (MrpTask          *task);
gint             mrp_task_get_nres                  (MrpTask          *task);

/**
 * mrp_task_get_fullwork:
 * @task: a #MrpTask
 * @Returns: an integer
 *
 * <warning>Unimplemented.</warning>
 */
gint             mrp_task_get_fullwork              (MrpTask          *task);
MrpAssignment   *mrp_task_get_assignment            (MrpTask          *task,
						     MrpResource      *resource);
void             mrp_task_reset_constraint          (MrpTask          *task);
gfloat           mrp_task_get_cost                  (MrpTask          *task);
void             mrp_task_invalidate_cost           (MrpTask          *task);
GList           *mrp_task_get_assigned_resources    (MrpTask          *task);
gint             mrp_task_compare                   (gconstpointer     a,
						     gconstpointer     b);
MrpTaskType      mrp_task_get_task_type             (MrpTask          *task);
MrpTaskSched     mrp_task_get_sched                 (MrpTask          *task);
gshort           mrp_task_get_percent_complete      (MrpTask          *task);
gboolean         mrp_task_get_critical              (MrpTask          *task);

G_END_DECLS
