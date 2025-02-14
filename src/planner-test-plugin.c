/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2002 Alvaro del Castillo <acs@barrapunto.com>
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

#include <config.h>
#include <glib.h>
#include "planner-main-window.h"
#include "planner-plugin.h"

void plugin_init (PlannerPlugin     *plugin);
void plugin_exit (PlannerPlugin     *plugin);

G_MODULE_EXPORT void
plugin_exit (PlannerPlugin     *plugin)
{
	/* g_message ("Test exit"); */
}

G_MODULE_EXPORT void
plugin_init (PlannerPlugin     *plugin)
{
	/* g_message ("Test init"); */
}
