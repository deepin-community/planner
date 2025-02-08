/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2004 Imendio AB
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

#include <config.h>
#include <gmodule.h>
#include "mrp-storage-module.h"
#include "mrp-project.h"
#include "mrp-private.h"

G_DEFINE_TYPE (MrpStorageModule, mrp_storage_module, G_TYPE_OBJECT)


static void
mrp_storage_module_init (MrpStorageModule *module)
{
}

static void
mrp_storage_module_class_init (MrpStorageModuleClass *klass)
{
}

gboolean
mrp_storage_module_load (MrpStorageModule  *module,
			 const gchar       *uri,
			 GError           **error)
{
	g_return_val_if_fail (MRP_IS_STORAGE_MODULE (module), FALSE);
	g_return_val_if_fail (uri != NULL, FALSE);

	if (MRP_STORAGE_MODULE_GET_CLASS (module)->load) {
		return MRP_STORAGE_MODULE_GET_CLASS (module)->load (module,
								    uri,
								    error);
	}

	return FALSE;
}

gboolean
mrp_storage_module_save (MrpStorageModule  *module,
			 const gchar       *uri,
			 gboolean           force,
			 GError           **error)
{
	g_return_val_if_fail (MRP_IS_STORAGE_MODULE (module), FALSE);
	g_return_val_if_fail (uri != NULL, FALSE);

	if (MRP_STORAGE_MODULE_GET_CLASS (module)->save) {
		return MRP_STORAGE_MODULE_GET_CLASS (module)->save (module,
								    uri,
								    force,
								    error);
	}

	return FALSE;
}

gboolean
mrp_storage_module_to_xml (MrpStorageModule  *module,
			   gchar            **str,
			   GError           **error)
{
	g_return_val_if_fail (MRP_IS_STORAGE_MODULE (module), FALSE);

	if (MRP_STORAGE_MODULE_GET_CLASS (module)->to_xml) {
		return MRP_STORAGE_MODULE_GET_CLASS (module)->to_xml (module,
								      str,
								      error);
	}

	return FALSE;
}

gboolean
mrp_storage_module_from_xml (MrpStorageModule  *module,
			     const gchar       *str,
			     GError           **error)
{
	g_return_val_if_fail (MRP_IS_STORAGE_MODULE (module), FALSE);
	g_return_val_if_fail (str != NULL, FALSE);

	if (MRP_STORAGE_MODULE_GET_CLASS (module)->from_xml) {
		return MRP_STORAGE_MODULE_GET_CLASS (module)->from_xml (module,
									str,
									error);
	}

	return FALSE;
}

void
imrp_storage_module_set_project (MrpStorageModule *module,
				 MrpProject       *project)
{
	g_return_if_fail (MRP_IS_STORAGE_MODULE (module));
	g_return_if_fail (MRP_IS_PROJECT (project));

	if (MRP_STORAGE_MODULE_GET_CLASS (module)->set_project) {
		MRP_STORAGE_MODULE_GET_CLASS (module)->set_project (module,
								    project);
	}
}

