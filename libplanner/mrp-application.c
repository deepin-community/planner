/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2002 CodeFactory AB
 * Copyright (C) 2002 Richard Hult <richard@imendio.com>
 * Copyright (C) 2002 Mikael Hallendal <micke@imendio.com>
 * Copyright (C) 2004 Alvaro del Castillo <acs@barrapunto.com>
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

/**
 * SECTION:mrp-application
 * @Short_description: the main project application object.
 * @Title: MrpApplication
 * @include: libplanner/mrp-application.h
 *
 * #MrpApplication is the libplanner infrastructure.
 * It loads #GModule plug-ins, registers file readers and writers.
 *
 * One must instantiate an #MrpApplication to use libplanner.
 * A sole one can be instantiated.
 *
 * #MrpApplication also features a unique identifier generator.
 * mrp_application_get_unique_id() generates a unique id within the application.
 *
 * #MrpApplication is able to register pointers against an id.
 * mrp_application_id_get_data() retrieves the pointer given the id.
 * Every #MrpObject registers itself this way.
 */

#include <config.h>

#include <stdlib.h>

#include <glib/gi18n.h>
#include "mrp-file-module.h"
#include "mrp-private.h"
#include "mrp-application.h"
#include "mrp-paths.h"

typedef struct {
	GList *file_readers;
	GList *file_writers;
	GList *modules;
} MrpApplicationPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (MrpApplication, mrp_application, G_TYPE_OBJECT)

static GObjectClass *parent_class;
static guint         last_used_id;
static GHashTable   *data_hash;

static void
mrp_application_finalize_file_modules (MrpApplication *app)
{
	MrpApplicationPrivate *priv = mrp_application_get_instance_private (app);
	g_list_foreach (priv->file_readers, (GFunc) g_free, NULL);
	g_list_free (priv->file_readers);
	priv->file_readers = NULL;

	g_list_foreach (priv->file_writers, (GFunc) g_free, NULL);
	g_list_free (priv->file_writers);
	priv->file_writers = NULL;

	g_list_foreach (priv->modules, (GFunc) g_free, NULL);
	g_list_free (priv->modules);
	priv->modules = NULL;
}

static void
mrp_application_finalize (GObject *object)
{
	MrpApplication *app = MRP_APPLICATION (object);

	mrp_application_finalize_file_modules (app);

	if (parent_class->finalize) {
		parent_class->finalize (object);
	}
}

static void
mrp_application_class_init (MrpApplicationClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	parent_class = G_OBJECT_CLASS (g_type_class_peek_parent (klass));

	object_class->finalize = mrp_application_finalize;

	data_hash = g_hash_table_new (NULL, NULL);

	last_used_id = 0;

}

static void
mrp_application_init_gettext (void)
{
	gchar *locale_dir;

	locale_dir = mrp_paths_get_locale_dir ();
	bindtextdomain (GETTEXT_PACKAGE, locale_dir);
	g_free(locale_dir);

	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
}

static void
mrp_application_init_file_modules (MrpApplication *app)
{
	MrpApplicationPrivate *priv = mrp_application_get_instance_private (app);
	priv->modules = mrp_file_module_load_all (app);
}

static void
mrp_application_init (MrpApplication *app)
{
	MrpApplicationPrivate *priv;
	static gboolean     first = TRUE;

	if (!first) {
		g_error ("You can only create one instance of MrpApplication");
		exit (1);
	}

	priv = mrp_application_get_instance_private (app);
	priv->file_readers = NULL;
	priv->file_writers = NULL;

	mrp_application_init_gettext ();
	mrp_application_init_file_modules (app);

	first = FALSE;
}

void
mrp_application_register_reader (MrpApplication *app, MrpFileReader *reader)
{
	MrpApplicationPrivate *priv = mrp_application_get_instance_private (app);

	g_return_if_fail (MRP_IS_APPLICATION (app));
	g_return_if_fail (reader != NULL);

	priv->file_readers = g_list_prepend (priv->file_readers, reader);
}

void
mrp_application_register_writer (MrpApplication *app, MrpFileWriter *writer)
{
	MrpApplicationPrivate *priv = mrp_application_get_instance_private (app);

	g_return_if_fail (MRP_IS_APPLICATION (app));
	g_return_if_fail (writer != NULL);

	priv->file_writers = g_list_prepend (priv->file_writers, writer);
}

GList *
mrp_application_get_all_file_readers (MrpApplication *app)
{
	MrpApplicationPrivate *priv = mrp_application_get_instance_private (app);

	g_return_val_if_fail (MRP_IS_APPLICATION (app), NULL);

	return priv->file_readers;
}

GList *
mrp_application_get_all_file_writers (MrpApplication *app)
{
	MrpApplicationPrivate *priv = mrp_application_get_instance_private (app);

	g_return_val_if_fail (MRP_IS_APPLICATION (app), NULL);

	return priv->file_writers;
}

/**
 * mrp_application_new:
 *
 * Creates a new #MrpApplication.
 *
 * Return value: the newly created application
 **/
MrpApplication *
mrp_application_new (void)
{
	return g_object_new (MRP_TYPE_APPLICATION, NULL);
}

/**
 * mrp_application_get_unique_id:
 *
 * Returns a unique identifier in the #MrpApplication namespace.
 *
 * Return value: the unique id
 **/
guint
mrp_application_get_unique_id (void)
{
	return ++last_used_id;
}

/*
 * imrp_application_id_set_data:
 *
 * Set the data unique identifier for a data
 *
 * Return value: TRUE if the change has been done
 **/
gboolean
mrp_application_id_set_data (gpointer data,
			      guint    data_id)
{
	g_assert (g_hash_table_lookup (data_hash, GUINT_TO_POINTER (data_id)) == NULL);

	g_hash_table_insert (data_hash, GUINT_TO_POINTER (data_id), data);

	return TRUE;
}



/**
 * mrp_application_id_get_data:
 * @object_id: an object id
 *
 * Get the object reference in the list of MrpObjects
 * using the object_id as locator.
 *
 * Return value: a pointer to the data
 **/
gpointer
mrp_application_id_get_data (guint object_id)
{
	return g_hash_table_lookup (data_hash, GUINT_TO_POINTER (object_id));
}

/*
 * mrp_application_id_get_data:
 * @object_id: an object id
 *
 * Get the object reference in the list of MrpObjects
 * using the object_id as locator
 *
 * Return value: a pointer to the data
 **/
gboolean
mrp_application_id_remove_data (guint object_id)
{
	return g_hash_table_remove (data_hash, GUINT_TO_POINTER (object_id));
}
