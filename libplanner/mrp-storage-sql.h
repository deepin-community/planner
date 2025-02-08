/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2003 CodeFactory AB
 * Copyright (C) 2003 Richard Hult <richard@imendio.com>
 * Copyright (C) 2003 Mikael Hallendal <micke@imendio.com>
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

extern GType mrp_storage_sql_type;

#define MRP_TYPE_STORAGE_SQL		(mrp_storage_sql_get_type())

G_DECLARE_FINAL_TYPE (MrpStorageSQL, mrp_storage_sql, MRP, STORAGE_SQL, MrpStorageModule)

struct _MrpStorageSQL {
	MrpStorageModule  parent;
	MrpProject       *project;
};

struct _MrpStorageSQLClass {
	MrpStorageModuleClass parent_class;
};

G_END_DECLS
