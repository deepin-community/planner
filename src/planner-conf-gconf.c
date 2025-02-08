/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2004 Imendio AB
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
#include <gio/gio.h>

#include "planner-conf.h"

GSettings *
planner_conf_get_gsettings (const gchar *set)
{
	static GSettings *gen_settings = NULL;
	static GSettings *ui_settings = NULL;
	static GSettings *gantt_settings = NULL;
	static GSettings *task_settings = NULL;
	static GSettings *res_settings = NULL;
	static GSettings *usage_settings = NULL;

	if (!strcmp (set, "gen")) {
		if (!gen_settings)
			gen_settings = g_settings_new ("app.drey.Planner");

		return gen_settings;
	}

	if (!strcmp (set, "ui")) {
		if (!ui_settings)
			ui_settings = g_settings_new ("app.drey.Planner.ui");

		return ui_settings;
	}

	if (!strcmp (set, "gantt")) {
		if (!gantt_settings)
			gantt_settings = g_settings_new ("app.drey.Planner.views.gantt-view");

		return gantt_settings;
	}

	if (!strcmp (set, "task")) {
		if (!task_settings)
			task_settings = g_settings_new ("app.drey.Planner.views.task-view");

		return task_settings;
	}

	if (!strcmp (set, "res")) {
		if (!res_settings)
			res_settings = g_settings_new ("app.drey.Planner.views.resource-view");

		return res_settings;
	}

	if (!strcmp (set, "usage")) {
		if (!usage_settings)
			usage_settings = g_settings_new ("app.drey.Planner.views.resource-usage-view");

		return usage_settings;
	}

	return NULL;
}

gboolean
planner_conf_get_bool (const gchar *key, const gchar *set)
{
	GSettings   *settings;
	gboolean     ret_val;

	settings = planner_conf_get_gsettings (set);

	ret_val = g_settings_get_boolean (settings, key);

	return ret_val;
}

gchar *
planner_conf_get_string (const gchar *key, const gchar *set)
{
	GSettings   *settings;
	gchar       *ret_val;

	settings = planner_conf_get_gsettings (set);

	ret_val = g_settings_get_string (settings, key);

	return ret_val;
}

gint
planner_conf_get_int (const gchar *key, const gchar *set)
{
	GSettings   *settings;
	gint         ret_val;

	settings = planner_conf_get_gsettings (set);

	ret_val = g_settings_get_int (settings, key);

	return ret_val;
}

gboolean
planner_conf_set_bool (const gchar *key, gboolean value, const gchar *set)
{
	GSettings   *settings;
	gboolean     ret_val;

	settings = planner_conf_get_gsettings (set);

	ret_val = g_settings_set_boolean (settings, key, value);

	return ret_val;
}

gboolean
planner_conf_set_string (const gchar *key, const gchar *value, const gchar *set)
{
	GSettings   *settings;
	gboolean     ret_val;

	settings = planner_conf_get_gsettings (set);

	ret_val = g_settings_set_string (settings, key, value);

	return ret_val;
}

gboolean
planner_conf_set_int (const gchar *key, gint value, const gchar *set)
{
	GSettings   *settings;
	gboolean     ret_val;

	settings = planner_conf_get_gsettings (set);

	ret_val = g_settings_set_int (settings, key, value);

	return ret_val;
}

