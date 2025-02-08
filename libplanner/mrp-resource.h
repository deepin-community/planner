/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2004 Imendio AB
 * Copyright (C) 2002 CodeFactory AB
 * Copyright (C) 2002 Richard Hult <richard@imendio.com>
 * Copyright (C) 2002 Mikael Hallendal <micke@imendio.com>
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

#include <libplanner/mrp-object.h>
#include <libplanner/mrp-task.h>
#include <libplanner/mrp-calendar.h>

G_BEGIN_DECLS

#define MRP_TYPE_RESOURCE         (mrp_resource_get_type ())

G_DECLARE_FINAL_TYPE (MrpResource, mrp_resource, MRP, RESOURCE, MrpObject)

/**
 * MrpResourceType:
 * @MRP_RESOURCE_TYPE_NONE: invalid type (unset)
 * @MRP_RESOURCE_TYPE_WORK: work resource
 * @MRP_RESOURCE_TYPE_MATERIAL: material resource
 *
 * The type of the resource, work or material.
 */
typedef enum {
	MRP_RESOURCE_TYPE_NONE,
	MRP_RESOURCE_TYPE_WORK,
	MRP_RESOURCE_TYPE_MATERIAL
} MrpResourceType;

MrpResource *mrp_resource_new                (void);
const gchar *mrp_resource_get_name           (MrpResource   *resource);
void         mrp_resource_set_name           (MrpResource   *resource,
                                              const gchar   *name);
const gchar *mrp_resource_get_short_name     (MrpResource   *resource);
void         mrp_resource_set_short_name     (MrpResource   *resource,
                                              const gchar   *short_name);
void         mrp_resource_assign             (MrpResource   *resource,
                                              MrpTask       *task,
                                              gint           units);
GList       *mrp_resource_get_assignments    (MrpResource   *resource);
GList       *mrp_resource_get_assigned_tasks (MrpResource   *resource);
gint         mrp_resource_compare            (gconstpointer  a,
                                              gconstpointer  b);
MrpCalendar *mrp_resource_get_calendar       (MrpResource   *resource);
void         mrp_resource_set_calendar       (MrpResource   *resource,
                                              MrpCalendar   *calendar);

G_END_DECLS
