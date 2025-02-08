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

#include <config.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include "libplanner/mrp-paths.h"
#include "planner-marshal.h"
#include "planner-resource-input-dialog.h"
#include "planner-resource-cmd.h"
#include "planner-util.h"

typedef struct {
	MrpProject *project;

	PlannerWindow *main_window;
	GtkWidget     *name_entry;
	GtkWidget     *short_name_entry;
	GtkWidget     *email_entry;
	GtkWidget     *group_combo_box;
} DialogData;

static void resource_input_dialog_setup_groups (DialogData *data);


static void
resource_input_dialog_group_changed (MrpGroup   *group,
				     GParamSpec *spec,
				     DialogData *data)
{
	/* Seems like the easiest way is to rebuild the option menu. */
	resource_input_dialog_setup_groups (data);
}

static MrpGroup *
resource_input_dialog_get_selected (GtkWidget *combo)
{
	GtkTreeIter iter;
	MrpGroup  *group;

	gtk_combo_box_get_active_iter (GTK_COMBO_BOX (combo), &iter);
	gtk_tree_model_get (gtk_combo_box_get_model (GTK_COMBO_BOX (combo)),
			    &iter,
			    0, &group,
			    -1);
	return group;
}

static void
menu_groups_name (GtkCellLayout   *layout,
                  GtkCellRenderer *cell,
                  GtkTreeModel    *model,
                  GtkTreeIter     *iter,
                  gpointer         user_data)
{
	MrpGroup *group;
	const gchar *name;

	gtk_tree_model_get (model, iter, 0, &group, -1);
	if (!group) {
		name = _("(None)");
	} else {
		name = mrp_group_get_name (group);
		if (!name) {
			name = _("(No name)");
		}
	}
	g_object_set (cell,
		      "text", name,
		      NULL);
}

static void
resource_input_dialog_setup_groups (DialogData *data)
{
	MrpGroup     *selected_group;
	GList        *groups;
	GtkComboBox  *combo_box;
	GtkListStore *store;
	GtkTreeIter   iter;
	GList        *l;
	gint          index;

	selected_group = resource_input_dialog_get_selected (data->group_combo_box);

	combo_box = GTK_COMBO_BOX (data->group_combo_box);

	groups = mrp_project_get_groups (data->project);

	store = gtk_list_store_new (1, MRP_TYPE_GROUP);
	gtk_combo_box_set_model (combo_box, GTK_TREE_MODEL (store));
	g_object_unref (store);

	/* Put "no group" at the top. */
	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter,
			    0, NULL,
			    -1);

	for (l = groups; l; l = l->next) {
		MrpGroup *group = l->data;

		gtk_list_store_append (store, &iter);
		gtk_list_store_set (store, &iter,
				    0, group,
				    -1);

		g_signal_connect (group,
				  "notify::name",
				  G_CALLBACK (resource_input_dialog_group_changed),
				  data);
	}

	/* Select the right group. +1 is for the empty group at the top. */
	if (groups != NULL && selected_group != NULL) {
		index = g_list_index (groups, selected_group) + 1;
	} else {
		index = 0;
	}

	gtk_combo_box_set_active (combo_box, index);
}

static void
resource_input_dialog_groups_updated (MrpProject *project,
				      MrpGroup   *dont_care,
				      GtkWidget  *dialog)
{
	DialogData *data;

	data = g_object_get_data (G_OBJECT (dialog), "data");

	resource_input_dialog_setup_groups (data);
}

static void
resource_input_dialog_free (gpointer user_data)
{
	DialogData *data = user_data;

	g_object_unref (data->project);
	g_object_unref (data->main_window);

	g_free (data);
}

static void
resource_input_dialog_response_cb (GtkWidget *button,
				   gint       response,
				   GtkWidget *dialog)
{
	DialogData  *data;
	MrpResource *resource;
	const gchar *name;
	const gchar *short_name;
	const gchar *email;
	MrpGroup    *group;

	switch (response) {
	case GTK_RESPONSE_OK:
		data = g_object_get_data (G_OBJECT (dialog), "data");

		name = gtk_entry_get_text (GTK_ENTRY (data->name_entry));
		short_name = gtk_entry_get_text (GTK_ENTRY (data->short_name_entry));
		email = gtk_entry_get_text (GTK_ENTRY (data->email_entry));

		group = resource_input_dialog_get_selected (data->group_combo_box);

		resource = g_object_new (MRP_TYPE_RESOURCE,
					 "name", name,
					 "short_name", short_name,
					 "email", email,
					 "group", group,
					 NULL);

		/* mrp_project_add_resource (data->project, resource); */
		planner_resource_cmd_insert (data->main_window, resource);

		gtk_entry_set_text (GTK_ENTRY (data->name_entry), "");
		gtk_entry_set_text (GTK_ENTRY (data->short_name_entry), "");
		gtk_entry_set_text (GTK_ENTRY (data->email_entry), "");

		gtk_widget_grab_focus (data->name_entry);
		break;

	case GTK_RESPONSE_DELETE_EVENT:
	case GTK_RESPONSE_CANCEL:
		gtk_widget_destroy (dialog);
		break;

	default:
		g_assert_not_reached ();
		break;
	}
}

static void
resource_input_dialog_activate_cb (GtkWidget *widget, GtkDialog *dialog)
{
	gtk_dialog_response (dialog, GTK_RESPONSE_OK);
}

GtkWidget *
planner_resource_input_dialog_new (PlannerWindow *main_window)
{
	GtkWidget  *dialog;
	DialogData *data;
	GtkBuilder *builder;
	MrpProject *project;
	GtkCellRenderer *renderer;

	project = planner_window_get_project (main_window);

	data = g_new0 (DialogData, 1);

	data->project = g_object_ref (project);
	data->main_window = g_object_ref (main_window);

	builder = gtk_builder_new_from_resource ("/app/drey/Planner/ui/resource-input-dialog.ui");

	dialog = GTK_WIDGET (gtk_builder_get_object (builder, "resource_input_dialog"));
	g_signal_connect (dialog,
			  "response",
			  G_CALLBACK (resource_input_dialog_response_cb),
			  dialog);

	data->name_entry = GTK_WIDGET (gtk_builder_get_object (builder, "name_entry"));
	g_signal_connect (data->name_entry,
			  "activate",
			  G_CALLBACK (resource_input_dialog_activate_cb),
			  dialog);

	data->short_name_entry = GTK_WIDGET (gtk_builder_get_object (builder, "short_name_entry"));
	g_signal_connect (data->short_name_entry,
			  "activate",
			  G_CALLBACK (resource_input_dialog_activate_cb),
			  dialog);

	data->email_entry = GTK_WIDGET (gtk_builder_get_object (builder, "email_entry"));
	g_signal_connect (data->email_entry,
			  "activate",
			  G_CALLBACK (resource_input_dialog_activate_cb),
			  dialog);

	data->group_combo_box = GTK_WIDGET (gtk_builder_get_object (builder, "group_combobox"));

	resource_input_dialog_setup_groups (data);
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (data->group_combo_box),
				    renderer, TRUE);
	gtk_cell_layout_set_cell_data_func (GTK_CELL_LAYOUT (data->group_combo_box),
					    renderer,
					    menu_groups_name,
					    NULL, NULL);

	g_signal_connect_object (project,
				 "group_added",
				 G_CALLBACK (resource_input_dialog_groups_updated),
				 dialog,
				 0);

	g_signal_connect_object (project,
				 "group_removed",
				 G_CALLBACK (resource_input_dialog_groups_updated),
				 dialog,
				 0);

	g_object_set_data_full (G_OBJECT (dialog),
				"data",
				data,
				resource_input_dialog_free);

	g_object_unref (builder);
	return dialog;
}
