/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2002-2003 CodeFactory AB
 * Copyright (C) 2002-2003 Richard Hult <richard@imendio.com>
 * Copyright (C) 2002 Mikael Hallendal <micke@imendio.com>
 * Copyright (C) 2002 Alvaro del Castillo <acs@barrapunto.com>
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
 * SECTION:mrp-relation
 * @Short_description: represents a task predecessor relation in the project.
 * @Title: MrpRelation
 * @include: libplanner/mrp-relation.h
 *
 * A predecessor relation is used to affect the scheduling of a task
 * relative another task.  A relation may have a lag time associated to it,
 * so that a task can be scheduled to start after another task has
 * finished, plus a lag time.
 */

#include <config.h>
#include <string.h>
#include <glib/gi18n.h>
#include "mrp-types.h"
#include "mrp-marshal.h"
#include "mrp-relation.h"

struct _MrpRelation {
	MrpObject parent_instance;
};

typedef struct {
	MrpTask         *successor;
	MrpTask         *predecessor;

	MrpRelationType  type;
	gint             lag;
} MrpRelationPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (MrpRelation, mrp_relation, MRP_TYPE_OBJECT)

/* Properties */
enum {
	PROP_0,
	PROP_PREDECESSOR,
	PROP_SUCCESSOR,
	PROP_TYPE,
	PROP_LAG
};

/* Signals */
enum {
	CHANGED, /* ? */
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];

static void
mrp_relation_finalize (GObject *object)
{
	MrpRelation *relation = MRP_RELATION (object);
	MrpRelationPrivate *priv = mrp_relation_get_instance_private (relation);

	g_object_unref (priv->successor);
	g_object_unref (priv->predecessor);

	G_OBJECT_CLASS (mrp_relation_parent_class)->finalize (object);
}

static void
mrp_relation_set_property (GObject      *object,
                           guint         prop_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
	MrpRelation     *relation;
	MrpRelationPrivate *priv;
	MrpTask         *task;
	gboolean         changed = FALSE;

	relation = MRP_RELATION (object);
	priv = mrp_relation_get_instance_private (relation);

	switch (prop_id) {
	case PROP_SUCCESSOR:
		priv->successor = g_value_get_object (value);
		if (priv->successor) {
			g_object_ref (priv->successor);
			changed = TRUE;
		}
		break;

	case PROP_PREDECESSOR:
		priv->predecessor = g_value_get_object (value);
		if (priv->predecessor) {
			g_object_ref (priv->predecessor);
			changed = TRUE;
		}
		break;

	case PROP_TYPE:
		priv->type = (MrpRelationType) g_value_get_enum (value);
		changed = TRUE;
		break;

	case PROP_LAG:
		priv->lag = g_value_get_int (value);
		changed = TRUE;
		break;

	default:
		break;
	}

	if (changed) {
		task = mrp_relation_get_predecessor (relation);
		/* If we get called from the constructor, we don't always have
		 * one of these.
		 */
		if (task == NULL) {
			task = mrp_relation_get_successor (relation);
		}
		if (task != NULL) {
			mrp_object_changed (MRP_OBJECT (task));
		}
	}
}

static void
mrp_relation_get_property (GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
	MrpRelation     *relation;
	MrpRelationPrivate *priv;

	relation = MRP_RELATION (object);
	priv = mrp_relation_get_instance_private (relation);

	switch (prop_id) {
	case PROP_SUCCESSOR:
		g_value_set_object (value, priv->successor);
		break;

	case PROP_PREDECESSOR:
		g_value_set_object (value, priv->predecessor);
		break;

	case PROP_TYPE:
		g_value_set_enum (value, priv->type);
		break;

	case PROP_LAG:
		g_value_set_int (value, priv->lag);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
mrp_relation_class_init (MrpRelationClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;

	object_class->finalize     = mrp_relation_finalize;
	object_class->set_property = mrp_relation_set_property;
	object_class->get_property = mrp_relation_get_property;

    /**
     * MrpRelation::changed:
     * @relation: an #MrpRelation.
     *
     * emitted when @relation changes.
     */
	signals[CHANGED] = g_signal_new
		("changed",
		 G_TYPE_FROM_CLASS (klass),
		 G_SIGNAL_RUN_LAST,
		 0, /*G_STRUCT_OFFSET (MrpRelationClass, method), */
		 NULL, NULL,
		 mrp_marshal_VOID__VOID,
		 G_TYPE_NONE,
		 0);

	/* Properties. */
	g_object_class_install_property (object_class,
					 PROP_SUCCESSOR,
					 g_param_spec_object ("successor",
							      "Successor",
							      "The successor in the relation",
							      MRP_TYPE_TASK,
							      G_PARAM_READWRITE |
							      G_PARAM_CONSTRUCT_ONLY));
	g_object_class_install_property (object_class,
					 PROP_PREDECESSOR,
					 g_param_spec_object ("predecessor",
							      "Predecessor",
							      "The predecessor in the relation",
							      MRP_TYPE_TASK,
							      G_PARAM_READWRITE |
							      G_PARAM_CONSTRUCT_ONLY));

	g_object_class_install_property (object_class,
					 PROP_TYPE,
					 g_param_spec_enum ("type",
							    "Type",
							    "The type of relation",
							    MRP_TYPE_RELATION_TYPE,
							    MRP_RELATION_FS,
							    G_PARAM_READWRITE));

	g_object_class_install_property (object_class,
					 PROP_LAG,
					 g_param_spec_int ("lag",
							   "Lag",
							   "Lag between the predecessor and successor",
							   -G_MAXINT, G_MAXINT, 0,
							   G_PARAM_READWRITE));
}

static void
mrp_relation_init (MrpRelation *relation)
{
	MrpRelationPrivate *priv = mrp_relation_get_instance_private (relation);

	priv->type = MRP_RELATION_FS;
	priv->lag  = 0;
}

/**
 * mrp_relation_get_predecessor:
 * @relation: an #MrpRelation
 *
 * Retrieves the predecessor of @relation.
 *
 * Return value: the predecessor task.
 **/
MrpTask *
mrp_relation_get_predecessor (MrpRelation *relation)
{
	MrpRelationPrivate *priv = mrp_relation_get_instance_private (relation);

	g_return_val_if_fail (MRP_IS_RELATION (relation), NULL);

	return priv->predecessor;
}

/**
 * mrp_relation_get_successor:
 * @relation: an #MrpRelation
 *
 * Retrieves the successor of @relation.
 *
 * Return value: the successor task.
 **/
MrpTask *
mrp_relation_get_successor (MrpRelation *relation)
{
	MrpRelationPrivate *priv = mrp_relation_get_instance_private (relation);

	g_return_val_if_fail (MRP_IS_RELATION (relation), NULL);

	return priv->successor;
}

/**
 * mrp_relation_get_lag:
 * @relation: an #MrpRelation
 *
 * Retrieves the lag between the predecessor and successor in @relation.
 *
 * Return value: Lag time in seconds.
 **/
gint
mrp_relation_get_lag (MrpRelation *relation)
{
	MrpRelationPrivate *priv = mrp_relation_get_instance_private (relation);

	g_return_val_if_fail (MRP_IS_RELATION (relation), 0);

	return priv->lag;
}

/**
 * mrp_relation_get_relation_type:
 * @relation: an #MrpRelation
 *
 * Retrieves the relation type of @relation.
 *
 * Return value: the #MrpRelationType of the relation.
 **/
/* Cumbersome name, but mrp_relation_get_type is already taken :) */
MrpRelationType
mrp_relation_get_relation_type (MrpRelation *relation)
{
	MrpRelationPrivate *priv = mrp_relation_get_instance_private (relation);

	g_return_val_if_fail (MRP_IS_RELATION (relation), MRP_RELATION_NONE);

	return priv->type;
}
