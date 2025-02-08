/*
 * Copyright (C) 2005 Imendio AB
 * Copyright (C) 2004 Chris Ladd <caladd@particlestorm.net>
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
#include "planner-task-tree.h"
#include "planner-task-popup.h"


static void task_popup_insert_task_cb         (GtkAction *action,
                                               gpointer   callback_data);
static void task_popup_insert_subtask_cb      (GtkAction *action,
                                               gpointer   callback_data);
static void task_popup_remove_task_cb         (GtkAction *action,
                                               gpointer   callback_data);
static void task_popup_edit_task_cb           (GtkAction *action,
                                               gpointer   callback_data);
static void task_popup_edit_task_resources_cb (GtkAction *action,
                                               gpointer   callback_data);
static void task_popup_unlink_task_cb         (GtkAction *action,
                                               gpointer   callback_data);


static GtkActionEntry popup_menu_entries[] = {
	{ "InsertTask", "planner-stock-insert-task",
	  N_("_Insert task"), NULL, NULL,
	  G_CALLBACK (task_popup_insert_task_cb)
	},
	{ "InsertSubtask", NULL,
	  N_("Insert _subtask"), NULL, NULL,
	  G_CALLBACK (task_popup_insert_subtask_cb)
	},
	{ "RemoveTask", "planner-stock-remove-task",
	  N_("_Remove task"), NULL, NULL,
	  G_CALLBACK (task_popup_remove_task_cb)
	},
	{ "UnlinkTask", "planner-stock-unlink-task",
	  N_("_Unlink task"), NULL, NULL,
	  G_CALLBACK (task_popup_unlink_task_cb)
	},
	{ "AssignResources", NULL,
	  N_("Assign _resources..."), NULL, NULL,
	  G_CALLBACK (task_popup_edit_task_resources_cb)
	},
	{ "EditTask", NULL,
	  N_("_Edit task..."), NULL, NULL,
	  G_CALLBACK (task_popup_edit_task_cb)
	},
};

static const char *popup_menu =
"<ui>"
	"<popup name='TaskPopup'>"
		"<menuitem action='InsertTask'/>"
		"<menuitem action='InsertSubtask'/>"
		"<menuitem action='RemoveTask'/>"
		"<separator/>"
		"<menuitem action='UnlinkTask'/>"
		"<separator/>"
		"<menuitem action='AssignResources'/>"
		"<menuitem action='EditTask'/>"
	"</popup>"
"</ui>";

static void
task_popup_insert_task_cb (GtkAction *action,
                           gpointer   callback_data)
{
	planner_task_tree_insert_task (callback_data);
}

static void
task_popup_insert_subtask_cb (GtkAction *action,
                              gpointer   callback_data)
{
	planner_task_tree_insert_subtask (callback_data);
}

static void
task_popup_remove_task_cb (GtkAction *action,
                           gpointer   callback_data)
{
	planner_task_tree_remove_task (callback_data);
}

static void
task_popup_edit_task_cb (GtkAction *action,
                         gpointer   callback_data)
{
	planner_task_tree_edit_task (callback_data,
				     PLANNER_TASK_DIALOG_PAGE_GENERAL);
}

static void
task_popup_edit_task_resources_cb (GtkAction *action,
                                   gpointer   callback_data)
{
	planner_task_tree_edit_task (callback_data,
				     PLANNER_TASK_DIALOG_PAGE_RESOURCES);
}

static void
task_popup_unlink_task_cb (GtkAction *action,
                           gpointer   callback_data)
{
	planner_task_tree_unlink_task (callback_data);
}

GtkUIManager *
planner_task_popup_new (PlannerTaskTree *tree)
{
	GtkActionGroup *action_group;
	GtkUIManager *ui_manager;
	GError *error;

	action_group = gtk_action_group_new ("TaskPopupActions");
	gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);
	gtk_action_group_add_actions (action_group,
				      popup_menu_entries,
				      G_N_ELEMENTS (popup_menu_entries),
				      tree);

	ui_manager = gtk_ui_manager_new ();
	gtk_ui_manager_insert_action_group (ui_manager, action_group, 0);

	error = NULL;
	if (!gtk_ui_manager_add_ui_from_string (ui_manager, popup_menu, -1, &error))
	{
		g_critical ("Building task popup menu failed: %s", error->message);
		g_error_free (error);
		g_abort ();
	}
	return ui_manager;
}

void
planner_task_popup_set_sensitive (GtkUIManager *ui_manager, const gchar *path, gboolean sensitive)
{
	GtkAction *action;

	action = gtk_ui_manager_get_action (ui_manager, path);
	gtk_action_set_sensitive (action, sensitive);
}

void
planner_task_popup_update_sensitivity (GtkUIManager *ui_manager,
				       GList        *tasks)
{
	gint          length;
	MrpTask      *task;
	MrpTaskType   type;
	gboolean      milestone;

	length = g_list_length (tasks);

	/* Can always insert task. */
	planner_task_popup_set_sensitive (ui_manager, "/TaskPopup/InsertTask", TRUE);

	/* Nothing else when nothing is selected. */
	if (length == 0) {
		planner_task_popup_set_sensitive (ui_manager, "/TaskPopup/InsertSubtask", FALSE);
		planner_task_popup_set_sensitive (ui_manager, "/TaskPopup/RemoveTask", FALSE);
		planner_task_popup_set_sensitive (ui_manager, "/TaskPopup/UnlinkTask", FALSE);
		planner_task_popup_set_sensitive (ui_manager, "/TaskPopup/AssignResources", FALSE);
		planner_task_popup_set_sensitive (ui_manager, "/TaskPopup/EditTask", FALSE);
		return;
	}

	/* Can only insert subtask when one !milestone is selected. */
	if (length == 1) {
		task = tasks->data;

		type = mrp_task_get_task_type (task);
		milestone = (type == MRP_TASK_TYPE_MILESTONE);
		planner_task_popup_set_sensitive (ui_manager, "/TaskPopup/InsertSubtask", !milestone);
	} else {
		planner_task_popup_set_sensitive (ui_manager, "/TaskPopup/InsertSubtask", FALSE);
	}

	/* The rest are always sensitive when one more more tasks are selected. */
	planner_task_popup_set_sensitive (ui_manager, "/TaskPopup/RemoveTask", TRUE);
	planner_task_popup_set_sensitive (ui_manager, "/TaskPopup/UnlinkTask", TRUE);
	planner_task_popup_set_sensitive (ui_manager, "/TaskPopup/AssignResources", TRUE);
	planner_task_popup_set_sensitive (ui_manager, "/TaskPopup/EditTask", TRUE);
}
