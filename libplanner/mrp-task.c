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

/**
 * SECTION:mrp-task
 * @Short_description: represents a task in the project.
 * @Title: MrpTask
 * @include: libplanner/mrp-task.h
 *
 * Tasks have a name.
 *
 * They form a tree: The work breakdown structure (WBS).
 *
 * A task is estimated in term of work effort, the mythical man-month.
 *
 * Some tasks are of the fixed-duration type.
 * They still can consume resources.
 * Some tasks are milestones.
 * They take no time, no resource.
 * They are just markers.
 *
 * A task have a time constraint.
 * By default, it starts as soon as possible.
 *
 * Tasks are in logical precedence relationship with each other.
 *
 * A task lists the assignments it is involved in.
 *
 * During scheduling, start and finish times are calculated.
 * CPM latest start and latest finish times are also calculated.
 * Tasks belonging to the critical path are marked.
 *
 * A task have a cost.
 *
 * A task have a priority.
 *
 * A task can retain its progress.
 *
 * Because of non-working periods, work might begin after the task start.
 */

#include <config.h>
#include <string.h>
#include "mrp-marshal.h"
#include <glib/gi18n.h>
#include "mrp-private.h"
#include "mrp-resource.h"
#include "mrp-error.h"
#include "mrp-relation.h"

/* Properties */
enum {
	PROP_0,
	PROP_NAME,
	PROP_START,
	PROP_FINISH,
	PROP_LATEST_START,
	PROP_LATEST_FINISH,
	PROP_DURATION,
	PROP_WORK,
	PROP_CRITICAL,
	PROP_TYPE,
	PROP_SCHED,
	PROP_CONSTRAINT,
	PROP_NOTE,
	PROP_PERCENT_COMPLETE,
	PROP_PRIORITY
};

/* Signals */
enum {
	RELATION_ADDED,
	RELATION_REMOVED,
	TASK_MOVED,
	ASSIGNMENT_ADDED,
	ASSIGNMENT_REMOVED,
	CHILD_ADDED,
	CHILD_REMOVED,
	LAST_SIGNAL
};

struct _MrpTask {
	MrpObject parent_instance;
};

typedef struct {
	guint             critical : 1;

	/* Used for topological order sorting. */
	guint             visited : 1;

	MrpTaskGraphNode *graph_node;

	/* FIXME: This might be a mistake... I can't think of any other types,
	 * besides milestone and normal. Should we have a boolean instead,
	 * is_milestone? That or a flags type variable.
	 */
	MrpTaskType       type;

	/* FIXME: See above, could use a flags type and keep this info there as
	 * well.
	 */
	MrpTaskSched      sched;

	/* Percent complete, 0-100. */
	gshort            percent_complete;

	/* Arbitrary range of 0,1..9999. A hint for any (3rd party) resource leveller */
	gint              priority;

	gchar            *name;
	gchar            *note;

	/* The amount of work effort (duration*units) necessary to complete this
	 * task.
	 */
	gint              work;

	/* Calculated duration. */
	gint              duration;

	/* Slack. */
	gint              slack;

	/* Calculated start and finish values. */
	mrptime           start;
	mrptime           finish;

	/* The time of the first work performed on the task. Needed so clients
	 * know when to start drawing if they don't want to draw from the start
	 * if the start is during non-work time.
	 */
	mrptime           work_start;

	/* Represents the Work Breakdown Structure tree hierarchy. */
	GNode            *node;

	/* The acyclic dependency graph. */
	GList            *successors;
	GList            *predecessors;

	/* Calculated CPM values. */
	mrptime           latest_start;
	mrptime           latest_finish;

	MrpConstraint     constraint;

	/* List of assignments. */
	GList            *assignments;

	/* Intervals to build graphical view of the task */
	GList            *unit_ivals;

	gfloat            cost;
	gboolean          cost_cached;
} MrpTaskPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (MrpTask, mrp_task, MRP_TYPE_OBJECT)

static void task_assignment_removed_cb (MrpAssignment      *assignment,
					MrpTask            *task);
static void task_remove_assignments    (MrpTask            *task);
static void task_remove_relations      (MrpTask            *task);


static guint signals[LAST_SIGNAL];

static void
mrp_task_init (MrpTask *task)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);

	priv->name = g_strdup ("");
	priv->node = g_node_new (task);
	priv->assignments = NULL;
	priv->constraint.type = MRP_CONSTRAINT_ASAP;
	priv->graph_node = g_new0 (MrpTaskGraphNode, 1);
	priv->note = g_strdup ("");

	priv->cost = 0.0;
	priv->cost_cached = FALSE;
	priv->unit_ivals = NULL;
}

static void
mrp_task_finalize (GObject *object)
{
	MrpTask *task = MRP_TASK (object);
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);

	g_free (priv->name);
	g_free (priv->note);

	/* Make sure we aren't left hanging in the tree. */
	g_assert (priv->node->parent == NULL);

	/* Make sure we don't have dangling relations. */
	g_assert (priv->predecessors == NULL);
	g_assert (priv->successors == NULL);

	g_node_destroy (priv->node);

	G_OBJECT_CLASS (mrp_task_parent_class)->finalize (object);
}

static void
mrp_task_set_property (GObject      *object,
                       guint         prop_id,
                       const GValue *value,
                       GParamSpec   *pspec)
{
	MrpTask     *task = MRP_TASK (object);
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);
	const gchar *str;
	gint         i_val;
	MrpTaskType  type;
	gboolean     changed = FALSE;

	switch (prop_id) {
	case PROP_NAME:
		str = g_value_get_string (value);

		if (!priv->name || strcmp (priv->name, str)) {
			g_free (priv->name);
			priv->name = g_strdup (str);
			changed = TRUE;
		}

		break;

	case PROP_NOTE:
		str = g_value_get_string (value);

		if (!priv->note || strcmp (priv->note, str)) {
			g_free (priv->note);
			priv->note = g_strdup (str);
			changed = TRUE;
		}

		break;

	case PROP_START:
		priv->start = g_value_get_int64 (value);
		break;

	case PROP_FINISH:
		priv->finish = g_value_get_int64 (value);
		break;

	case PROP_DURATION:
		if (mrp_task_get_n_children (task) > 0 ||
		    priv->type == MRP_TASK_TYPE_MILESTONE) {
			return;
		}

		if (priv->sched == MRP_TASK_SCHED_FIXED_WORK) {
			return;
		}

		i_val = g_value_get_int (value);

		if (priv->duration != i_val) {
			priv->duration = i_val;

			g_object_notify (object, "work");
			changed = TRUE;

			mrp_task_invalidate_cost (task);
		}
		break;

	case PROP_WORK:
		if (mrp_task_get_n_children (task) > 0 ||
		    priv->type == MRP_TASK_TYPE_MILESTONE) {
			return;
		}

		i_val = g_value_get_int (value);

		if (priv->work != i_val) {
			priv->work = i_val;

			g_object_notify (object, "duration");
			changed = TRUE;

			mrp_task_invalidate_cost (task);
		}
		break;

	case PROP_CRITICAL:
		priv->critical = g_value_get_boolean (value);
		break;

	case PROP_TYPE:
		type = g_value_get_enum (value);
		if (type != priv->type) {
			priv->type = type;

			if (type == MRP_TASK_TYPE_MILESTONE) {
				priv->duration = 0;
				priv->work = 0;
			} else {
				/* FIXME: we need a way to specify a default
				 * work/duration for tasks.
				 */
				priv->duration = 60*60*8;
				priv->work = 60*60*8;
			}

			g_object_notify (G_OBJECT (task), "duration");
			g_object_notify (G_OBJECT (task), "work");

			changed = TRUE;
		}
		break;

	case PROP_SCHED:
		priv->sched = g_value_get_enum (value);
		changed = TRUE;
		break;

	case PROP_CONSTRAINT:
		/* FIXME: compare */
		priv->constraint = *(MrpConstraint *) g_value_get_boxed (value);
		changed = TRUE;
		break;

	case PROP_PERCENT_COMPLETE:
		i_val = g_value_get_int (value);

		if (priv->percent_complete != i_val) {
			priv->percent_complete = i_val;
			changed = TRUE;
		}

		break;

	case PROP_PRIORITY:
		i_val = g_value_get_int (value);

		if (priv->priority != i_val) {
			priv->priority = i_val;
			changed = TRUE;
		}

		break;

	default:
		break;
	}

	if (changed) {
		mrp_object_changed (MRP_OBJECT (object));
	}
}

static void
mrp_task_get_property (GObject    *object,
                       guint       prop_id,
                       GValue     *value,
                       GParamSpec *pspec)
{
	MrpTask     *task = MRP_TASK (object);
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);

	switch (prop_id) {
	case PROP_NAME:
		g_value_set_string (value, priv->name);
		break;
	case PROP_NOTE:
		g_value_set_string (value, priv->note);
		break;
	case PROP_START:
		g_value_set_int64 (value, priv->start);
		break;
	case PROP_FINISH:
		g_value_set_int64 (value, priv->finish);
		break;
	case PROP_LATEST_START:
		g_value_set_int64 (value, priv->latest_start);
		break;
	case PROP_LATEST_FINISH:
		g_value_set_int64 (value, priv->latest_finish);
		break;
	case PROP_DURATION:
		g_value_set_int (value, priv->duration);
		break;
	case PROP_WORK:
		g_value_set_int (value, priv->work);
		break;
	case PROP_CRITICAL:
		g_value_set_boolean (value, priv->critical);
		break;
	case PROP_TYPE:
		g_value_set_enum (value, priv->type);
		break;
	case PROP_SCHED:
		g_value_set_enum (value, priv->sched);
		break;
	case PROP_CONSTRAINT:
		g_value_set_boxed (value, &priv->constraint);
		break;
	case PROP_PERCENT_COMPLETE:
		g_value_set_int (value, priv->percent_complete);
		break;
	case PROP_PRIORITY:
		g_value_set_int (value, priv->priority);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
mrp_task_removed (MrpObject *object)
{
	MrpTask     *task;

	g_return_if_fail (MRP_IS_TASK (object));

	task = MRP_TASK (object);

	task_remove_assignments (task);
	task_remove_relations (task);

	if (MRP_OBJECT_CLASS (mrp_task_parent_class)->removed)
		MRP_OBJECT_CLASS (mrp_task_parent_class)->removed (object);
}

static void
mrp_task_class_init (MrpTaskClass *klass)
{
	GObjectClass   *object_class     = G_OBJECT_CLASS (klass);
	MrpObjectClass *mrp_object_class = MRP_OBJECT_CLASS (klass);

	object_class->finalize = mrp_task_finalize;
	object_class->set_property = mrp_task_set_property;
	object_class->get_property = mrp_task_get_property;

	mrp_object_class->removed  = mrp_task_removed;

    /**
     * MrpTask::task-moved:
     * @task: the object which received the signal.
     * @other_task: another #MrpTask.
     *
     * emitted when @other_task is moved.
     */
	signals[TASK_MOVED] =
		g_signal_new ("task_moved",
			      G_TYPE_FROM_CLASS (klass),
			      G_SIGNAL_RUN_LAST,
			      0,
			      NULL, NULL,
			      mrp_marshal_VOID__OBJECT_INT,
			      G_TYPE_NONE, 2, MRP_TYPE_TASK, G_TYPE_INT);

    /**
     * MrpTask::relation-added:
     * @task: the object which received the signal.
     * @relation: the added #MrpRelation.
     *
     * emitted when @relation is added.
     */
	signals[RELATION_ADDED] =
		g_signal_new ("relation_added",
			      G_TYPE_FROM_CLASS (klass),
			      G_SIGNAL_RUN_LAST,
			      0,
			      NULL, NULL,
			      mrp_marshal_VOID__POINTER,
			      G_TYPE_NONE, 1, G_TYPE_POINTER);

    /**
     * MrpTask::relation-removed:
     * @task: the object which received the signal.
     * @relation: the removed #MrpRelation
     *
     * emitted when @relation is removed.
     */
	signals[RELATION_REMOVED] =
		g_signal_new ("relation_removed",
			      G_TYPE_FROM_CLASS (klass),
			      G_SIGNAL_RUN_LAST,
			      0,
			      NULL, NULL,
			      mrp_marshal_VOID__POINTER,
			      G_TYPE_NONE, 1, G_TYPE_POINTER);

    /**
     * MrpTask::assignment-added:
     * @task: the object which received the signal.
     * @assignment: the added #MrpAssignment.
     *
     * emitted when @assignment is added.
     */
	signals[ASSIGNMENT_ADDED] =
		g_signal_new ("assignment_added",
			      G_TYPE_FROM_CLASS (klass),
			      G_SIGNAL_RUN_LAST,
			      0,
			      NULL, NULL,
			      mrp_marshal_VOID__OBJECT,
			      G_TYPE_NONE,
			      1, MRP_TYPE_ASSIGNMENT);

    /**
     * MrpTask::assignment-removed:
     * @task: the object which received the signal.
     * @assignment: the removed #MrpAssignment.
     *
     * emitted when @assignment is removed.
     */
	signals[ASSIGNMENT_REMOVED] =
		g_signal_new ("assignment_removed",
			      G_TYPE_FROM_CLASS (klass),
			      G_SIGNAL_RUN_LAST,
			      0,
			      NULL, NULL,
			      mrp_marshal_VOID__OBJECT,
			      G_TYPE_NONE,
			      1, MRP_TYPE_ASSIGNMENT);

    /**
     * MrpTask::child-added:
     * @task: the object which received the signal.
     *
     * emitted when a child is added.
     */
	signals[CHILD_ADDED] =
		g_signal_new ("child_added",
			      G_TYPE_FROM_CLASS (klass),
			      G_SIGNAL_RUN_LAST,
			      0,
			      NULL, NULL,
			      mrp_marshal_VOID__VOID,
			      G_TYPE_NONE, 0);

    /**
     * MrpTask::child-removed:
     * @task: the object which received the signal.
     *
     * emitted when a child is removed.
     */
	signals[CHILD_REMOVED] =
		g_signal_new ("child_removed",
			      G_TYPE_FROM_CLASS (klass),
			      G_SIGNAL_RUN_LAST,
			      0,
			      NULL, NULL,
			      mrp_marshal_VOID__VOID,
			      G_TYPE_NONE, 0);

	g_object_class_install_property (
		object_class,
		PROP_NAME,
		g_param_spec_string ("name",
				     "Name",
				     "Name of the task",
				     "",
				     G_PARAM_READWRITE));

	g_object_class_install_property (
		object_class,
		PROP_NOTE,
		g_param_spec_string ("note",
				     "Note",
				     "Note attached to the task",
				     "",
				     G_PARAM_READWRITE));

	g_object_class_install_property (
		object_class,
		PROP_START,
		mrp_param_spec_time ("start",
				     "Start time",
				     "Task Start time",
				     G_PARAM_READABLE));

	g_object_class_install_property (
		object_class,
		PROP_FINISH,
		mrp_param_spec_time ("finish",
				     "Finish time",
				     "Task finish time",
				     G_PARAM_READABLE));

	g_object_class_install_property (
		object_class,
		PROP_LATEST_START,
		mrp_param_spec_time ("latest_start",
				     "Latest start",
				     "Latest task start time",
				     G_PARAM_READABLE));

	g_object_class_install_property (
		object_class,
		PROP_LATEST_FINISH,
		mrp_param_spec_time ("latest_finish",
				     "Latest finish",
				     "Latest task finish time",
				     G_PARAM_READABLE));

	g_object_class_install_property (
		object_class,
		PROP_DURATION,
		g_param_spec_int ("duration",
				  "Duration",
				  "Duration of the task",
				  -1, G_MAXINT, 0,
				  G_PARAM_READWRITE));

	g_object_class_install_property (
		object_class,
		PROP_WORK,
		g_param_spec_int ("work",
				  "Work",
				  "Task work",
				  -1, G_MAXINT, 0,
				  G_PARAM_READWRITE));

	g_object_class_install_property (
		object_class,
		PROP_CRITICAL,
		g_param_spec_boolean ("critical",
				      "Critical",
				      "In critical path",
				      FALSE,
				      G_PARAM_READWRITE));

	g_object_class_install_property (
		object_class,
		PROP_TYPE,
		g_param_spec_enum ("type",
				   "Type",
				   "Task type",
				   MRP_TYPE_TASK_TYPE,
				   MRP_TASK_TYPE_NORMAL,
				   G_PARAM_READWRITE));

	g_object_class_install_property (
		object_class,
		PROP_SCHED,
		g_param_spec_enum ("sched",
				   "Scheduling type",
				   "Task scheduling type",
				   MRP_TYPE_TASK_SCHED,
				   MRP_TASK_SCHED_FIXED_WORK,
				   G_PARAM_READWRITE));

	g_object_class_install_property (
		object_class,
		PROP_CONSTRAINT,
		g_param_spec_boxed ("constraint",
				    "Constraint",
				    "Task scheduling constraint",
				    MRP_TYPE_CONSTRAINT,
				    G_PARAM_READWRITE));

	g_object_class_install_property (
		object_class,
		PROP_PERCENT_COMPLETE,
		g_param_spec_int ("percent_complete",
				  "Percent complete",
				  "Percent completed of task",
				  0, 100, 0,
				  G_PARAM_READWRITE));

	g_object_class_install_property (
		object_class,
		PROP_PRIORITY,
		g_param_spec_int ("priority",
				  "Priority",
				  "Priority of the task",
				  0, 9999, 0,
				  G_PARAM_READWRITE));
}

static void
task_assignment_removed_cb (MrpAssignment *assignment, MrpTask *task)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);

	g_return_if_fail (MRP_IS_TASK (task));
	g_return_if_fail (MRP_IS_ASSIGNMENT (assignment));

	priv->assignments = g_list_remove (priv->assignments, assignment);

	g_signal_emit (task, signals[ASSIGNMENT_REMOVED], 0, assignment);
	g_object_unref (assignment);

	mrp_object_changed (MRP_OBJECT (task));
}

static void
task_remove_relations (MrpTask *task)
{
	GList       *l, *next;
	MrpRelation *relation;
	MrpTask     *predecessor;
	MrpTask     *successor;
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);

	g_return_if_fail (MRP_IS_TASK (task));

	/* Cut relations involving the task (make sure to be robust when
	 * removing during traversing).
	 */
	l = priv->predecessors;
	while (l) {
		next = l->next;
		relation = l->data;

		predecessor = mrp_relation_get_predecessor (relation);

		mrp_task_remove_predecessor (task, predecessor);

		l = next;
	}

	l = priv->successors;
	while (l) {
		next = l->next;
		relation = l->data;

		successor = mrp_relation_get_successor (relation);

		mrp_task_remove_predecessor (successor, task);

		l = next;
	}
}

static void
task_remove_assignments (MrpTask *task)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);
	GList         *copy, *l;
	MrpAssignment *assignment;

	g_return_if_fail (MRP_IS_TASK (task));

	copy = g_list_copy (priv->assignments);

	for (l = copy; l; l = l->next) {
		assignment = l->data;

		g_signal_handlers_disconnect_by_func (MRP_ASSIGNMENT (l->data),
						      task_assignment_removed_cb,
						      task);
		g_object_unref (assignment);
		mrp_object_removed (MRP_OBJECT (assignment));
	}

	g_list_free (priv->assignments);
	g_list_free (copy);

	priv->assignments = NULL;
}

static gboolean
task_remove_subtree_cb (GNode *node, gpointer data)
{
	MrpTask     *task = node->data;
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);

	task_remove_relations (task);
	task_remove_assignments (task);

	g_node_unlink (priv->node);

	mrp_object_removed (MRP_OBJECT (task));

	g_object_unref (task);

	return FALSE;
}

void
imrp_task_remove_subtree (MrpTask *task)
{
	MrpTask *parent;
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);

	g_return_if_fail (MRP_IS_TASK (task));

	parent = NULL;
	if (priv->node->parent) {
		parent = priv->node->parent->data;
	}

	g_object_ref (task);

	/* Remove the tasks one by one using post order so we don't mess with
	 * the tree while traversing it.
	 */
	g_node_traverse (priv->node,
			 G_POST_ORDER,
			 G_TRAVERSE_ALL,
			 -1,
			 (GNodeTraverseFunc) task_remove_subtree_cb,
			 NULL);

	g_object_unref (task);

	if (parent) {
		mrp_task_invalidate_cost (parent);
		g_signal_emit (parent, signals[CHILD_REMOVED], 0);
	}
}

void
imrp_task_detach (MrpTask *task)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);

	g_return_if_fail (MRP_IS_TASK (task));

	/* FIXME: Do some extra checking. */

	g_node_unlink (priv->node);
}

void
imrp_task_reattach (MrpTask  *task,
		    MrpTask  *sibling,
		    MrpTask  *parent,
		    gboolean  before)
{
	GNode *node;
	MrpTaskPrivate *sibling_priv;
	MrpTaskPrivate *task_priv= mrp_task_get_instance_private (task);
	MrpTaskPrivate *parent_priv = mrp_task_get_instance_private (parent);

	g_return_if_fail (MRP_IS_TASK (task));
	g_return_if_fail (sibling == NULL || MRP_IS_TASK (sibling));
	g_return_if_fail (MRP_IS_TASK (parent));

	/* FIXME: Do some extra checking. */

	if (parent_priv->type == MRP_TASK_TYPE_MILESTONE &&
	    !parent_priv->node->children) {
		g_object_set (parent,
			      "type", MRP_TASK_TYPE_NORMAL,
			      "sched", MRP_TASK_SCHED_FIXED_WORK,
			      NULL);
	}

	if (sibling == NULL) {
		if (before) {
			node = g_node_first_child (parent_priv->node);
		} else {
			node = g_node_last_child (parent_priv->node);
		}

		if (node) {
			sibling = node->data;
		}
	}

	sibling_priv = mrp_task_get_instance_private (sibling);

	if (before) {
		if (sibling != NULL) {
			g_node_insert_before (parent_priv->node,
					      sibling_priv->node,
					      task_priv->node);
		} else {
			g_node_prepend (parent_priv->node,
					task_priv->node);
		}
	} else {
		if (sibling != NULL) {
			g_node_insert_after (parent_priv->node,
					     sibling_priv->node,
					     task_priv->node);
		} else {
			g_node_append (parent_priv->node,
				       task_priv->node);
		}
	}
}

void
imrp_task_reattach_pos (MrpTask  *task,
			gint      pos,
			MrpTask  *parent)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);
	MrpTaskPrivate *parent_priv = mrp_task_get_instance_private (parent);

	g_return_if_fail (MRP_IS_TASK (task));
	g_return_if_fail (pos >= -1);
	g_return_if_fail (MRP_IS_TASK (parent));

	/* FIXME: Do some extra checking. */

	g_node_insert (parent_priv->node,
		       pos,
		       priv->node);
}

/**
 * mrp_task_new:
 *
 * Create a new task.
 *
 * Return value: the newly created #MrpTask.
 **/
MrpTask *
mrp_task_new (void)
{
	MrpTask *task;

	task = g_object_new (MRP_TYPE_TASK, NULL);

	return task;
}

/**
 * mrp_task_get_name:
 * @task: an #MrpTask
 *
 * Retrieves the name of @task.
 *
 * Return value: the name
 **/
const gchar *
mrp_task_get_name (MrpTask *task)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);

	g_return_val_if_fail (MRP_IS_TASK (task), NULL);

	return priv->name;
}

/**
 * mrp_task_set_name:
 * @task: an #MrpTask
 * @name: new name of @task
 *
 * Sets the name of @task.
 **/
void mrp_task_set_name (MrpTask *task, const gchar *name)
{
	g_return_if_fail (MRP_IS_TASK (task));
	g_return_if_fail (name != NULL);

	mrp_object_set (MRP_OBJECT (task), "name", name, NULL);
}

void
imrp_task_insert_child (MrpTask *parent,
			gint     position,
			MrpTask *child)
{
	MrpTaskPrivate *parent_priv = mrp_task_get_instance_private (parent);
	MrpTaskPrivate *child_priv = mrp_task_get_instance_private (child);

	g_return_if_fail (MRP_IS_TASK (parent));
	g_return_if_fail (MRP_IS_TASK (child));

	if (child_priv->duration == -1) {
		child_priv->duration = parent_priv->duration;
	}

	g_node_insert (parent_priv->node,
		       position,
		       child_priv->node);

	mrp_task_invalidate_cost (parent);

	if (parent_priv->type == MRP_TASK_TYPE_MILESTONE) {
		g_object_set (parent,
			      "type", MRP_TASK_TYPE_NORMAL,
			      NULL);
	}

	g_signal_emit (parent, signals[CHILD_ADDED], 0);
}

static MrpRelation *
task_get_predecessor_relation (MrpTask *task,
			       MrpTask *predecessor)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);

	GList *l;

	for (l = priv->predecessors; l; l = l->next) {
		MrpRelation *relation = l->data;

		if (mrp_relation_get_successor (relation) == task &&
		    mrp_relation_get_predecessor (relation) == predecessor) {
			return relation;
		}
	}

	return NULL;
}

static MrpRelation *
task_get_successor_relation (MrpTask *task,
			     MrpTask *successor)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);

	GList *l;

	for (l = priv->successors; l; l = l->next) {
		MrpRelation *relation = l->data;

		if (mrp_relation_get_predecessor (relation) == task &&
		    mrp_relation_get_successor (relation) == successor) {
			return relation;
		}
	}

	return NULL;
}

/**
 * mrp_task_has_relation_to:
 * @task_a: an #MrpTask
 * @task_b: an #MrpTask
 *
 * Checks if @task_a and @task_b has a relation, i.e. if @task_a is a
 * predecessor or successor of @task_b.
 *
 * Return value: %TRUE if @a and @b has a relation
 **/
gboolean
mrp_task_has_relation_to (MrpTask *task_a, MrpTask *task_b)
{
	return (task_get_predecessor_relation (task_a, task_b) != NULL) ||
		(task_get_predecessor_relation (task_b, task_a) != NULL);
}

/**
 * mrp_task_has_relation:
 * @task: an #MrpTask
 *
 * Checks if a task has any relations, i.e. predecessors or successors.
 *
 * Return value: %TRUE if there are any relations.
 **/
gboolean
mrp_task_has_relation (MrpTask *task)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);

	return (priv->predecessors != NULL ||
		priv->successors != NULL);
}

/**
 * mrp_task_add_predecessor:
 * @task: an #MrpTask
 * @predecessor: the predecessor
 * @type: type of relation
 * @lag: lag time, if negative, it means lead time
 * @error: location to store error, or %NULL
 *
 * Adds a predecessor task to a task. Depending on type, the predecessor
 * must be started or finished before task can be started or finished,
 * with an optional lag/lead time.
 *
 * Return value: the relation that represents the predecessor/successor link.
 **/
MrpRelation *
mrp_task_add_predecessor (MrpTask          *task,
			  MrpTask          *predecessor,
			  MrpRelationType   type,
			  glong             lag,
			  GError          **error)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);
	MrpTaskPrivate *predecessor_priv = mrp_task_get_instance_private (predecessor);

	MrpRelation             *relation;
	MrpProject              *project;
	MrpTaskManager          *manager;
	GList                   *relations;
	gchar                   *tmp;
	MrpConstraint            constraint;
	mrptime                  pred_start;

	g_return_val_if_fail (MRP_IS_TASK (task), NULL);
	g_return_val_if_fail (MRP_IS_TASK (predecessor), NULL);

	/* Can't have more than one relation between the same two tasks. */
	if (mrp_task_has_relation_to (task, predecessor)) {
		g_set_error (error,
			     MRP_ERROR,
			     MRP_ERROR_TASK_RELATION_FAILED,
			     _("Could not add a predecessor relation, because the tasks are already related."));

		return NULL;
	}

	relations = mrp_task_get_predecessor_relations (task);

	/* check for attempt to add SF or FF relation when other relation types already present */
	if ((type == MRP_RELATION_SF || type == MRP_RELATION_FF) && relations) {

		if (type == MRP_RELATION_SF) {
			tmp = _("Start to Finish relation type cannot be combined with other relations.");
		} else {
			tmp = _("Finish to Finish relation type cannot be combined with other relations.");
		}

		g_set_error (error,
			     MRP_ERROR,
			     MRP_ERROR_TASK_RELATION_FAILED,
			     "%s",
			     tmp);

		return NULL;
	}

	/* check for attempt to add SF or FF when a Start No Earlier Than constraint exists */
	constraint = imrp_task_get_constraint (task);
	if ((type == MRP_RELATION_SF || type == MRP_RELATION_FF) &&
	    constraint.type == MRP_CONSTRAINT_SNET) {
		if (type == MRP_RELATION_SF) {
			tmp = _("Start to Finish relation type cannot be combined with Start No Earlier Than constraint.");
		} else {
			tmp = _("Finish to Finish relation type cannot be combined with Start No Earlier Than constraint.");
		}

		g_set_error (error,
			     MRP_ERROR,
			     MRP_ERROR_TASK_RELATION_FAILED,
			     "%s",
			     tmp);

		return NULL;
	}

	/* check for attempt to add SF when predecessor starts on project start date
	   this would of course cause the task to be scheduled before project start */

	project = mrp_object_get_project (MRP_OBJECT (task));
	pred_start = mrp_time_align_day (mrp_task_get_work_start (predecessor));

	if ((type == MRP_RELATION_SF) &&
	    pred_start == mrp_project_get_project_start (project)) {

		g_set_error (error,
			     MRP_ERROR,
			     MRP_ERROR_TASK_RELATION_FAILED,
			     _("Start to Finish relation cannot be set. Predecessor starts on project start date."));

		return NULL;
	}

	manager = imrp_project_get_task_manager (project);
	if (!mrp_task_manager_check_predecessor (manager, task, predecessor, error)) {
		return NULL;
	}

	relation = g_object_new (MRP_TYPE_RELATION,
				 "successor", task,
				 "predecessor", predecessor,
				 "type", type,
				 "lag", lag,
				 NULL);

	priv->predecessors = g_list_prepend (priv->predecessors, relation);
	predecessor_priv->successors = g_list_prepend (predecessor_priv->successors, relation);

	g_signal_emit (task, signals[RELATION_ADDED], 0, relation);
	g_signal_emit (predecessor, signals[RELATION_ADDED], 0, relation);

	mrp_object_changed (MRP_OBJECT (task));
	mrp_object_changed (MRP_OBJECT (predecessor));

	return relation;
}

/**
 * mrp_task_remove_predecessor:
 * @task: an #MrpTask
 * @predecessor: the predecessor to remove
 *
 * Removes a predecessor previously added to task.
 **/
void
mrp_task_remove_predecessor (MrpTask *task,
			     MrpTask *predecessor)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);
	MrpTaskPrivate *predecessor_priv = mrp_task_get_instance_private (predecessor);

	MrpRelation *relation;

	g_return_if_fail (MRP_IS_TASK (task));
	g_return_if_fail (MRP_IS_TASK (predecessor));

	relation = task_get_predecessor_relation (task, predecessor);

	priv->predecessors = g_list_remove (priv->predecessors, relation);
	predecessor_priv->successors = g_list_remove (predecessor_priv->successors, relation);

	/* Notify. */
	mrp_object_removed (MRP_OBJECT (relation));

	g_signal_emit (task, signals[RELATION_REMOVED], 0, relation);
	g_signal_emit (predecessor, signals[RELATION_REMOVED], 0, relation);

	mrp_object_changed (MRP_OBJECT (task));
	mrp_object_changed (MRP_OBJECT (predecessor));

	/* FIXME: ? */
	g_object_unref (relation);
}

/**
 * mrp_task_get_relation:
 * @task_a: an #MrpTask
 * @task_b: an #MrpTask
 *
 * Fetches a relation between two tasks if it exists.
 *
 * Return value: a #MrpRelation representing the relation between @task_a and
 * @task_b or %NULL if they don't have any relation.
 **/
MrpRelation *
mrp_task_get_relation (MrpTask *task_a, MrpTask *task_b)
{
	MrpRelation *relation;

	g_return_val_if_fail (MRP_IS_TASK (task_a), NULL);
	g_return_val_if_fail (MRP_IS_TASK (task_b), NULL);

	relation = task_get_predecessor_relation (task_a, task_b);
	if (relation != NULL) {
		return relation;
	}

	return task_get_successor_relation (task_a, task_b);
}

/**
 * mrp_task_get_predecessor_relation:
 * @task: an #MrpTask
 * @predecessor: #an MrpTask
 *
 * Fetches a predecessor relation between task and it's predecessor.
 *
 * Return value: the #MrpRelation if it exists, otherwise %NULL
 **/
MrpRelation *
mrp_task_get_predecessor_relation (MrpTask *task, MrpTask *predecessor)
{
	g_return_val_if_fail (MRP_IS_TASK (task), NULL);
	g_return_val_if_fail (MRP_IS_TASK (predecessor), NULL);

	return task_get_predecessor_relation (task, predecessor);
}

/**
 * mrp_task_get_successor_relation:
 * @task: an #MrpTask
 * @successor: an #MrpTask
 *
 * Fetches a successor relation between task and it's successor.
 *
 * Return value: the #MrpRelation if it exists, otherwise %NULL
 **/
MrpRelation *
mrp_task_get_successor_relation (MrpTask *task, MrpTask *successor)
{
	MrpRelation *relation;

	g_return_val_if_fail (MRP_IS_TASK (task), NULL);
	g_return_val_if_fail (MRP_IS_TASK (successor), NULL);

	relation = task_get_successor_relation (task, successor);
	return relation;
}

/**
 * mrp_task_get_predecessor_relations:
 * @task: an #MrpTask
 *
 * Fetches a list of predecessor relations to @task.
 *
 * Return value: the list of predecessor relations to @task
 **/
GList *
mrp_task_get_predecessor_relations (MrpTask *task)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);

	g_return_val_if_fail (MRP_IS_TASK (task), NULL);

	return priv->predecessors;
}

/**
 * mrp_task_get_successor_relations:
 * @task: an #MrpTask
 *
 * Fetches a list of successor relations to @task.
 *
 * Return value: a list of successor relations to @task
 **/
GList *
mrp_task_get_successor_relations (MrpTask *task)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);

	g_return_val_if_fail (MRP_IS_TASK (task), NULL);

	return priv->successors;
}

/**
 * mrp_task_get_parent:
 * @task: an #MrpTask
 *
 * Fetches the parent of @task.
 *
 * Return value: the parent of @task, or %NULL if there is no parent..
 **/
MrpTask *
mrp_task_get_parent (MrpTask *task)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);

	GNode *node;

	g_return_val_if_fail (MRP_IS_TASK (task), NULL);

	node = priv->node->parent;
	return node ? node->data : NULL;
}

/**
 * mrp_task_get_first_child:
 * @task: an #MrpTask
 *
 * Fetches the first child of @task.
 *
 * Return value: the first child of @task, or %NULL if there are no children.
 **/
MrpTask *
mrp_task_get_first_child (MrpTask *task)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);

	GNode *node;

	g_return_val_if_fail (MRP_IS_TASK (task), NULL);

	node = g_node_first_child (priv->node);
	return node ? node->data : NULL;
}

/**
 * mrp_task_get_next_sibling:
 * @task: an #MrpTask
 *
 * Fetches the next sibling of @task.
 *
 * Return value: the next sibling of @task, or %NULL if there is no next
 * sibling.
 **/
MrpTask *
mrp_task_get_next_sibling (MrpTask *task)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);

	GNode *node;

	g_return_val_if_fail (MRP_IS_TASK (task), NULL);

	node = g_node_next_sibling (priv->node);
	return node ? node->data : NULL;
}

/**
 * mrp_task_get_prev_sibling:
 * @task: an #MrpTask
 *
 * Fetches the previous sibling of @task.
 *
 * Return value: the previous sibling of @task, or %NULL if there is none.
 **/
MrpTask *
mrp_task_get_prev_sibling (MrpTask *task)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);

	GNode *node;

	g_return_val_if_fail (MRP_IS_TASK (task), NULL);

	node = g_node_prev_sibling (priv->node);
	return node ? node->data : NULL;
}

/**
 * mrp_task_get_n_children:
 * @task: an #MrpTask
 *
 * Fetches the number of children @task has.
 *
 * Return value: the number of children @task has
 **/
guint
mrp_task_get_n_children (MrpTask *task)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);

	g_return_val_if_fail (MRP_IS_TASK (task), 0);

	return g_node_n_children (priv->node);
}

/**
 * mrp_task_get_nth_child:
 * @task: an #MrpTask
 * @n: the index of the child to get
 *
 * Fetches the nth child of @task.
 *
 * Return value: the nth child of @task, or %NULL if there is no such child.
 **/
MrpTask *
mrp_task_get_nth_child (MrpTask *task, guint n)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);

	GNode *node;

	g_return_val_if_fail (MRP_IS_TASK (task), 0);

	node = g_node_nth_child (priv->node, n);
	return node ? node->data : NULL;
}

/**
 * mrp_task_get_position:
 * @task: an #MrpTask
 *
 * Fetches the index or position of @task among its siblings.
 *
 * Return value: the position of @task among its siblings.
 **/
gint
mrp_task_get_position (MrpTask *task)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);

	GNode *parent;

	g_return_val_if_fail (MRP_IS_TASK (task), 0);
	g_return_val_if_fail (priv->node->parent != NULL, 0);

	parent = priv->node->parent;

	return g_node_child_position (parent, priv->node);
}

/**
 * mrp_task_add_assignment:
 * @task: an #MrpTask
 * @assignment: an #MrpAssignment
 *
 * Adds an assignment to @task.
 **/
void
imrp_task_add_assignment (MrpTask *task, MrpAssignment *assignment)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);

	g_return_if_fail (MRP_IS_TASK (task));
	g_return_if_fail (MRP_IS_ASSIGNMENT (assignment));

	priv->assignments = g_list_prepend (priv->assignments,
					    g_object_ref (assignment));

	g_signal_connect (assignment,
			  "removed",
			  G_CALLBACK (task_assignment_removed_cb),
			  task);

	g_signal_emit (task, signals[ASSIGNMENT_ADDED], 0, assignment);

	mrp_object_changed (MRP_OBJECT (task));
}

/**
 * mrp_task_get_start:
 * @task: an #MrpTask
 *
 * Fetches the start time of @task.
 *
 * Return value: the start time of @task.
 **/
mrptime
mrp_task_get_start (MrpTask *task)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);

	g_return_val_if_fail (MRP_IS_TASK (task), 0);

	return priv->start;
}

/**
 * mrp_task_get_finish:
 * @task: an #MrpTask
 *
 * Fetches the finish time of @task.
 *
 * Return value: the finish time of @task.
 **/
mrptime
mrp_task_get_finish (MrpTask *task)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);

	g_return_val_if_fail (MRP_IS_TASK (task), 0);

	return priv->finish;
}

/**
 * mrp_task_get_work_start:
 * @task: an #MrpTask
 *
 * Retrieves the first time where work is performed of @task. This might be
 * different from the start time, if the start time is during non-working
 * time. In that case, the work start would be right after the non-working
 * interval.
 *
 * Return value: The work start time of @task.
 **/
mrptime
mrp_task_get_work_start (MrpTask *task)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);

	g_return_val_if_fail (MRP_IS_TASK (task), 0);

	return priv->work_start;
}

/**
 * mrp_task_get_latest_start:
 * @task: an #MrpTask
 *
 * Retrieves the latest start time of @task, i.e. the latest time the task can
 * start without delaying the project.
 *
 * Return value: The latest start time of @task.
 **/
mrptime
mrp_task_get_latest_start (MrpTask *task)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);
	return priv->latest_start;
}

/**
 * mrp_task_get_latest_finish:
 * @task: an #MrpTask
 *
 * Retrieves the latest finish time of @task, i.e. the latest time the task can
 * finish without delaying the project.
 *
 * Return value: The latest finish time of @task.
 **/
mrptime
mrp_task_get_latest_finish (MrpTask *task)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);
	return priv->latest_finish;
}

/**
 * mrp_task_get_duration:
 * @task: an #MrpTask
 *
 * Fetches the duration of @task. This differs from the calendar duration that
 * is retrieved by (finish - start).
 *
 * Return value: The duration of @task.
 **/
gint
mrp_task_get_duration (MrpTask *task)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);

	g_return_val_if_fail (MRP_IS_TASK (task), 0);

	return priv->duration;
}

/**
 * mrp_task_get_work:
 * @task: an #MrpTask
 *
 * Retrieves the amount of work of @task.
 *
 * Return value: The work of @task.
 **/
gint
mrp_task_get_work (MrpTask *task)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);

	g_return_val_if_fail (MRP_IS_TASK (task), 0);

	return priv->work;
}

/**
 * mrp_task_get_priority:
 * @task: an #MrpTask
 *
 * Retrieves the priority of @task.
 *
 * Return value: The priority of @task.
 **/
gint
mrp_task_get_priority (MrpTask *task)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);

	g_return_val_if_fail (MRP_IS_TASK (task), 0);

	return priv->priority;
}

#ifdef WITH_SIMPLE_PRIORITY_SCHEDULING
/**
 * mrp_task_is_dominant:
 * @task: an #MrpTask
 *
 * Retrieves if @task is a dominant task.
 *
 * Return value: if @task is a dominant task.
 **/
gboolean
mrp_task_is_dominant (MrpTask *task)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);
	MrpConstraint constraint;

	g_return_val_if_fail (MRP_IS_TASK (task), 0);

	constraint = imrp_task_get_constraint (task);
	if (constraint.type  != MRP_CONSTRAINT_MSO) {
		return (FALSE);
	}

	if (priv->priority != MRP_DOMINANT_PRIORITY) {
		return (FALSE);
	}

	return (TRUE);
}
#endif
/**
 * mrp_task_get_unit_ivals:
 * @task: an #MrpTask
 *
 * Retrieves the list of intervals of @task.
 *
 * Return value: Intervals of @task.
 **/
GList *
mrp_task_get_unit_ivals (MrpTask *task)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);

	g_return_val_if_fail (MRP_IS_TASK (task), 0);

	return priv->unit_ivals;
}

/**
 * mrp_task_set_unit_ivals:
 * @task: an #MrpTask
 * @ivals: a list of intervals.
 *
 * Set the list of intervals of @task.
 *
 * Return value: Intervals of @task.
 **/
GList *
mrp_task_set_unit_ivals (MrpTask *task, GList *ivals)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);

	g_return_val_if_fail (MRP_IS_TASK (task), 0);

	if (priv->unit_ivals) {
		g_list_foreach (priv->unit_ivals, (GFunc) g_free, NULL);
		g_list_free (priv->unit_ivals);
		priv->unit_ivals = NULL;
	}
	priv->unit_ivals = ivals;

	return priv->unit_ivals;
}

/**
 * mrp_task_get_assignments:
 * @task: an #MrpTask
 *
 * Fetches a list of #MrpAssignment.
 *
 * Return value: the list of assignments.
 **/
GList *
mrp_task_get_assignments (MrpTask *task)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);

	g_return_val_if_fail (MRP_IS_TASK (task), NULL);

	return priv->assignments;
}

/**
 * mrp_task_get_nres:
 * @task: an #MrpTask
 *
 * Calculate the number of resources assigned to task.
 *
 * Return value: the number of resources.
 **/
gint mrp_task_get_nres (MrpTask *task)
{
	GList *assignments, *a;
	gint nres = 0;

	assignments = mrp_task_get_assignments (task);

	for (a = assignments; a; a = a->next) {
		nres++;
	}

	return (nres);
}

/**
 * mrp_task_get_assignment:
 * @task: an #MrpTask
 * @resource: an #MrpResource
 *
 * retrieves the #MrpAssignment associated with @task and @resource if the
 * resource is assigned to @task, or %NULL if there is no such assignment.
 *
 * Return value: The assignment if it exists, otherwise %NULL.
 **/
MrpAssignment *
mrp_task_get_assignment (MrpTask *task, MrpResource *resource)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);
	GList *l;

	g_return_val_if_fail (MRP_IS_TASK (task), NULL);
	g_return_val_if_fail (MRP_IS_RESOURCE (resource), NULL);

	for (l = priv->assignments; l; l = l->next) {
		MrpAssignment *assignment = l->data;

		if (mrp_assignment_get_resource (assignment) == resource) {
			return assignment;
		}
	}

	return NULL;
}

/**
 * mrp_task_get_assigned_resources:
 * @task: an #MrpTask
 *
 * Fetches a list of resources assigned to @task. The list needs to be freed
 * with g_list_free() by caller.
 *
 * Return value: A newly created list of #MrpResource.
 **/
GList *
mrp_task_get_assigned_resources (MrpTask *task)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);
	GList *list = NULL;
	GList *l;

	g_return_val_if_fail (MRP_IS_TASK (task), NULL);

	for (l = priv->assignments; l; l = l->next) {
		MrpAssignment *assignment = l->data;

		list = g_list_prepend (
			list, mrp_assignment_get_resource (assignment));
	}

	list = g_list_sort (list, mrp_resource_compare);

	return list;
}

/**
 * mrp_task_compare:
 * @a: an #MrpTask
 * @b: an #MrpTask
 *
 * Compares the name of the tasks, by calling strcmp() on the names.
 *
 * Return value: the return value of strcmp (a->name, b->name).
 **/
gint
mrp_task_compare (gconstpointer a, gconstpointer b)
{
	MrpTaskPrivate *priv_a = mrp_task_get_instance_private (MRP_TASK ((gpointer *)a));
	MrpTaskPrivate *priv_b = mrp_task_get_instance_private (MRP_TASK ((gpointer *)b));

	return strcmp (priv_a->name, priv_b->name);
}

/**
 * mrp_task_reset_constraint:
 * @task: an #MrpTask
 *
 * Sets the constraint type to #MRP_CONSTRAINT_ASAP and notifies listeners.
 **/
void
mrp_task_reset_constraint (MrpTask *task)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);

	g_return_if_fail (MRP_IS_TASK (task));

	if (priv->constraint.type != MRP_CONSTRAINT_ASAP) {
		priv->constraint.type = MRP_CONSTRAINT_ASAP;
		g_object_notify (G_OBJECT (task), "constraint");
	}
}

/**
 * mrp_task_get_cost:
 * @task: an #MrpTask
 *
 * Calculates the cost to complete @task.
 *
 * Return value: The cost to complete @task.
 **/
gfloat
mrp_task_get_cost (MrpTask *task)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);
	GList       *assignments, *l;
	MrpResource *resource;
	gfloat       total = 0;
	gfloat       cost;
	MrpTask     *child;

	g_return_val_if_fail (MRP_IS_TASK (task), 0);

	if (priv->cost_cached) {
		return priv->cost;
	}

	/* summary task cost calc */
	child = mrp_task_get_first_child (task);
	if (child) {
		while (child) {
			total += mrp_task_get_cost (child);
			child = mrp_task_get_next_sibling (child);
		}
	} else {
	/* non summary task cost calc */
		assignments = mrp_task_get_assignments (task);
		for (l = assignments; l; l = l->next) {
			resource = mrp_assignment_get_resource (l->data);

			mrp_object_get (resource, "cost", &cost, NULL);
			total += mrp_assignment_get_units (l->data) * priv->duration * cost / (3600.0 * 100);
		}
	}

	priv->cost = total;
	priv->cost_cached = TRUE;

	return total;
}

/**
 * mrp_task_invalidate_cost:
 * @task: a task.
 *
 * invalidates @task cost.
 */
void
mrp_task_invalidate_cost (MrpTask *task)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);

	g_return_if_fail (MRP_IS_TASK (task));

	priv->cost_cached = FALSE;

	if (priv->node->parent) {
		mrp_task_invalidate_cost (priv->node->parent->data);
	}
}

/* Boxed types. */

static MrpConstraint *
mrp_constraint_copy (const MrpConstraint *src)
{
	MrpConstraint *copy;

	copy = g_new (MrpConstraint, 1);
	memcpy (copy, src, sizeof (MrpConstraint));

	return copy;
}

GType
mrp_constraint_get_type (void)
{
	static GType our_type = 0;

	if (our_type == 0) {
		our_type = g_boxed_type_register_static ("MrpConstraint",
							 (GBoxedCopyFunc) mrp_constraint_copy,
							 (GBoxedFreeFunc) g_free);
	}

	return our_type;
}

/* Getters and setters. */

void
imrp_task_set_visited (MrpTask *task, gboolean visited)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);

	g_return_if_fail (MRP_IS_TASK (task));

	priv->visited = visited;
}

gboolean
imrp_task_get_visited (MrpTask *task)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);

	g_return_val_if_fail (MRP_IS_TASK (task), FALSE);

	return priv->visited;
}

void
imrp_task_set_start (MrpTask *task, mrptime start)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);

	g_return_if_fail (MRP_IS_TASK (task));

	priv->start = start;
}

void
imrp_task_set_work_start (MrpTask *task, mrptime work_start)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);

	g_return_if_fail (MRP_IS_TASK (task));

	priv->work_start = work_start;
}

void
imrp_task_set_finish (MrpTask *task, mrptime finish)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);

	g_return_if_fail (MRP_IS_TASK (task));

	priv->finish = finish;
}

void
imrp_task_set_duration (MrpTask *task, gint duration)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);

	g_return_if_fail (MRP_IS_TASK (task));

	priv->duration = duration;
}

void
imrp_task_set_work (MrpTask *task, gint work)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);

	g_return_if_fail (MRP_IS_TASK (task));

	priv->work = work;
}

void
imrp_task_set_latest_start (MrpTask *task,
			    mrptime  time)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);
	priv->latest_start = time;
}

void
imrp_task_set_latest_finish (MrpTask *task,
			     mrptime  time)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);
	priv->latest_finish = time;
}

MrpConstraint
imrp_task_get_constraint (MrpTask *task)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);
	MrpConstraint c = { 0 };

	g_return_val_if_fail (MRP_IS_TASK (task), c);

	return priv->constraint;
}

void
imrp_task_set_constraint (MrpTask *task, MrpConstraint constraint)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);

	g_return_if_fail (MRP_IS_TASK (task));

	priv->constraint = constraint;
}

gint
imrp_task_get_depth (MrpTask *task)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);

	g_return_val_if_fail (MRP_IS_TASK (task), 0);

	return g_node_depth (priv->node);
}

GNode *
imrp_task_get_node (MrpTask *task)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);

	g_return_val_if_fail (MRP_IS_TASK (task), NULL);

	return priv->node;
}

MrpTaskGraphNode *
imrp_task_get_graph_node (MrpTask *task)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);

	g_return_val_if_fail (MRP_IS_TASK (task), NULL);

	return priv->graph_node;
}

GList *
imrp_task_peek_predecessors (MrpTask *task)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);

	g_return_val_if_fail (MRP_IS_TASK (task), NULL);

	return priv->predecessors;
}

GList *
imrp_task_peek_successors (MrpTask *task)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);

	g_return_val_if_fail (MRP_IS_TASK (task), NULL);

	return priv->successors;
}

/**
 * mrp_task_get_task_type:
 * @task: a task.
 * @Returns: a #MrpTaskType.
 *
 * get @task type.
 */
MrpTaskType
mrp_task_get_task_type (MrpTask *task)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);

	g_return_val_if_fail (MRP_IS_TASK (task), MRP_TASK_TYPE_NORMAL);

	return priv->type;
}

/**
 * mrp_task_get_sched:
 * @task: a task.
 * @Returns: a #MrpTaskSched.
 *
 * get @task scheduling type.
 */
MrpTaskSched
mrp_task_get_sched (MrpTask *task)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);

	g_return_val_if_fail (MRP_IS_TASK (task), MRP_TASK_SCHED_FIXED_WORK);

	return priv->sched;
}

/**
 * mrp_task_get_percent_complete:
 * @task: a task.
 * @Returns: a integer.
 *
 * get @task percent complete.
 */
gshort
mrp_task_get_percent_complete (MrpTask *task)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);

	g_return_val_if_fail (MRP_IS_TASK (task), 0);

	return priv->percent_complete;
}

/**
 * mrp_task_get_critical:
 * @task: a task.
 * @Returns: a boolean.
 *
 * is @task critical?
 */
gboolean
mrp_task_get_critical (MrpTask *task)
{
	MrpTaskPrivate *priv = mrp_task_get_instance_private (task);

	g_return_val_if_fail (MRP_IS_TASK (task), FALSE);

	return priv->critical;
}
