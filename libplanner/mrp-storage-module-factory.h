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

#pragma once

#include <glib-object.h>
#include <gmodule.h>
#include <libplanner/mrp-storage-module.h>
#include <libplanner/mrp-project.h>

G_BEGIN_DECLS

#define MRP_TYPE_STORAGE_MODULE_FACTORY		  (mrp_storage_module_factory_get_type ())

G_DECLARE_DERIVABLE_TYPE (MrpStorageModuleFactory, mrp_storage_module_factory, MRP, STORAGE_MODULE_FACTORY, GTypeModule)

/*struct _MrpStorageModuleFactory
{
	GTypeModule   parent;

	GModule       *library;
	gchar         *name;

};*/

struct _MrpStorageModuleFactoryClass
{
	GTypeModuleClass parent_class;
};

MrpStorageModuleFactory *mrp_storage_module_factory_get	          (const gchar	    	   *name);

MrpStorageModule        *mrp_storage_module_factory_create_module (MrpStorageModuleFactory *factory);

G_END_DECLS
