/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2001-2003 CodeFactory AB
 * Copyright (C) 2001-2003 Richard Hult <richard@imendio.com>
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

#include <libplanner/mrp-time.h>

#define MRP_TYPE_RELATION_TYPE (mrp_relation_type_get_type ())
#define MRP_TYPE_TASK_TYPE     (mrp_task_type_get_type ())
#define MRP_TYPE_TASK_SCHED    (mrp_task_sched_get_type ())
#define MRP_TYPE_PROPERTY_TYPE (mrp_property_type_get_type ())
#define MRP_TYPE_STRING_LIST   (mrp_string_list_get_type ())

/**
 * MrpRelationType:
 * @MRP_RELATION_NONE: invalid relation type (unset)
 * @MRP_RELATION_FS: finish to start
 * @MRP_RELATION_FF: finish to finish (unimplemented)
 * @MRP_RELATION_SS: start to start (unimplemented)
 * @MRP_RELATION_SF: start to finish (unimplemented)
 *
 * The type of relation.
 */
typedef enum {
	MRP_RELATION_NONE = 0,
	MRP_RELATION_FS,
	MRP_RELATION_FF,
	MRP_RELATION_SS,
	MRP_RELATION_SF
} MrpRelationType;

/**
 * MrpConstraintType:
 * @MRP_CONSTRAINT_ASAP: as soon as possible (default)
 * @MRP_CONSTRAINT_ALAP: as late as possible (unimplemented)
 * @MRP_CONSTRAINT_SNET: start no earlier than
 * @MRP_CONSTRAINT_FNLT: finish no later than (unimplemented)
 * @MRP_CONSTRAINT_MSO: must start on
 *
 * The type of constraint for the task. The default is #MRP_CONSTRAINT_ASAP.
 */
typedef enum {
	MRP_CONSTRAINT_ASAP = 0,
	MRP_CONSTRAINT_ALAP,
	MRP_CONSTRAINT_SNET,
	MRP_CONSTRAINT_FNLT,
	MRP_CONSTRAINT_MSO,
} MrpConstraintType;

struct _MrpConstraint {
	MrpConstraintType type;
	mrptime           time;
};

/**
 * MrpTaskType:
 * @MRP_TASK_TYPE_NORMAL: normal
 * @MRP_TASK_TYPE_MILESTONE: milestone
 *
 * Task types.
 */
typedef enum {
	MRP_TASK_TYPE_NORMAL,
	MRP_TASK_TYPE_MILESTONE
} MrpTaskType;

/**
 * MrpTaskSched:
 * @MRP_TASK_SCHED_FIXED_WORK: fixed work
 * @MRP_TASK_SCHED_FIXED_DURATION: fixed duration
 *
 * Task scheduling types.
 */
typedef enum {
	MRP_TASK_SCHED_FIXED_WORK,
	MRP_TASK_SCHED_FIXED_DURATION
} MrpTaskSched;


/**
 * MrpTask:
 *
 * Object representing a task in the project.
 */
typedef struct _MrpTask       MrpTask;
/**
 * MrpResource:
 *
 * Object representing a resource in the project.
 */
typedef struct _MrpResource   MrpResource;
/**
 * MrpGroup:
 *
 * Object representing a resource group.
 */
typedef struct _MrpGroup      MrpGroup;
/**
 * MrpAssignment:
 *
 * Represents an assignment, i.e. the assignment of a task to a resource.
 */
typedef struct _MrpAssignment MrpAssignment;
/**
 * MrpConstraint:
 * @type: a constraint type.
 * @time: a time.
 *
 * A struct representing a scheduling constraint on a task.
 */
typedef struct _MrpConstraint MrpConstraint;

GType   mrp_relation_type_get_type (void) G_GNUC_CONST;

GType   mrp_task_type_get_type     (void) G_GNUC_CONST;

GType   mrp_task_sched_get_type    (void) G_GNUC_CONST;

GType   mrp_property_type_get_type (void) G_GNUC_CONST;

GList * mrp_string_list_copy       (const GList *list);

void    mrp_string_list_free       (GList       *list);
