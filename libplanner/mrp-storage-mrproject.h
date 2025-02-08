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
#include "mrp-storage-module.h"
#include "mrp-types.h"
#include "mrp-project.h"

G_BEGIN_DECLS

#define MRP_TYPE_STORAGE_MRPROJECT		(mrp_storage_mrproject_get_type ())

G_DECLARE_FINAL_TYPE (MrpStorageMrproject, mrp_storage_mrproject, MRP, STORAGE_MRPROJECT, MrpStorageModule)

struct _MrpStorageMrproject
{
	MrpStorageModule  parent;

	MrpProject       *project;

	MrpTask    *root_task;
	GHashTable *task_id_hash;
	GList      *delayed_relations;
	GList      *groups;
	GList      *resources;
	GList      *assignments;
	mrptime     project_start;
	MrpGroup   *default_group;
};

struct _MrpStorageMrprojectClass
{
	MrpStorageModuleClass parent_class;
};

G_END_DECLS
