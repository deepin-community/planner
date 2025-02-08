/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
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

#include <glib-object.h>
#include <libplanner/mrp-object.h>
#include <libplanner/mrp-task.h>

G_BEGIN_DECLS

#define MRP_TYPE_RELATION         (mrp_relation_get_type ())

G_DECLARE_FINAL_TYPE (MrpRelation, mrp_relation, MRP, RELATION, MrpObject)

/**
 * MrpRelation:
 *
 * Object representing a predecessor relation between two tasks.
 */

MrpTask         *mrp_relation_get_predecessor      (MrpRelation     *relation);
MrpTask         *mrp_relation_get_successor        (MrpRelation     *relation);
gint             mrp_relation_get_lag              (MrpRelation     *relation);
MrpRelationType  mrp_relation_get_relation_type    (MrpRelation     *relation);

G_END_DECLS
