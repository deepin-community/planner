/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2002 CodeFactory AB
 * Copyright (C) 2002 Richard Hult <richard@imendio.com>
 * Copyright (C) 2002 Mikael Hallendal <micke@imendio.com>
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

#pragma once

#include <glib-object.h>

G_BEGIN_DECLS

#define MRP_TYPE_APPLICATION         (mrp_application_get_type ())

G_DECLARE_DERIVABLE_TYPE (MrpApplication, mrp_application, MRP, APPLICATION, GObject)

/**
 * MrpApplication:
 *
 * Object representing the application using libmrproject. You need to
 * create an #MrpApplication object to create projects and use
 * libmrproject.
 */
struct _MrpApplicationClass {
	GObjectClass        parent_class;
};


/* General functions.
 */

MrpApplication * mrp_application_new           (void);
guint            mrp_application_get_unique_id (void);
gpointer         mrp_application_id_get_data   (guint object_id);

G_END_DECLS
