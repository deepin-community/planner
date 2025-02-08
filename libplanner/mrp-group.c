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

/**
 * SECTION:mrp-group
 * @Short_Description: resource groups.
 * @Title: MrpGroup
 * @include: libplanner/mrp-group.h
 *
 * Resources can be grouped.
 * An #MrpResource can refer to a resource group as his.
 *
 * A resource group has got a name.
 * A resource group has got a manager.
 *
 * An #MrpProject maintains a list of resource groups and can designates one as
 * the default. If you specify a default group, every new #MrpResource that you
 * add will be placed in this group.
 */

#include <config.h>
#include "string.h"
#include <glib/gi18n.h>
#include "mrp-group.h"

struct _MrpGroup {
	MrpObject parent_instance;
};

typedef struct {
	gchar *name;
	gchar *manager_name;
	gchar *manager_phone;
	gchar *manager_email;
} MrpGroupPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (MrpGroup, mrp_group, MRP_TYPE_OBJECT)

/* Properties */
enum {
        PROP_0,
        PROP_NAME,
        PROP_MANAGER_NAME,
        PROP_MANAGER_PHONE,
	PROP_MANAGER_EMAIL
};

static void
mrp_group_finalize (GObject *object)
{
        MrpGroup     *group = MRP_GROUP (object);
	MrpGroupPrivate *priv = mrp_group_get_instance_private (group);

        g_free (priv->name);
        priv->name = NULL;

        g_free (priv->manager_name);
        priv->manager_name = NULL;

        g_free (priv->manager_phone);
        priv->manager_phone = NULL;

        g_free (priv->manager_email);
        priv->manager_email = NULL;

	G_OBJECT_CLASS (mrp_group_parent_class)->finalize (object);
}

static void
mrp_group_set_property (GObject      *object,
                        guint         prop_id,
                        const GValue *value,
                        GParamSpec   *pspec)
{
	MrpGroup     *group;
	MrpGroupPrivate *priv;
	gboolean      changed = FALSE;
	const gchar  *str;

	g_return_if_fail (MRP_IS_GROUP (object));

	group = MRP_GROUP (object);
	priv = mrp_group_get_instance_private (group);

	switch (prop_id) {
	case PROP_NAME:
		str = g_value_get_string (value);

		if (!priv->name || strcmp (priv->name, str)) {
			g_free (priv->name);
			priv->name = g_strdup (str);
			changed = TRUE;
		}

		break;
	case PROP_MANAGER_NAME:
		str = g_value_get_string (value);

		if (!priv->manager_name || strcmp (priv->manager_name, str)) {
			g_free (priv->manager_name);
			priv->manager_name = g_strdup (str);
			changed = TRUE;
		}

		break;
	case PROP_MANAGER_PHONE:
		str = g_value_get_string (value);

		if (!priv->manager_phone || strcmp (priv->manager_phone, str)) {
			g_free (priv->manager_phone);
			priv->manager_phone = g_strdup (str);
			changed = TRUE;
		}

		break;
	case PROP_MANAGER_EMAIL:
		str = g_value_get_string (value);

		if (!priv->manager_email || strcmp (priv->manager_email, str)) {
			g_free (priv->manager_email);
			priv->manager_email = g_strdup (str);
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
mrp_group_get_property (GObject    *object,
                        guint       prop_id,
                        GValue     *value,
                        GParamSpec *pspec)
{
	MrpGroup     *group;
	MrpGroupPrivate *priv;

	g_return_if_fail (MRP_IS_GROUP (object));

	group = MRP_GROUP (object);
	priv = mrp_group_get_instance_private (group);

	switch (prop_id) {
	case PROP_NAME:
		g_value_set_string (value, priv->name);
		break;
	case PROP_MANAGER_NAME:
		g_value_set_string (value, priv->manager_name);
		break;
	case PROP_MANAGER_PHONE:
		g_value_set_string (value, priv->manager_phone);
		break;
	case PROP_MANAGER_EMAIL:
		g_value_set_string (value, priv->manager_email);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}


static void
mrp_group_class_init (MrpGroupClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);

        object_class->finalize     = mrp_group_finalize;
        object_class->set_property = mrp_group_set_property;
        object_class->get_property = mrp_group_get_property;

        g_object_class_install_property (object_class,
                                         PROP_NAME,
                                         g_param_spec_string ("name",
                                                              "Name",
                                                              "Name of the group",
                                                              "empty",
                                                              G_PARAM_READWRITE));
        g_object_class_install_property (object_class,
                                         PROP_MANAGER_NAME,
                                         g_param_spec_string ("manager_name",
							      "Manager Name",
							      "The name of the group manager",
							      "empty",
							      G_PARAM_READWRITE));
        g_object_class_install_property (object_class,
                                         PROP_MANAGER_PHONE,
                                         g_param_spec_string ("manager_phone",
							      "Manager Phone",
							      "The phone number of the group manager",
							      "empty",
							      G_PARAM_READWRITE));

        g_object_class_install_property (object_class,
                                         PROP_MANAGER_EMAIL,
                                         g_param_spec_string ("manager_email",
							      "Manager Email",
							      "The email address of the group manager",
							      "empty",
							      G_PARAM_READWRITE));

}


static void
mrp_group_init (MrpGroup *group)
{
	MrpGroupPrivate *priv = mrp_group_get_instance_private (group);

	priv->name          = g_strdup ("");
	priv->manager_name  = g_strdup ("");
	priv->manager_phone = g_strdup ("");
	priv->manager_email = g_strdup ("");
}

/**
 * mrp_group_new:
 *
 * Creates a new end group.
 *
 * Return value: the newly created group.
 **/
MrpGroup *
mrp_group_new (void)
{
        MrpGroup *group;

        group = g_object_new (MRP_TYPE_GROUP,
			      "name", "",
			      NULL);

        return group;
}

/**
 * mrp_group_get_name:
 * @group: an #MrpGroup
 *
 * Retrieves the name of @group.
 *
 * Return value: the name
 **/
const gchar *
mrp_group_get_name (MrpGroup *group)
{
	MrpGroupPrivate *priv = mrp_group_get_instance_private (group);

	g_return_val_if_fail (MRP_IS_GROUP (group), NULL);

	return priv->name;
}

/**
 * mrp_group_set_name:
 * @group: an #MrpGroup
 * @name: new name of @group
 *
 * Sets the name of @group.
 **/
void
mrp_group_set_name (MrpGroup *group, const gchar *name)
{
	g_return_if_fail (MRP_IS_GROUP (group));
	g_return_if_fail (name != NULL);

	mrp_object_set (MRP_OBJECT (group), "name", name, NULL);
}
