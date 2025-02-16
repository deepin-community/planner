/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2003-2005 Imendio AB
 * Copyright (C) 2001-2002 CodeFactory AB
 * Copyright (C) 2001-2002 Richard Hult <richard@imendio.com>
 * Copyright (C) 2001-2002 Mikael Hallendal <micke@imendio.com>
 * Copyright (C) 2001-2002 Alvaro del Castillo <acs@barrapunto.com>
 * Copyright (C) 2004      Lincoln Phipps <lincoln.phipps@openmutual.net>
 * Copyright (C) 2022      Mart Raudsepp <mart@leio.tech>
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
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <libplanner/mrp-project.h>
#include <libplanner/mrp-time.h>
#include "libplanner/mrp-paths.h"
#include "planner-calendar-popover.h"
#include "planner-calendar-selector.h"
#include "planner-format.h"
#include "planner-project-properties.h"
#include "planner-util.h"

typedef struct {
	PlannerWindow *main_window;
	MrpProject    *project;
	GtkWidget     *dialog;
	GtkWidget     *name_entry;
	GtkWidget     *org_entry;
	GtkWidget     *manager_entry;
	GtkWidget     *start_entry;
	GtkWidget     *start_popover;
	GtkWidget     *phase_combo_box_text;
	GtkWidget     *calendar_label;

	GtkTreeView   *properties_tree;
	GtkWidget     *add_property_button;
	GtkWidget     *remove_property_button;
} DialogData;

typedef struct {
	PlannerCmd       base;

	MrpProject      *project;
	gchar	 	*name;
	MrpPropertyType	 type;
	gchar	 	*label;
	gchar	 	*description;
	GType		 owner;
	gboolean	 user_defined;
} PropertyCmdAdd;

typedef struct {
	PlannerCmd       base;

	MrpProject      *project;
	gchar	 	*name;
	MrpPropertyType	 type;
	gchar	 	*label;
	gchar	 	*description;
	GType		 owner;
	gboolean	 user_defined;
	gchar           *old_text;
} PropertyCmdRemove;

typedef struct {
	PlannerCmd       base;

	MrpProject      *project;
	MrpProperty	*property;
	gchar           *old_text;
	gchar  	     	*new_text;
} PropertyCmdValueEdited;


#define DIALOG_GET_DATA(d) g_object_get_data ((GObject*)d, "data")

static void     mpp_select_calendar_clicked_cb        (GtkWidget           *button,
						       GtkWidget           *dialog);
static void     mpp_name_set_from_widget              (GtkWidget           *dialog);
static gboolean mpp_name_focus_out_event_cb           (GtkWidget           *widget,
						       GdkEvent            *event,
						       GtkWidget           *dialog);
static void     mpp_project_name_notify_cb            (MrpProject          *project,
						       GParamSpec          *pspec,
						       GtkWidget           *dialog);
static void     mpp_org_set_from_widget               (GtkWidget           *dialog);
static gboolean mpp_org_focus_out_event_cb            (GtkWidget           *widget,
						       GdkEvent            *event,
						       GtkWidget           *dialog);
static void     mpp_project_org_notify_cb             (MrpProject          *project,
						       GParamSpec          *pspec,
						       GtkWidget           *dialog);
static void     mpp_project_manager_notify_cb         (MrpProject          *project,
						       GParamSpec          *pspec,
						       GtkWidget           *dialog);
static void     mpp_manager_set_from_widget           (GtkWidget           *dialog);
static gboolean mpp_manager_focus_out_event_cb        (GtkWidget           *widget,
						       GdkEvent            *event,
						       GtkWidget           *dialog);
static void     mpp_project_start_notify_cb           (MrpProject          *project,
						       GParamSpec          *pspec,
						       GtkWidget           *dialog);
static void     mpp_start_set_from_widget             (GtkWidget           *dialog);
static gboolean mpp_start_focus_out_event_cb          (GtkWidget           *widget,
						       GdkEvent            *event,
						       GtkWidget           *dialog);
static void     mpp_project_calendar_notify_cb        (MrpProject          *project,
						       GParamSpec          *pspec,
						       GtkWidget           *dialog);
static void     mpp_project_calendar_notify_cb        (MrpProject          *project,
						       GParamSpec          *pspec,
						       GtkWidget           *dialog);
static void     mpp_phase_combo_box_text_changed_cb   (GtkComboBoxText     *combo_box_text,
						       GtkWidget           *dialog);
static void     mpp_project_phases_notify_cb          (MrpProject          *project,
						       GParamSpec          *pspec,
						       GtkWidget           *dialog);
static void     mpp_project_phase_notify_cb           (MrpProject          *project,
						       GParamSpec          *pspec,
						       GtkWidget           *dialog);
static void     mpp_setup_phases                      (DialogData          *data);
static void     mpp_set_phase                         (DialogData          *data,
						       const gchar         *phase);
static void     mpp_setup_properties_list             (GtkWidget           *dialog);
static void     mpp_property_name_data_func           (GtkTreeViewColumn   *tree_column,
						       GtkCellRenderer     *cell,
						       GtkTreeModel        *tree_model,
						       GtkTreeIter         *iter,
						       GtkWidget           *dialog);
static void     mpp_property_value_data_func          (GtkTreeViewColumn   *tree_column,
						       GtkCellRenderer     *cell,
						       GtkTreeModel        *tree_model,
						       GtkTreeIter         *iter,
						       GtkWidget           *dialog);
static void     mpp_property_added                    (MrpProject          *project,
						       GType                object_type,
						       MrpProperty         *property,
						       GtkWidget           *dialog);
static void     mpp_property_removed                  (MrpProject          *project,
						       MrpProperty         *property,
						       GtkWidget           *dialog);
static void     mpp_property_changed                  (MrpProject          *project,
						       GType               object_type,
						       MrpProperty         *property,
						       GtkWidget           *dialog);
static void     mpp_add_property_button_clicked_cb    (GtkButton           *button,
						       GtkWidget           *dialog);
static void     mpp_remove_property_button_clicked_cb (GtkButton           *button,
						       GtkWidget           *dialog);
static void     mpp_property_value_edited             (GtkCellRendererText *cell,
						       gchar               *path_string,
						       gchar               *new_text,
						       GtkWidget           *dialog);
const char     *mpp_project_property_get_value_string (MrpProject  	   *project,
						       MrpProperty 	   *property);
static gboolean	mpp_project_property_set_value_string (MrpProject  	   *project,
						       MrpProperty  	   *property,
						       gchar  		   *text);

static PlannerCmd *property_cmd_add               (PlannerWindow   *window,
						   MrpProject      *project,
						   GType            owner,
						   const gchar     *name,
						   MrpPropertyType  type,
						   const gchar     *label,
						   const gchar     *description,
						   gboolean         user_defined);
static gboolean    property_cmd_add_do            (PlannerCmd      *cmd_base);
static void        property_cmd_add_undo          (PlannerCmd      *cmd_base);
static void        property_cmd_add_free          (PlannerCmd      *cmd_base);
static PlannerCmd *property_cmd_remove            (PlannerWindow   *window,
						   MrpProject      *project,
						   GType            owner,
						   const gchar     *name);
static gboolean    property_cmd_remove_do         (PlannerCmd      *cmd_base);
static void        property_cmd_remove_undo       (PlannerCmd      *cmd_base);
static void        property_cmd_remove_free       (PlannerCmd      *cmd_base);
static PlannerCmd *property_cmd_value_edited      (PlannerWindow   *window,
						   MrpProject      *project,
						   MrpProperty     *property,
						   const gchar     *new_text);
static gboolean    property_cmd_value_edited_do   (PlannerCmd      *cmd_base);
static void        property_cmd_value_edited_undo (PlannerCmd      *cmd_base);
static void        property_cmd_value_edited_free (PlannerCmd      *cmd_base);


enum {
	COL_PROPERTY,
	NUM_OF_COLS
};


/*
 * Commands
 */

typedef enum {
	PROP_STRING,
	PROP_DATE,
	PROP_CALENDAR
} PropType;

typedef struct {
	PlannerCmd   base;

	MrpProject  *project;

	PropType     type;
	gchar       *property;

	/* String */
	gchar       *str;
	gchar       *str_old;

	/* Date */
	mrptime      t;
	mrptime      t_old;

	/* This won't work... the calendar might be removed and re-added... so
	 * the pointer will not be the same.
	 */
	MrpCalendar *calendar;
	MrpCalendar *calendar_old;
} PropertyCmdEdit;

static gboolean
property_cmd_edit_do (PlannerCmd *cmd_base)
{
	PropertyCmdEdit *cmd;

	cmd = (PropertyCmdEdit*) cmd_base;

	switch (cmd->type) {
	case PROP_STRING:
		g_object_set (cmd->project,
			      cmd->property, cmd->str,
			      NULL);
		break;

	case PROP_DATE:
		g_object_set (cmd->project,
			      cmd->property, cmd->t,
			      NULL);
		break;

	case PROP_CALENDAR:
		g_object_set (cmd->project,
			      cmd->property, cmd->calendar,
			      NULL);
		break;
	}

	return TRUE;
}

static void
property_cmd_edit_undo (PlannerCmd *cmd_base)
{
	PropertyCmdEdit *cmd;

	cmd = (PropertyCmdEdit*) cmd_base;

	switch (cmd->type) {
	case PROP_STRING:
		g_object_set (cmd->project, cmd->property,
			      cmd->str_old, NULL);
		break;

	case PROP_DATE:
		g_object_set (cmd->project, cmd->property,
			      cmd->t_old, NULL);
		break;

	case PROP_CALENDAR:
		g_object_set (cmd->project, cmd->property,
			      cmd->calendar_old, NULL);
		break;
	}
}

static PlannerCmd *
property_cmd_edit (DialogData  *data,
		   const gchar *label,
		   PropType     type,
		   const gchar *property,
		   const gchar *str_value,
		   time_t       time_value,
		   MrpCalendar *calendar_value)
{
	PlannerCmd      *cmd_base;
	PropertyCmdEdit *cmd;

	cmd_base = planner_cmd_new (PropertyCmdEdit,
				    label,
				    property_cmd_edit_do,
				    property_cmd_edit_undo,
				    NULL /* FIXME */);

	cmd = (PropertyCmdEdit *) cmd_base;

	switch (type) {
	case PROP_STRING:
		g_object_get (data->project, property, &cmd->str_old, NULL);
		if (str_value == NULL && cmd->str_old == NULL) {
			goto no_change;
		}

		if (str_value && cmd->str_old &&
		    strcmp (str_value, cmd->str_old) == 0) {
			goto no_change;
		}

		cmd->str = g_strdup (str_value);
		break;

	case PROP_DATE:
		g_object_get (data->project, property, &cmd->t_old, NULL);
		if (time_value == cmd->t_old) {
			goto no_change;
		}

		cmd->t = time_value;
		break;

	case PROP_CALENDAR:
		g_object_get (data->project, property, &cmd->calendar_old, NULL);
		if (calendar_value == cmd->calendar_old) {
			goto no_change;
		}

		cmd->calendar = calendar_value;
		break;
	}

	cmd->project = data->project;
	cmd->type = type;
	cmd->property = g_strdup (property);

	planner_cmd_manager_insert_and_do (planner_window_get_cmd_manager (data->main_window),
					   cmd_base);

	return cmd_base;

 no_change:
	g_free (cmd->str_old);

	return NULL;
}

static void
mpp_select_calendar_clicked_cb (GtkWidget *button,
				GtkWidget *dialog)
{
	DialogData  *data;
	GtkWidget   *selector;
	gint         response;
	MrpCalendar *calendar;

	data = DIALOG_GET_DATA (dialog);

	selector = planner_calendar_selector_new (
		data->main_window,
		_("Select a calendar to use for this project:"));

	response = gtk_dialog_run (GTK_DIALOG (selector));
	switch (response) {
	case GTK_RESPONSE_OK:
		calendar = planner_calendar_selector_get_calendar (selector);
		g_object_set (data->project, "calendar", calendar, NULL);
		break;

	case GTK_RESPONSE_DELETE_EVENT:
	case GTK_RESPONSE_CANCEL:
		break;

	default:
		break;
	}

	gtk_widget_destroy (selector);
}

static void
mpp_connect_to_project (MrpProject *project, GtkWidget *dialog)
{
	g_signal_connect_object (project, "notify::name",
				 G_CALLBACK (mpp_project_name_notify_cb),
				 dialog,
				 0);

	g_signal_connect_object (project, "notify::project-start",
				 G_CALLBACK (mpp_project_start_notify_cb),
				 dialog,
				 0);

	g_signal_connect_object (project, "notify::organization",
				 G_CALLBACK (mpp_project_org_notify_cb),
				 dialog,
				 0);

	g_signal_connect_object (project, "notify::manager",
				 G_CALLBACK (mpp_project_manager_notify_cb),
				 dialog,
				 0);

	g_signal_connect_object (project, "notify::calendar",
				 G_CALLBACK (mpp_project_calendar_notify_cb),
				 dialog,
				 0);

	g_signal_connect_object (project, "notify::phases",
				 G_CALLBACK (mpp_project_phases_notify_cb),
				 dialog,
				 0);

	g_signal_connect_object (project,
				 "notify::phase",
				 G_CALLBACK (mpp_project_phase_notify_cb),
				 dialog,
				 0);

	g_signal_connect_object (project,
				 "property_added",
				 G_CALLBACK (mpp_property_added),
				 dialog,
				 0);
	g_signal_connect_object (project,
				 "property_removed",
				 G_CALLBACK (mpp_property_removed),
				 dialog,
				 0);
	g_signal_connect_object (project,
				 "property_changed",
				 G_CALLBACK (mpp_property_changed),
				 dialog,
				 0);
}

static void
mpp_project_name_notify_cb  (MrpProject *project,
			     GParamSpec *pspec,
			     GtkWidget  *dialog)
{
	DialogData *data;
	gchar      *name;

	data = DIALOG_GET_DATA (dialog);

	g_object_get (project, "name", &name, NULL);
	gtk_entry_set_text (GTK_ENTRY (data->name_entry), name);
	g_free (name);
}

static void
mpp_name_set_from_widget (GtkWidget *dialog)
{
	DialogData  *data;
	const gchar *str;

	data = DIALOG_GET_DATA (dialog);
	str = gtk_entry_get_text (GTK_ENTRY (data->name_entry));

	property_cmd_edit (data, _("Edit Project Name"), PROP_STRING, "name", str, 0, NULL);
}

static gboolean
mpp_name_focus_out_event_cb (GtkWidget *widget,
			     GdkEvent  *event,
			     GtkWidget *dialog)
{
	mpp_name_set_from_widget (dialog);

	return FALSE;
}

static void
mpp_project_org_notify_cb (MrpProject *project,
			   GParamSpec *pspec,
			   GtkWidget *dialog)
{
	DialogData *data;
	gchar      *org;

	g_object_get (project, "organization", &org, NULL);
	data = DIALOG_GET_DATA (dialog);
	gtk_entry_set_text (GTK_ENTRY (data->org_entry), org);
	g_free (org);
}

static void
mpp_org_set_from_widget (GtkWidget *dialog)
{
	DialogData  *data;
	const gchar *str;

	data = DIALOG_GET_DATA (dialog);
	str = gtk_entry_get_text (GTK_ENTRY (data->org_entry));

	property_cmd_edit (data, _("Edit Organization"), PROP_STRING, "organization", str, 0, NULL);
}

static gboolean
mpp_org_focus_out_event_cb (GtkWidget *widget,
			    GdkEvent  *event,
			    GtkWidget *dialog)
{
	mpp_org_set_from_widget (dialog);

	return FALSE;
}

static void
mpp_project_manager_notify_cb (MrpProject *project,
			       GParamSpec *pspec,
			       GtkWidget  *dialog)
{
	DialogData *data;
	gchar      *manager;

	g_object_get (project, "manager", &manager, NULL);
	data = DIALOG_GET_DATA (dialog);
	gtk_entry_set_text (GTK_ENTRY (data->manager_entry), manager);
	g_free (manager);
}

static void
mpp_manager_set_from_widget (GtkWidget *dialog)
{
	DialogData  *data;
	const gchar *str;

	data = DIALOG_GET_DATA (dialog);
	str = gtk_entry_get_text (GTK_ENTRY (data->manager_entry));

	property_cmd_edit (data, _("Edit Manager"), PROP_STRING, "manager", str, 0, NULL);
}

static gboolean
mpp_manager_focus_out_event_cb (GtkWidget *widget,
				GdkEvent  *event,
				GtkWidget *dialog)
{
	mpp_manager_set_from_widget (dialog);

	return FALSE;
}

static void
mpp_set_start (GtkWidget *dialog, mrptime start)
{
	DialogData  *data;
	gchar       *str;

	data = DIALOG_GET_DATA (dialog);

	str = mrp_time_format_locale (start);

	gtk_entry_set_text (GTK_ENTRY (data->start_entry), str);
	g_free (str);
}

static void
mpp_project_start_notify_cb (MrpProject *project,
			     GParamSpec *pspec,
			     GtkWidget  *dialog)
{
	DialogData *data;
	mrptime     start;

	data = DIALOG_GET_DATA (dialog);

	start = mrp_project_get_project_start (data->project);
	mpp_set_start (dialog, start);
}

static void
mpp_start_set_from_widget (GtkWidget *dialog)
{
	DialogData  *data;
	const gchar *str;
	mrptime      start;
	GDate       *date;
	gint         year, month, day;

	data = DIALOG_GET_DATA (dialog);

	str = gtk_entry_get_text (GTK_ENTRY (data->start_entry));

	date = g_date_new ();
	g_date_set_parse (date, str);

	if (!g_date_valid (date)) {
		return;
	}

	year = g_date_get_year (date);
	month = g_date_get_month (date);
	day = g_date_get_day (date);

	g_date_free (date);

	start = mrp_time_compose (year, month, day, 0, 0, 0);
	if (start < 0) {
		return;
	}

	g_object_set (data->start_popover,
		      "mrptime", start,
		      NULL);
	property_cmd_edit (data, _("Edit Project Start"), PROP_DATE, "project-start", NULL, start, NULL);
}

static gboolean
mpp_start_focus_out_event_cb (GtkWidget *widget,
			      GdkEvent  *event,
			      GtkWidget *dialog)
{
	mpp_start_set_from_widget (dialog);

	return FALSE;
}

static void
mpp_start_popover_closed_cb (GtkWidget              *dialog,
                             PlannerCalendarPopover *popover)
{
	DialogData *data;
	mrptime     start;

	data = DIALOG_GET_DATA (dialog);
	g_object_get (popover,
		      "mrptime", &start,
		      NULL);
	property_cmd_edit (data, _("Edit Project Start"), PROP_DATE, "project-start", NULL, start, NULL);
	mpp_set_start (dialog, start);
}

static void
mpp_project_calendar_notify_cb (MrpProject *project,
				GParamSpec *pspec,
				GtkWidget  *dialog)
{
	DialogData  *data;
	MrpCalendar *calendar;

	data = DIALOG_GET_DATA (dialog);

	calendar = mrp_project_get_calendar (project);

	gtk_label_set_text (GTK_LABEL (data->calendar_label),
			    mrp_calendar_get_name (calendar));
}

static void
mpp_phase_combo_box_text_changed_cb (GtkComboBoxText *combo_box_text,
				     GtkWidget       *dialog)
{
	property_cmd_edit (DIALOG_GET_DATA (dialog),
			   _("Edit Project Phase"), PROP_STRING, "phase",
			   gtk_combo_box_get_active (GTK_COMBO_BOX (combo_box_text))
			   ? gtk_combo_box_text_get_active_text (combo_box_text)
			   : NULL, /* First item is always "None", meaning no phase is selected */
			   0, NULL);
}

static void
mpp_project_phases_notify_cb (MrpProject  *project,
			      GParamSpec  *pspec,
			      GtkWidget   *dialog)
{
	DialogData *data = DIALOG_GET_DATA (dialog);

	g_signal_handlers_block_by_func (data->phase_combo_box_text,
					 mpp_phase_combo_box_text_changed_cb,
					 dialog);

	mpp_setup_phases (data);

	g_signal_handlers_unblock_by_func (data->phase_combo_box_text,
					   mpp_phase_combo_box_text_changed_cb,
					   dialog);
}

static void
mpp_project_phase_notify_cb (MrpProject  *project,
			     GParamSpec  *pspec,
			     GtkWidget   *dialog)
{
	DialogData *data;
	gchar      *phase;

	data = DIALOG_GET_DATA (dialog);

	g_object_get (data->project, "phase", &phase, NULL);
	mpp_set_phase (data, phase);
	g_free (phase);
}

static void
mpp_setup_phases (DialogData *data)
{
	GtkComboBoxText *combo_text;
	GList           *phases, *l;
	gchar           *phase;

	phases = NULL;

	g_object_get (data->project,
		      "phases", &phases,
		      "phase", &phase,
		      NULL);

	combo_text = GTK_COMBO_BOX_TEXT (data->phase_combo_box_text);
	/* TODO-GTK3: Use gtk_combo_box_text_remove_all */
	gtk_list_store_clear (GTK_LIST_STORE (gtk_combo_box_get_model (GTK_COMBO_BOX (combo_text))));

	gtk_combo_box_text_append_text (combo_text, _("None"));

	for (l = phases; l; l = l->next) {
		gtk_combo_box_text_append_text (combo_text, (l->data));
	}

	mpp_set_phase (data, phase);

	g_free (phase);
	mrp_string_list_free (phases);
}

static void
mpp_set_phase (DialogData  *data,
	       const gchar *phase)
{
	if (phase) {
		GtkComboBox  *combo;
		GtkTreeModel *model;
		GtkTreeIter   iter;
		gchar        *name;

		combo = GTK_COMBO_BOX (data->phase_combo_box_text);
		model = gtk_combo_box_get_model (combo);
		gtk_tree_model_get_iter_first (model, &iter);

		while (gtk_tree_model_iter_next (model, &iter)) { /* Skips the first "None" item */
			gtk_tree_model_get (model, &iter, 0, &name, -1);
			if (!strcmp (phase, name)) {
				gtk_combo_box_set_active_iter (combo, &iter);
				return;
			}
		}
	}

	/* If we didn't match, set "None". */
	gtk_combo_box_set_active (GTK_COMBO_BOX (data->phase_combo_box_text), 0);
}

static void
mpp_setup_properties_list (GtkWidget *dialog)
{
	DialogData *data = DIALOG_GET_DATA (dialog);
	GList             *properties, *l;
	GtkTreeModel      *model;
	GtkTreeViewColumn *col;
	GtkCellRenderer   *cell;

	model = GTK_TREE_MODEL (
		gtk_list_store_new (NUM_OF_COLS, G_TYPE_POINTER));
	gtk_tree_view_set_model (data->properties_tree, model);
	g_object_unref (model);

	properties = mrp_project_get_properties_from_type (data->project,
							   MRP_TYPE_PROJECT);

	for (l = properties; l; l = l->next) {
		MrpProperty *property = MRP_PROPERTY (l->data);
		GtkTreeIter  iter;

		if (!mrp_property_get_user_defined (property)) {
			continue;
		}

		gtk_list_store_append (GTK_LIST_STORE (model),
				       &iter);
		gtk_list_store_set (GTK_LIST_STORE (model),
				    &iter,
				    COL_PROPERTY, mrp_property_ref (property),
				    -1);
	}

	cell = gtk_cell_renderer_text_new ();
	g_object_set (cell, "editable", FALSE, NULL);

	col = gtk_tree_view_column_new_with_attributes (_("Property"),
							cell, NULL);
	gtk_tree_view_column_set_cell_data_func (col, cell,
						 (GtkTreeCellDataFunc) mpp_property_name_data_func,
						 dialog, NULL);

	gtk_tree_view_append_column (data->properties_tree, col);

	cell = gtk_cell_renderer_text_new ();
	g_object_set (cell, "editable", TRUE, NULL);

	col = gtk_tree_view_column_new_with_attributes (_("Value"),
							cell, NULL);

	gtk_tree_view_column_set_cell_data_func (col, cell,
						 (GtkTreeCellDataFunc) mpp_property_value_data_func,
						 dialog, NULL);

	g_signal_connect (cell,
			  "edited",
			  G_CALLBACK (mpp_property_value_edited),
			  dialog);

	gtk_tree_view_append_column (data->properties_tree, col);
}

static void
mpp_property_name_data_func (GtkTreeViewColumn *tree_column,
			     GtkCellRenderer   *cell,
			     GtkTreeModel      *tree_model,
			     GtkTreeIter       *iter,
			     GtkWidget         *dialog)
{
	MrpProperty *property;

	gtk_tree_model_get (tree_model, iter, COL_PROPERTY, &property, -1);

	g_object_set (cell,
		      "text", mrp_property_get_name (property),
		      NULL);
}

static void
mpp_property_value_data_func (GtkTreeViewColumn *tree_column,
			      GtkCellRenderer   *cell,
			      GtkTreeModel      *tree_model,
			      GtkTreeIter       *iter,
			      GtkWidget         *dialog)
{
	DialogData      *data = DIALOG_GET_DATA (dialog);
	MrpProperty     *property;
	gchar           *svalue;
	MrpPropertyType  type;
	gint             ivalue;
	gfloat           fvalue;
/* 	mrptime          tvalue; */

	gtk_tree_model_get (tree_model, iter, COL_PROPERTY, &property, -1);

	type = mrp_property_get_property_type (property);

	switch (type) {
	case MRP_PROPERTY_TYPE_STRING:
		mrp_object_get (data->project,
				mrp_property_get_name (property), &svalue,
				NULL);

		if (svalue == NULL) {
			svalue = g_strdup ("");
		}

		break;
	case MRP_PROPERTY_TYPE_INT:
		mrp_object_get (data->project,
				mrp_property_get_name (property), &ivalue,
				NULL);
		svalue = g_strdup_printf ("%d", ivalue);
		break;

	case MRP_PROPERTY_TYPE_FLOAT:
		mrp_object_get (data->project,
				mrp_property_get_name (property), &fvalue,
				NULL);

		svalue = planner_format_float (fvalue, 4, FALSE);
		break;

	case MRP_PROPERTY_TYPE_DATE:
		svalue = g_strdup ("");


/* 		mrp_object_get (data->project, */
/* 				mrp_property_get_name (property), &tvalue, */
/* 				NULL);  */
/* 		svalue = planner_format_date (tvalue); */
		break;

	case MRP_PROPERTY_TYPE_DURATION:
		mrp_object_get (data->project,
				mrp_property_get_name (property), &ivalue,
				NULL);
		svalue = planner_format_duration (data->project, ivalue);
		break;

	case MRP_PROPERTY_TYPE_COST:
		mrp_object_get (data->project,
				mrp_property_get_name (property), &fvalue,
				NULL);

		svalue = planner_format_float (fvalue, 2, FALSE);
		break;

	default:
		g_warning ("Type not implemented.");
		svalue = g_strdup ("");
		break;
	}

	g_object_set (cell, "text", svalue, NULL);
	g_free (svalue);
}

static void
mpp_property_added (MrpProject  *project,
		    GType        object_type,
		    MrpProperty *property,
		    GtkWidget   *dialog)
{
	DialogData   *data = DIALOG_GET_DATA (dialog);
	GtkTreeModel *model;
	GtkTreeIter   iter;

	model = gtk_tree_view_get_model (data->properties_tree);

	if (object_type != MRP_TYPE_PROJECT ||
	    !mrp_property_get_user_defined (property)) {
 		return;
	}
/*
  if (gtk_tree_view_get_model (data->properties_tree) != model) {
  return;
  }
*/
	gtk_list_store_append (GTK_LIST_STORE (model), &iter);
	gtk_list_store_set (GTK_LIST_STORE (model),
			    &iter,
			    COL_PROPERTY, mrp_property_ref (property),
			    -1);
}

typedef struct {
	MrpProperty *property;
	GtkTreePath *found_path;
	GtkTreeIter *found_iter;
} PropertyFindData;

static gboolean
mpp_property_find (GtkTreeModel     *model,
		   GtkTreePath      *path,
		   GtkTreeIter      *iter,
		   PropertyFindData *data)
{
	MrpProperty *property;

	gtk_tree_model_get (model, iter,
			    COL_PROPERTY, &property,
			    -1);

	if (strcmp (mrp_property_get_name (data->property),
		    mrp_property_get_name (property)) == 0) {
		data->found_path = gtk_tree_path_copy (path);
		data->found_iter = gtk_tree_iter_copy (iter);
		return TRUE;
	}

	return FALSE;
}

static void
mpp_property_removed (MrpProject  *project,
		      MrpProperty *property,
		      GtkWidget   *dialog)
{
	DialogData       *data = DIALOG_GET_DATA (dialog);
	GtkTreeModel     *model;
	PropertyFindData *find_data;

	model = gtk_tree_view_get_model (data->properties_tree);

	find_data = g_new0 (PropertyFindData, 1);
	find_data->property = mrp_property_ref (property);
	find_data->found_path = NULL;
	find_data->found_iter = NULL;

	gtk_tree_model_foreach (model,
				(GtkTreeModelForeachFunc) mpp_property_find,
				find_data);

	mrp_property_unref (property);

	if (find_data->found_path) {
		gtk_list_store_remove (GTK_LIST_STORE (model),
				       find_data->found_iter);
		mrp_property_unref (property);

		gtk_tree_iter_free (find_data->found_iter);
		gtk_tree_path_free (find_data->found_path);
	}

	g_free (find_data);
}

static void
mpp_property_changed (MrpProject   *project,
		      GType          object_type,
		      MrpProperty    *property,
		      GtkWidget      *dialog)
{
	DialogData       *data;
	GtkTreeModel     *model;
	PropertyFindData *find_data;

	/* Check for NULL here and shift the check for GTK_IS_DIALOG further in.
	 * The problem is with property_changed due to the way that there are
	 *  3 types of properties so if you have different dialogs opened the
	 *  wrong data can upset the open dialogs.
	 */

	if (object_type == MRP_TYPE_PROJECT) {
		data = DIALOG_GET_DATA (dialog);
		model = gtk_tree_view_get_model (data->properties_tree);

		find_data = g_new0 (PropertyFindData, 1);
		find_data->property = property;
		find_data->found_path = NULL;
		find_data->found_iter = NULL;

		gtk_tree_model_foreach (model,
					(GtkTreeModelForeachFunc) mpp_property_find,
					find_data);

		if (find_data->found_path) {
			gtk_tree_model_row_changed (model,
						    find_data->found_path,
						    find_data->found_iter);
		}
		if (find_data) {
			g_free (find_data);
		}
	}
}

static gboolean
mpp_property_dialog_label_changed_cb (GtkWidget *label_entry,
				      GdkEvent  *event,
				      GtkWidget *name_entry)
{
	const gchar *name;
	const gchar *label;

	name = gtk_entry_get_text (GTK_ENTRY (name_entry));

	if (name == NULL || name[0] == 0) {
		label = gtk_entry_get_text (GTK_ENTRY (label_entry));
		gtk_entry_set_text (GTK_ENTRY (name_entry), label);
	}

	return FALSE;
}

static void
type_combo_get_name (GtkCellLayout   *layout,
                     GtkCellRenderer *cell,
                     GtkTreeModel    *model,
                     GtkTreeIter     *iter,
                     gpointer         user_data)
{
	gchar *name;
	gtk_tree_model_get (model, iter, 0, &name, -1);
	g_object_set (cell, "text", name, NULL);
}

static void
mpp_property_dialog_setup_combobox (GtkWidget     *combo,
                                    gconstpointer  str1,
                                    ...)
{
	GtkListStore    *store;
	GtkTreeIter      iter;
	gint             i;
	va_list          args;
	gconstpointer    str;
	gint             type;
	GtkCellRenderer *renderer;

	store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_INT);
	gtk_combo_box_set_model (GTK_COMBO_BOX (combo), GTK_TREE_MODEL (store));
	g_object_unref (store);

	va_start (args, str1);
	for (str = str1, i = 0; str != NULL; str = va_arg (args, gpointer), i++) {
		type = va_arg (args, gint);

		gtk_list_store_append (store, &iter);
		gtk_list_store_set (store, &iter,
				    0, str,
				    1, type,
				    -1);
	}
	va_end (args);

	gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 0);

	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo),
				    renderer, TRUE);
	gtk_cell_layout_set_cell_data_func (GTK_CELL_LAYOUT (combo),
					    renderer,
					    type_combo_get_name,
					    NULL, NULL);
}

static gint
mpp_property_dialog_get_selected (GtkWidget *combo)
{
	GtkTreeIter iter;
	gint        ret;

	gtk_combo_box_get_active_iter (GTK_COMBO_BOX (combo), &iter);
	gtk_tree_model_get (gtk_combo_box_get_model (GTK_COMBO_BOX (combo)),
			    &iter,
			    1, &ret,
			    -1);

	return ret;
}

static void
mpp_add_property_button_clicked_cb (GtkButton *button, GtkWidget *dialog)
{
	DialogData      *data = DIALOG_GET_DATA (dialog);
	MrpPropertyType  type;
	const gchar     *label;
	const gchar     *name;
	const gchar     *description;
	GtkBuilder      *builder;
	GtkWidget       *label_entry;
	GtkWidget       *name_entry;
	GtkWidget       *add_dialog;
	GtkWidget       *w;
	gint             response;
	gboolean         finished = FALSE;

	builder = gtk_builder_new_from_resource ("/app/drey/Planner/ui/new-property.ui");

	add_dialog = GTK_WIDGET (gtk_builder_get_object (builder, "add_dialog"));

	label_entry = GTK_WIDGET (gtk_builder_get_object (builder, "label_entry"));
	name_entry = GTK_WIDGET (gtk_builder_get_object (builder, "name_entry"));

	g_signal_connect (label_entry,
			  "focus_out_event",
			  G_CALLBACK (mpp_property_dialog_label_changed_cb),
			  name_entry);

	mpp_property_dialog_setup_combobox (
		GTK_WIDGET (gtk_builder_get_object (builder, "type_menu")),
		mrp_property_type_as_string (MRP_PROPERTY_TYPE_STRING),
		MRP_PROPERTY_TYPE_STRING,
		mrp_property_type_as_string (MRP_PROPERTY_TYPE_INT),
		MRP_PROPERTY_TYPE_INT,
		mrp_property_type_as_string (MRP_PROPERTY_TYPE_FLOAT),
		MRP_PROPERTY_TYPE_FLOAT,
/*  		mrp_property_type_as_string (MRP_PROPERTY_TYPE_DATE), */
/*  		MRP_PROPERTY_TYPE_DATE, */
/*  		mrp_property_type_as_string (MRP_PROPERTY_TYPE_DURATION), */
/*  		MRP_PROPERTY_TYPE_DURATION, */
/* 		mrp_property_type_as_string (MRP_PROPERTY_TYPE_COST), */
/*  		MRP_PROPERTY_TYPE_COST, */
		NULL);

	while (!finished) {
		response = gtk_dialog_run (GTK_DIALOG (add_dialog));

		switch (response) {
		case GTK_RESPONSE_OK:
			label = gtk_entry_get_text (GTK_ENTRY (label_entry));
			if (label == NULL || label[0] == 0) {
				finished = FALSE;
				break;
			}

			name = gtk_entry_get_text (GTK_ENTRY (name_entry));
			if (name == NULL || name[0] == 0) {
				finished = FALSE;
				break;
			}

			/* FIXME: This is broken wrt UTF-8. */
			if (!isalpha(name[0])) {
				GtkWidget *dialog;

				dialog = gtk_message_dialog_new (GTK_WINDOW (add_dialog),
								 GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
								 GTK_MESSAGE_WARNING,
								 GTK_BUTTONS_OK,
								 _("The name of the custom property needs to "
								   "start with a letter."));
				gtk_dialog_run (GTK_DIALOG (dialog));
				gtk_widget_destroy (dialog);

				finished = FALSE;
				break;
			}

			w = GTK_WIDGET (gtk_builder_get_object (builder, "description_entry"));
			description = gtk_entry_get_text (GTK_ENTRY (w));

			w = GTK_WIDGET (gtk_builder_get_object (builder, "type_menu"));

			type = mpp_property_dialog_get_selected (w);

			if (type != MRP_PROPERTY_TYPE_NONE) {
				property_cmd_add (data->main_window,
						  data->project,
						  MRP_TYPE_PROJECT,
						  name,
						  type,
						  label,
						  description,
						  TRUE);
			}

			finished = TRUE;
			break;

		case GTK_RESPONSE_DELETE_EVENT:
		case GTK_RESPONSE_CANCEL:
			finished = TRUE;
			break;

		default:
			break;
		}
	}

	gtk_widget_destroy (add_dialog);
	g_object_unref (builder);
}

static void
mpp_remove_property_button_clicked_cb (GtkButton *button, GtkWidget *dialog)
{
	DialogData       *data = DIALOG_GET_DATA (dialog);
	GtkTreeModel     *model;
	GtkTreeSelection *selection;
	GtkTreeIter       iter;
	MrpProperty      *property;
	GtkWidget        *remove_dialog;
	gint              response;

	model = gtk_tree_view_get_model (data->properties_tree);

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (data->properties_tree));

	if (!gtk_tree_selection_get_selected (selection, NULL, &iter)) {
		return;
	}

	gtk_tree_model_get (model, &iter,
			    COL_PROPERTY, &property,
			    -1);

	remove_dialog = gtk_message_dialog_new (GTK_WINDOW (dialog),
						GTK_DIALOG_MODAL |
						GTK_DIALOG_DESTROY_WITH_PARENT,
						GTK_MESSAGE_QUESTION,
						GTK_BUTTONS_YES_NO,
						_("Do you really want to remove the property '%s' from "
						  "the project?"),
						mrp_property_get_name (property));

	response = gtk_dialog_run (GTK_DIALOG (remove_dialog));

	switch (response) {
	case GTK_RESPONSE_YES:
		property_cmd_remove (data->main_window,
				     data->project,
				     MRP_TYPE_PROJECT,
				     mrp_property_get_name (property));
		break;

	case GTK_RESPONSE_DELETE_EVENT:
	case GTK_RESPONSE_CANCEL:
		break;

	default:
		break;
	}

	gtk_widget_destroy (remove_dialog);
}

static void
mpp_property_value_edited (GtkCellRendererText *cell,
			   gchar               *path_string,
			   gchar               *new_text,
			   GtkWidget           *dialog)
{
	DialogData      *data = DIALOG_GET_DATA (dialog);
	GtkTreePath     *path;
	GtkTreeIter      iter;
	GtkTreeModel    *model;
	MrpProperty     *property;
	MrpProject      *project;

	model = gtk_tree_view_get_model (data->properties_tree);
	project = data->project;

	path = gtk_tree_path_new_from_string (path_string);
	gtk_tree_model_get_iter (model, &iter, path);

	gtk_tree_model_get (model, &iter,
			    COL_PROPERTY, &property,
			    -1);

	property_cmd_value_edited (data->main_window,
				   project,
				   property,
				   new_text);

	gtk_tree_path_free (path);
}

static void
mpp_parent_destroy_cb (GtkWidget *parent,
		       GtkWidget *dialog)
{
	gtk_widget_destroy (dialog);
}

static void
mpp_dialog_destroy_cb (GtkWidget *dialog,
		       gpointer   user_data)
{
	mpp_name_set_from_widget (dialog);
	mpp_org_set_from_widget (dialog);
	mpp_manager_set_from_widget (dialog);
	mpp_start_set_from_widget (dialog);
}

GtkWidget *
planner_project_properties_new (PlannerWindow *window)
{
	GtkBuilder  *builder;
	GtkWidget   *dialog;
	GtkWidget   *w;
	DialogData  *data;
	mrptime      start;
	gchar       *name, *org, *manager;
	MrpCalendar *calendar;

	g_return_val_if_fail (PLANNER_IS_WINDOW (window), NULL);

	builder = gtk_builder_new_from_resource ("/app/drey/Planner/ui/project-properties.ui");

	dialog = GTK_WIDGET (gtk_builder_get_object (builder, "project_properties"));

	data = g_new0 (DialogData, 1);

	data->main_window = window;
	data->project = planner_window_get_project (window);
	data->dialog = dialog;

	g_signal_connect_object (GTK_WIDGET (gtk_builder_get_object (builder, "close_button")),
				 "clicked",
				 G_CALLBACK (mpp_parent_destroy_cb),
				 dialog,
				 0);

	g_signal_connect_object (window,
				 "destroy",
				 G_CALLBACK (mpp_parent_destroy_cb),
				 dialog,
				 0);

	g_signal_connect (dialog,
			  "destroy",
			  G_CALLBACK (mpp_dialog_destroy_cb),
			  NULL);

	data->name_entry = GTK_WIDGET (gtk_builder_get_object (builder, "entry_name"));
	data->org_entry = GTK_WIDGET (gtk_builder_get_object (builder, "entry_org"));
	data->manager_entry = GTK_WIDGET (gtk_builder_get_object (builder, "entry_manager"));
	data->start_entry = GTK_WIDGET (gtk_builder_get_object (builder, "entry_start"));
	data->start_popover = GTK_WIDGET (gtk_builder_get_object (builder, "calendar_popover"));
	data->phase_combo_box_text = GTK_WIDGET (gtk_builder_get_object (builder, "comboboxtext_phase"));
	data->calendar_label = GTK_WIDGET (gtk_builder_get_object (builder, "label_calendar"));
	data->properties_tree = GTK_TREE_VIEW (gtk_builder_get_object (builder, "properties_tree"));

	data->add_property_button = GTK_WIDGET (gtk_builder_get_object (builder,
									"add_property_button"));
	data->remove_property_button = GTK_WIDGET (gtk_builder_get_object (builder,
									   "remove_property_button"));

	g_signal_connect_object (data->start_popover,
				 "closed",
				 G_CALLBACK (mpp_start_popover_closed_cb),
				 dialog,
				 G_CONNECT_SWAPPED);

	g_object_set_data_full (G_OBJECT (dialog), "data", data, g_free);
	g_object_set_data (G_OBJECT (dialog), "project", data->project);

	g_object_get (data->project,
		      "name", &name,
		      "organization", &org,
		      "manager", &manager,
		      NULL);

	start = mrp_project_get_project_start (data->project);
	mpp_set_start (dialog, start);
	g_object_set (data->start_popover,
		      "mrptime", start,
		      NULL);
	g_signal_connect (data->start_entry,
			  "focus_out_event",
			  G_CALLBACK (mpp_start_focus_out_event_cb),
			  dialog);

	if (name) {
		gtk_entry_set_text (GTK_ENTRY (data->name_entry), name);
	}
	g_signal_connect (data->name_entry,
			  "focus_out_event",
			  G_CALLBACK (mpp_name_focus_out_event_cb),
			  dialog);

	if (org) {
		gtk_entry_set_text (GTK_ENTRY (data->org_entry), org);
	}
	g_signal_connect (data->org_entry,
			  "focus_out_event",
			  G_CALLBACK (mpp_org_focus_out_event_cb),
			  dialog);

	if (manager) {
		gtk_entry_set_text (GTK_ENTRY (data->manager_entry), manager);
	}
	g_signal_connect (data->manager_entry,
			  "focus_out_event",
			  G_CALLBACK (mpp_manager_focus_out_event_cb),
			  dialog);

	/* Project phase. */
	mpp_setup_phases (data);

	g_signal_connect (data->phase_combo_box_text,
			  "changed",
			  G_CALLBACK (mpp_phase_combo_box_text_changed_cb),
			  dialog);
	g_signal_connect (data->add_property_button,
			  "clicked",
			  G_CALLBACK (mpp_add_property_button_clicked_cb),
			  dialog);
	g_signal_connect (data->remove_property_button,
			  "clicked",
			  G_CALLBACK (mpp_remove_property_button_clicked_cb),
			  dialog);

	mpp_setup_properties_list (dialog);

	/* Project calendar. */
	calendar = mrp_project_get_calendar (data->project);

	gtk_label_set_text (GTK_LABEL (data->calendar_label),
			    mrp_calendar_get_name (calendar));

	w = GTK_WIDGET (gtk_builder_get_object (builder, "button_calendar"));
	g_signal_connect (w,
			  "clicked",
			  G_CALLBACK (mpp_select_calendar_clicked_cb),
			  dialog);

	g_free (name);
	g_free (manager);
	g_free (org);

	mpp_connect_to_project (data->project, dialog);

	gtk_widget_show_all (dialog);

	g_object_unref (builder);

	return dialog;
}



/*
 * Gets the value of a MrpProperty and converts to a string.
 *
 * Return value: A const char *, if found, otherwise NULL.
 * NOTES: This can't be made part of mrp-project.c as it uses planner-format.h
 * and thats not in libplanner so a bit messy. Its obviously not able to be in
 * mrp-properties.c as thats common to all projects and not just one. So this'll
 * have to stay here for now.
 *
 */

const char *
mpp_project_property_get_value_string (MrpProject  *project,
				       MrpProperty *property)
{
	MrpPropertyType  type;
	gchar           *svalue;
	gint             ivalue;
	gfloat           fvalue;

	g_return_val_if_fail (MRP_IS_PROJECT (project), NULL);
	g_return_val_if_fail (property != NULL, NULL);

	type = mrp_property_get_property_type (property);

	switch (type) {
	case MRP_PROPERTY_TYPE_STRING:
		mrp_object_get (project,
				mrp_property_get_name (property), &svalue,
				NULL);

		if (svalue == NULL) {
			svalue = g_strdup ("");
		}

		break;
	case MRP_PROPERTY_TYPE_INT:
		mrp_object_get (project,
				mrp_property_get_name (property), &ivalue,
				NULL);
		svalue = g_strdup_printf ("%d", ivalue);
		break;

	case MRP_PROPERTY_TYPE_FLOAT:
		mrp_object_get (project,
				mrp_property_get_name (property), &fvalue,
				NULL);

		svalue = planner_format_float (fvalue, 4, FALSE);
		break;

	case MRP_PROPERTY_TYPE_DATE:
		svalue = g_strdup ("");


/* 		mrp_object_get (data->project, */
/* 				mrp_property_get_name (property), &tvalue, */
/* 				NULL);  */
/* 		svalue = planner_format_date (tvalue); */
		break;

	case MRP_PROPERTY_TYPE_DURATION:
		mrp_object_get (project,
				mrp_property_get_name (property), &ivalue,
				NULL);
		svalue = planner_format_duration (project, ivalue);
		break;

	case MRP_PROPERTY_TYPE_COST:
		mrp_object_get (project,
				mrp_property_get_name (property), &fvalue,
				NULL);

		svalue = planner_format_float (fvalue, 2, FALSE);
		break;

	default:
		g_warning ("Property type not implemented.");
		svalue = g_strdup ("");
		break;
	}
	return ((const char *) svalue);
}

static gboolean
mpp_project_property_set_value_string (MrpProject  *project,
			  	       MrpProperty *property,
				       gchar  	   *text)
{
	MrpPropertyType type;
	gfloat		fvalue;

	type = mrp_property_get_property_type (property);
	switch (type) {
	case MRP_PROPERTY_TYPE_STRING:
		mrp_object_set (MRP_OBJECT (project),
				mrp_property_get_name (property),
				text,
				NULL);
		break;
	case MRP_PROPERTY_TYPE_INT:
		mrp_object_set (MRP_OBJECT (project),
				mrp_property_get_name (property),
				atoi (text),
				NULL);
		break;
	case MRP_PROPERTY_TYPE_FLOAT:
		fvalue = g_strtod (text, NULL);
		mrp_object_set (MRP_OBJECT (project),
				mrp_property_get_name (property),
				fvalue,
				NULL);
		break;

	case MRP_PROPERTY_TYPE_DURATION:
		/* FIXME: support reading units etc... */
		mrp_object_set (MRP_OBJECT (project),
				mrp_property_get_name (property),
				atoi (text) *8*60*60,
				NULL);
		break;

	case MRP_PROPERTY_TYPE_DATE:
		/* 		date = PLANNER_CELL_RENDERER_DATE (cell); */
		/* 		mrp_object_set (MRP_OBJECT (project), */
		/* 				mrp_property_get_name (property),  */
		/* 				&(date->time), */
		/* 				NULL); */
		break;
	case MRP_PROPERTY_TYPE_COST:
		fvalue = g_strtod (text, NULL);
		mrp_object_set (MRP_OBJECT (project),
				mrp_property_get_name (property),
				fvalue,
				NULL);
		break;
	case MRP_PROPERTY_TYPE_STRING_LIST:
		/* FIXME: Should string-list still be around? */
		break;
	default:
		g_assert_not_reached ();
		return (FALSE);
		break;
	}
	return (TRUE);
}


/* Start of UNDO/REDO routines */

static gboolean
property_cmd_add_do (PlannerCmd *cmd_base)
{
	PropertyCmdAdd   *cmd;
	MrpProperty		*property;

	cmd = (PropertyCmdAdd*) cmd_base;

	if (cmd->name == NULL) {
		return FALSE;
	}

	property = mrp_property_new (cmd->name,
				     cmd->type,
				     cmd->label,
				     cmd->description,
				     cmd->user_defined);

	mrp_project_add_property (cmd->project,
				  cmd->owner,
				  property,
				  cmd->user_defined);

	/* This functions as a simple success check for the REDO/UNDO stuff. Check if its added. */
	if (!mrp_project_has_property (cmd->project, cmd->owner, cmd->name)) {
 		g_warning ("%s: object of type '%s' still has no property named '%s'",
			   G_STRLOC,
			   g_type_name (cmd->owner),
			   cmd->name);
		return FALSE;
	}

	return TRUE;

}

static void
property_cmd_add_undo (PlannerCmd *cmd_base)
{
	PropertyCmdAdd *cmd;

	cmd = (PropertyCmdAdd*) cmd_base;

	if (cmd->name != NULL) {
		mrp_project_remove_property (cmd->project,
				  	     cmd->owner,
				  	     cmd->name);
		/* This functions as a simple success check for the REDO/UNDO stuff. Check if its removed. */
		if (mrp_project_has_property (cmd->project, cmd->owner, cmd->name)) {
 			g_warning ("%s: object of type '%s' still has the property named '%s'",
				   G_STRLOC,
			   	   g_type_name (cmd->owner),
			   	   cmd->name);
		}
	}
}


static void
property_cmd_add_free (PlannerCmd *cmd_base)
{
	PropertyCmdAdd  *cmd;

	cmd = (PropertyCmdAdd*) cmd_base;

	g_free (cmd->name);
	g_free (cmd->label);
	g_free (cmd->description);
	cmd->project = NULL;
}

static
PlannerCmd *
property_cmd_add 	(PlannerWindow *window,
			 MrpProject	*project,
			 GType		owner,
			 const gchar 	*name,
			 MrpPropertyType type,
			 const gchar     *label,
			 const gchar     *description,
			 gboolean	user_defined)
{
	PlannerCmd      *cmd_base;
	PropertyCmdAdd  *cmd;


	cmd_base = planner_cmd_new (PropertyCmdAdd,
				    _("Add project property"),
 				    property_cmd_add_do,
				    property_cmd_add_undo,
				    property_cmd_add_free);

	cmd = (PropertyCmdAdd *) cmd_base;

	cmd->project = project;
	cmd->owner = owner;
	cmd->name = g_strdup (name);
	cmd->type = type;
	cmd->label = g_strdup (label);
	cmd->description = g_strdup (description);
	cmd->user_defined = user_defined;

	planner_cmd_manager_insert_and_do (planner_window_get_cmd_manager (window),
					   cmd_base);

	return cmd_base;
}


static gboolean
property_cmd_remove_do (PlannerCmd *cmd_base)
{
	PropertyCmdRemove *cmd;

	cmd = (PropertyCmdRemove*) cmd_base;

	if (!mrp_project_has_property (cmd->project, cmd->owner, cmd->name)) {
 		g_warning ("%s: object of type '%s' has no property named '%s' to remove",
			   G_STRLOC,
			   g_type_name (cmd->owner),
			   cmd->name);
		return FALSE;
	}

	mrp_project_remove_property (cmd->project,
				     cmd->owner,
				     cmd->name);

	return TRUE;
}


static void
property_cmd_remove_undo (PlannerCmd *cmd_base)
{
	PropertyCmdRemove *cmd;
	MrpProperty  *property;
	MrpProject   *project;
	gchar			*new_text;

	cmd = (PropertyCmdRemove*) cmd_base;

	if (cmd->name != NULL) {

		project = cmd->project;
		new_text = cmd->old_text;

		property = mrp_property_new (cmd->name,
					     cmd->type,
					     cmd->label,
					     cmd->description,
					     cmd->user_defined);

		mrp_project_add_property (project,
					  cmd->owner,
					  property,
					  cmd->user_defined);

		/* Now restore the previous text value We've kept it as new_text so it was easy to cut+paste code */

		mpp_project_property_set_value_string (project, property, new_text);

		/* This functions as a simple success check for the REDO/UNDO stuff. Check if its removed. */
		if (!mrp_project_has_property (project, cmd->owner, cmd->name)) {
 			g_warning ("%s: object of type '%s' property named '%s' not restored.",
				   G_STRLOC,
			   	   g_type_name (cmd->owner),
			   	   cmd->name);
		}
	}
}

static void
property_cmd_remove_free (PlannerCmd *cmd_base)
{
	PropertyCmdRemove *cmd;

	cmd = (PropertyCmdRemove*) cmd_base;

	g_free (cmd->name);
	g_free (cmd->label);
	g_free (cmd->description);
	g_free (cmd->old_text);
	cmd->project = NULL;
}

static PlannerCmd *
property_cmd_remove 	(PlannerWindow *window,
			 MrpProject	*project,
			 GType		owner,
			 const gchar 	*name)
{
	PlannerCmd      *cmd_base;
	PropertyCmdRemove  *cmd;
	MrpProperty     *property;

	cmd_base = planner_cmd_new (PropertyCmdRemove,
				    _("Remove project property"),
 				    property_cmd_remove_do,
				    property_cmd_remove_undo,
				    property_cmd_remove_free);

	cmd = (PropertyCmdRemove *) cmd_base;

	cmd->project = project;
	cmd->owner   = owner;
	cmd->name = g_strdup (name);

	property = mrp_project_get_property (project,
					     name,
					     owner);

	cmd->type = mrp_property_get_property_type (property);
	cmd->description = g_strdup (mrp_property_get_description (property));
	cmd->label = g_strdup ( mrp_property_get_label (property));
	cmd->user_defined = mrp_property_get_user_defined (property);

	cmd->old_text = (gchar *) mpp_project_property_get_value_string (project, property);

	planner_cmd_manager_insert_and_do (planner_window_get_cmd_manager (window),
					   cmd_base);

	return cmd_base;
}

/* Label Edited Routine */

static gboolean
property_cmd_value_edited_do (PlannerCmd *cmd_base)
{
	PropertyCmdValueEdited *cmd;
	MrpProject  		*project;
	MrpProperty		*property;
	gchar			*new_text;

	cmd = (PropertyCmdValueEdited*) cmd_base;

	project = cmd->project;
	property = cmd->property;
	new_text = cmd->new_text;

	if (!cmd->property) {
		return FALSE;
	}

	return (mpp_project_property_set_value_string (project, property, new_text));
}

static void
property_cmd_value_edited_undo (PlannerCmd *cmd_base)
{
	PropertyCmdValueEdited *cmd;
	MrpProject		*project;
	MrpProperty		*property;
	gchar			*new_text;

	cmd = (PropertyCmdValueEdited*) cmd_base;

	property = cmd->property;
	project = cmd->project;
	new_text = cmd->old_text;

	if (cmd->property != NULL) {
		mpp_project_property_set_value_string (project, property, new_text);
	}
}

static void
property_cmd_value_edited_free (PlannerCmd *cmd_base)
{
	PropertyCmdValueEdited *cmd;

	cmd = (PropertyCmdValueEdited*) cmd_base;

	g_free (cmd->old_text);
	g_free (cmd->new_text);
	cmd->property = NULL;
	cmd->project = NULL;
}

static PlannerCmd *
property_cmd_value_edited 	(PlannerWindow *window,
				 MrpProject	*project,
				 MrpProperty	*property,
				 const gchar 	*new_text)
{
	PlannerCmd      *cmd_base;
	PropertyCmdValueEdited  *cmd;

	cmd_base = planner_cmd_new (PropertyCmdValueEdited,
				    _("Edit project property value"),
 				    property_cmd_value_edited_do,
				    property_cmd_value_edited_undo,
				    property_cmd_value_edited_free);

	cmd = (PropertyCmdValueEdited *) cmd_base;

	cmd->property = property;
	cmd->project = project;
	cmd->new_text = g_strdup (new_text);

	cmd->old_text = (gchar *) mpp_project_property_get_value_string (project, property);

	planner_cmd_manager_insert_and_do (planner_window_get_cmd_manager (window),
					   cmd_base);

	return cmd_base;
}


