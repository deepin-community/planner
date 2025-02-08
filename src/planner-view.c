/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2005 Imendio AB
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
#include "planner-view.h"
#include "planner-conf.h"

G_DEFINE_TYPE (PlannerView, planner_view, G_TYPE_OBJECT);

static void
planner_view_class_init (PlannerViewClass *klass)
{
}

static void
planner_view_init (PlannerView *view)
{
}

const gchar *
planner_view_get_label (PlannerView *view)
{
	g_return_val_if_fail (PLANNER_IS_VIEW (view), NULL);

	if (PLANNER_VIEW_GET_CLASS (view)->get_label) {
		return PLANNER_VIEW_GET_CLASS (view)->get_label (view);
	}

	return NULL;
}

const gchar *
planner_view_get_menu_label (PlannerView *view)
{
	g_return_val_if_fail (PLANNER_IS_VIEW (view), NULL);

	if (PLANNER_VIEW_GET_CLASS (view)->get_menu_label) {
		return PLANNER_VIEW_GET_CLASS (view)->get_menu_label (view);
	}

	return NULL;
}

const gchar *
planner_view_get_icon (PlannerView *view)
{
	g_return_val_if_fail (PLANNER_IS_VIEW (view), NULL);

	if (PLANNER_VIEW_GET_CLASS (view)->get_icon) {
		return PLANNER_VIEW_GET_CLASS (view)->get_icon (view);
	}

	return NULL;
}

const gchar *
planner_view_get_name (PlannerView *view)
{
	g_return_val_if_fail (PLANNER_IS_VIEW (view), NULL);

	if (PLANNER_VIEW_GET_CLASS (view)->get_name) {
		return PLANNER_VIEW_GET_CLASS (view)->get_name (view);
	}

	return NULL;
}

GtkWidget *
planner_view_get_widget (PlannerView *view)
{
	g_return_val_if_fail (PLANNER_IS_VIEW (view), NULL);

	if (PLANNER_VIEW_GET_CLASS (view)->get_widget) {
		return PLANNER_VIEW_GET_CLASS (view)->get_widget (view);
	}

	return NULL;
}

void
planner_view_setup (PlannerView   *view,
		   PlannerWindow *main_window)
{
	g_return_if_fail (PLANNER_IS_VIEW (view));

	view->main_window = main_window;

	if (PLANNER_VIEW_GET_CLASS (view)->setup) {
		PLANNER_VIEW_GET_CLASS (view)->setup (view, main_window);
	}
}

void
planner_view_activate (PlannerView *view)
{
	g_return_if_fail (PLANNER_IS_VIEW (view));

	view->activated = TRUE;

	if (PLANNER_VIEW_GET_CLASS (view)->activate) {
		PLANNER_VIEW_GET_CLASS (view)->activate (view);
	}
}

void
planner_view_deactivate (PlannerView *view)
{
	g_return_if_fail (PLANNER_IS_VIEW (view));

	view->activated = FALSE;

	if (PLANNER_VIEW_GET_CLASS (view)->deactivate) {
		PLANNER_VIEW_GET_CLASS (view)->deactivate (view);
	}
}

void
planner_view_print_init (PlannerView     *view,
			 PlannerPrintJob *job)
{
	g_return_if_fail (PLANNER_IS_VIEW (view));
	g_return_if_fail (PLANNER_IS_PRINT_JOB (job));

	if (PLANNER_VIEW_GET_CLASS (view)->print_init) {
		PLANNER_VIEW_GET_CLASS (view)->print_init (view, job);
	}
}

gint
planner_view_print_get_n_pages (PlannerView *view)
{
	g_return_val_if_fail (PLANNER_IS_VIEW (view), 0);

	if (PLANNER_VIEW_GET_CLASS (view)->print_get_n_pages) {
		return PLANNER_VIEW_GET_CLASS (view)->print_get_n_pages (view);
	}

	return 0;
}

void
planner_view_print (PlannerView *view, gint page_nr)
{
	g_return_if_fail (PLANNER_IS_VIEW (view));

	if (PLANNER_VIEW_GET_CLASS (view)->print) {
		PLANNER_VIEW_GET_CLASS (view)->print (view, page_nr);
	}
}

void
planner_view_print_cleanup (PlannerView *view)
{
	g_return_if_fail (PLANNER_IS_VIEW (view));

	if (PLANNER_VIEW_GET_CLASS (view)->print_cleanup) {
		PLANNER_VIEW_GET_CLASS (view)->print_cleanup (view);
	}
}

static gint
column_sort_func (gconstpointer a,
		  gconstpointer b)
{
	gint   order_a, order_b;

	order_a = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (a), "order"));
	order_b = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (b), "order"));

	if (order_a < order_b) {
		return -1;
	}
	else if (order_a > order_b) {
		return 1;
	} else {
		return 0;
	}
}

void
planner_view_column_load_helper (PlannerView *view,
				 GtkTreeView *tree,
				 const gchar *set)
{
	GList             *columns, *l;
	GSettings         *settings, *child;
	GtkTreeViewColumn *column;
	const gchar       *id;
	gboolean           visible, order;
	gint               width;

	columns = gtk_tree_view_get_columns (tree);
	for (l = columns; l; l = l->next) {
		column = l->data;

		if (g_object_get_data (G_OBJECT (column), "custom")) {
			continue;
		}

		id = g_object_get_data (G_OBJECT (column), "id");
		if (!id) {
			continue;
		}

		settings = planner_conf_get_gsettings (set);
		child = g_settings_get_child (settings, id);

		visible = g_settings_get_boolean (child, "visible");
		gtk_tree_view_column_set_visible (column, visible);

		order = g_settings_get_int (child, "order");
		g_object_set_data (G_OBJECT (column), "order", GINT_TO_POINTER (order));

		width = g_settings_get_int (child, "width");
		if (width > 0) {
			gtk_tree_view_column_set_fixed_width (column, width);
		}
		g_object_unref (child);
	}

	/* Sort the list of columns in the right order. */
	columns = g_list_sort (columns, column_sort_func);

	for (l = columns; l; l = l->next) {
		column = l->data;

		gtk_tree_view_move_column_after (tree,
						 column,
						 l->prev ? l->prev->data : NULL);
	}

	g_list_free (columns);
}

void
planner_view_column_save_helper (PlannerView *view,
				 GtkTreeView *tree,
				 const gchar *set)
{
	GList             *columns, *l;
	GSettings         *settings, *child;
	GtkTreeViewColumn *column;
	const gchar       *id;
	gint               i;

	columns = gtk_tree_view_get_columns (tree);
	for (i = 0, l = columns; l; i++, l = l->next) {
		column = l->data;

		if (g_object_get_data (G_OBJECT (column), "custom")) {
			continue;
		}

		id = g_object_get_data (G_OBJECT (column), "id");
		if (!id) {
			continue;
		}

		settings = planner_conf_get_gsettings (set);
		child = g_settings_get_child (settings, id);
		g_settings_delay (child);

		g_settings_set_boolean (child, "visible",
					gtk_tree_view_column_get_visible (column));

		g_settings_set_int (child, "order", i);

		g_settings_set_int (child, "width",
				    gtk_tree_view_column_get_width (column));

		g_settings_apply (child);
		g_object_unref (child);
	}

	g_list_free (columns);
}
