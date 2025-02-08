/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
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

#include <config.h>
#include <gmodule.h>
#include "mrp-paths.h"
#include "mrp-storage-module-factory.h"
#include "mrp-storage-module.h"

static GHashTable       *module_hash = NULL;

typedef struct {
	GModule       *library;
	gchar         *name;

	/* Initialization and uninitialization. */
	void		  (* init)	(GTypeModule      *module);
	void		  (* exit)	(MrpStorageModule *module);
	MrpStorageModule *(* new)	(void);
} MrpStorageModuleFactoryPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (MrpStorageModuleFactory, mrp_storage_module_factory, G_TYPE_TYPE_MODULE)

static gboolean
mrp_storage_module_factory_load (GTypeModule *module)
{
	MrpStorageModuleFactory *factory = MRP_STORAGE_MODULE_FACTORY (module);
	MrpStorageModuleFactoryPrivate *priv = mrp_storage_module_factory_get_instance_private (factory);

	priv->library = g_module_open (priv->name, 0);
	if (!priv->library) {
		g_warning ("%s", g_module_error ());
		return FALSE;
	}

	/* These must be implemented by all storage modules. */
	if (!g_module_symbol (priv->library, "module_init", (gpointer)&priv->init) ||
	    !g_module_symbol (priv->library, "module_new", (gpointer)&priv->new) ||
	    !g_module_symbol (priv->library, "module_exit", (gpointer)&priv->exit)) {
		g_warning ("%s", g_module_error ());
		g_module_close (priv->library);

		return FALSE;
	}

	priv->init (module);

	return TRUE;
}

static void
mrp_storage_module_factory_unload (GTypeModule *module)
{
	MrpStorageModuleFactory *factory = MRP_STORAGE_MODULE_FACTORY (module);
	MrpStorageModuleFactoryPrivate *priv = mrp_storage_module_factory_get_instance_private (factory);

	/*g_print ("Unload '%s'\n", factory->name);*/

	/* FIXME: should pass the module here somehow. */
	/*factory->exit (NULL); */

	g_module_close (priv->library);
	priv->library = NULL;

	priv->init = NULL;
	priv->exit = NULL;
}

static void
mrp_storage_module_factory_class_init (MrpStorageModuleFactoryClass *klass)
{
	GTypeModuleClass *module_class = G_TYPE_MODULE_CLASS (klass);

	module_class->load = mrp_storage_module_factory_load;
	module_class->unload = mrp_storage_module_factory_unload;
}

static void
mrp_storage_module_factory_init (MrpStorageModuleFactory *factory)
{
}


MrpStorageModuleFactory *
mrp_storage_module_factory_get (const gchar *name)
{
	MrpStorageModuleFactory *factory;
	gchar                   *fullname, *libname;
	gchar                   *path;

	fullname = g_strconcat ("storage-", name, NULL);

	path = mrp_paths_get_storagemodule_dir (NULL);
	libname = g_module_build_path (path, fullname);
	g_free (path);

	if (!module_hash) {
		module_hash = g_hash_table_new (g_str_hash, g_str_equal);
	}

	factory = g_hash_table_lookup (module_hash, libname);

	if (!factory) {
		factory = g_object_new (MRP_TYPE_STORAGE_MODULE_FACTORY, NULL);
		MrpStorageModuleFactoryPrivate *priv = mrp_storage_module_factory_get_instance_private (factory);
		g_type_module_set_name (G_TYPE_MODULE (factory), libname);
		priv->name = libname;

		g_hash_table_insert (module_hash, priv->name, factory);
	}

	g_free (fullname);

	if (!g_type_module_use (G_TYPE_MODULE (factory))) {
		return NULL;
	}

	return factory;
}

MrpStorageModule *
mrp_storage_module_factory_create_module (MrpStorageModuleFactory *factory)
{
	MrpStorageModule *module;
	MrpStorageModuleFactoryPrivate *priv = mrp_storage_module_factory_get_instance_private (factory);

	module = priv->new ();

	return module;
}


