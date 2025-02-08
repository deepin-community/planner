/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2002 CodeFactory AB
 * Copyright (C) 2002 Richard Hult <richard@imendio.com>
 * Copyright (C) 2002 Mikael Hallendal <micke@imendio.com>
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
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <libplanner/mrp-project.h>
#include "libplanner/mrp-paths.h"
#include "planner-default-week-dialog.h"
#include "planner-util.h"

#define RESPONSE_ADD    1
#define RESPONSE_REMOVE 2
#define RESPONSE_CLOSE  GTK_RESPONSE_CLOSE
#define RESPONSE_APPLY  GTK_RESPONSE_APPLY

/* FIXME: Move to mrp-time and use locale. */
static struct {
	gint   day;
	gchar *name;
} days[] = {
	{ MRP_CALENDAR_DAY_MON, N_("Monday") },
	{ MRP_CALENDAR_DAY_TUE, N_("Tuesday") },
	{ MRP_CALENDAR_DAY_WED, N_("Wednesday") },
	{ MRP_CALENDAR_DAY_THU, N_("Thursday") },
	{ MRP_CALENDAR_DAY_FRI, N_("Friday") },
	{ MRP_CALENDAR_DAY_SAT, N_("Saturday") },
	{ MRP_CALENDAR_DAY_SUN, N_("Sunday") }
};

enum {
	COL_NAME,
	COL_ID,
	NUM_COLS
};

typedef struct {
	PlannerWindow *main_window;
	MrpProject    *project;

	MrpCalendar   *calendar;

	GtkWidget     *dialog;
	GtkWidget     *weekday_combobox;
	GtkWidget     *daytype_combobox;

	GtkWidget     *from_label[5];
	GtkWidget     *to_label[5];
	GtkWidget     *dash_label[5];
} DialogData;

typedef struct {
	PlannerCmd   base;

	MrpProject  *project;
	MrpCalendar *calendar;

	gint         weekday;

	/* If work/nonwork/use base, we use the day, otherwise the ID. */
	MrpDay      *day;
	gint         day_id;

	/* If work/nonwork/use base, we use the day, otherwise the ID. */
	MrpDay      *old_day;
	gint         old_day_id;
} DefaultWeekCmdEdit;


#define DIALOG_GET_DATA(d) g_object_get_data ((GObject*)d, "data")

static void        default_week_dialog_response_cb               (GtkWidget     *dialog,
								  gint           response,
								  DialogData    *data);
static void        default_week_dialog_update_labels             (DialogData    *data);
static void        default_week_dialog_weekday_selected_cb       (GtkComboBoxText *combotext,
								  DialogData    *data);
static void        default_week_dialog_day_selected_cb           (GtkComboBox   *combo,
								  DialogData    *data);
static void        default_week_dialog_setup_daytype_combobox    (GtkComboBox   *combo,
								  MrpProject    *project,
								  MrpCalendar   *calendar);
static void        default_week_dialog_setup_weekday_combobox    (GtkComboBoxText *combotext);
static gint        default_week_dialog_get_selected_weekday      (DialogData    *data);
static MrpDay *    default_week_dialog_get_selected_day          (DialogData    *data);
static void        default_week_dialog_set_selected_day          (DialogData    *data,
								  MrpDay        *day);
static PlannerCmd *default_week_cmd_edit                         (DialogData    *data,
								  gint           weekday,
								  MrpDay        *day);


static void
default_week_dialog_response_cb (GtkWidget  *dialog,
				 gint        response,
				 DialogData *data)
{
	MrpDay *day;
	gint    weekday;

	switch (response) {
	case RESPONSE_APPLY:
		weekday = default_week_dialog_get_selected_weekday (data);
		day = default_week_dialog_get_selected_day (data);

		default_week_cmd_edit (data, weekday, day);

/*		mrp_calendar_set_default_days (data->calendar,
					       weekday, day,
					       -1);
*/
		break;

	case RESPONSE_CLOSE:
		gtk_widget_destroy (data->dialog);
		break;

	case GTK_RESPONSE_DELETE_EVENT:
		break;

	default:
		g_assert_not_reached ();
	}
}

static void
default_week_dialog_parent_destroy_cb (GtkWidget *window, GtkWidget *dialog)
{
	gtk_widget_destroy (dialog);
}

static void
daytype_combo_name_and_sensitive (GtkCellLayout   *layout,
                                  GtkCellRenderer *cell,
                                  GtkTreeModel    *model,
                                  GtkTreeIter     *iter,
                                  gpointer         user_data)
{
	MrpDay      *day;
	gboolean     sensitive;

	gtk_tree_model_get (model, iter, 0, &day, 1, &sensitive, -1);
	g_object_set (cell,
		      "text", mrp_day_get_name (day),
		      "sensitive", sensitive,
		      NULL);
}

GtkWidget *
planner_default_week_dialog_new (PlannerWindow *window,
				 MrpCalendar  *calendar)
{
	DialogData *data;
	GtkBuilder *builder;
	GtkWidget  *dialog;
	GtkWidget  *w;
	gint        i;
	GtkCellRenderer *renderer;

	g_return_val_if_fail (PLANNER_IS_WINDOW (window), NULL);

	builder = gtk_builder_new_from_resource ("/app/drey/Planner/ui/default-week-dialog.ui");

	dialog = GTK_WIDGET (gtk_builder_get_object (builder, "default_week_dialog"));

	data = g_new0 (DialogData, 1);

	data->main_window = window;
	data->calendar = calendar;
	data->project = planner_window_get_project (window);
	data->dialog = dialog;

	g_signal_connect_object (window,
				 "destroy",
				 G_CALLBACK (default_week_dialog_parent_destroy_cb),
				 dialog,
				 0);

	/* Get the from/to labels. */
	for (i = 0; i < 5; i++) {
		gchar *tmp;

		tmp = g_strdup_printf ("from%d_label", i + 1);
		data->from_label[i] = GTK_WIDGET (gtk_builder_get_object (builder, tmp));
		g_free (tmp);

		tmp = g_strdup_printf ("to%d_label", i + 1);
		data->to_label[i] = GTK_WIDGET (gtk_builder_get_object (builder, tmp));
		g_free (tmp);

		tmp = g_strdup_printf ("dash%d_label", i + 1);
		data->dash_label[i] = GTK_WIDGET (gtk_builder_get_object (builder, tmp));
		g_free (tmp);
	}

	w = GTK_WIDGET (gtk_builder_get_object (builder, "name_label"));
	gtk_label_set_text (GTK_LABEL (w), mrp_calendar_get_name (calendar));

	data->weekday_combobox = GTK_WIDGET (gtk_builder_get_object (builder, "weekday_combobox"));
	data->daytype_combobox = GTK_WIDGET (gtk_builder_get_object (builder, "daytype_combobox"));

	default_week_dialog_setup_daytype_combobox (GTK_COMBO_BOX (data->daytype_combobox),
						    data->project,
						    calendar);

	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (data->daytype_combobox),
				    renderer, TRUE);
	gtk_cell_layout_set_cell_data_func (GTK_CELL_LAYOUT (data->daytype_combobox),
					    renderer,
					    daytype_combo_name_and_sensitive,
					    NULL, NULL);

	g_signal_connect (data->daytype_combobox,
			  "changed",
			  G_CALLBACK (default_week_dialog_day_selected_cb),
			  data);

	g_signal_connect (data->weekday_combobox,
			  "changed",
			  G_CALLBACK (default_week_dialog_weekday_selected_cb),
			  data);

	default_week_dialog_setup_weekday_combobox (GTK_COMBO_BOX_TEXT (data->weekday_combobox));

	g_signal_connect (dialog,
			  "response",
			  G_CALLBACK (default_week_dialog_response_cb),
			  data);

	g_object_set_data_full (G_OBJECT (dialog),
				"data", data,
				g_free);

	default_week_dialog_update_labels (data);

	g_object_unref (builder);
	return dialog;
}

static void
default_week_dialog_update_labels (DialogData *data)
{
	gint         i;
	MrpDay      *day;
	GList       *ivals, *l;
	MrpInterval *ival;
	gchar       *str;
	mrptime      start, end;

	day = default_week_dialog_get_selected_day (data);

	/* Special case "use base", since we can't get intervals for that
	 * type.
	 */
	if (day == mrp_day_get_use_base ()) {
		MrpCalendar *parent;
		gint         weekday;

		parent = mrp_calendar_get_parent (data->calendar);
		if (parent) {
			weekday = default_week_dialog_get_selected_weekday (data);

			day = mrp_calendar_get_default_day (parent, weekday);

			ivals = mrp_calendar_day_get_intervals (parent, day, TRUE);
		} else {
			ivals = NULL;
		}
	} else {
		ivals = mrp_calendar_day_get_intervals (data->calendar, day, TRUE);
	}

	for (i = 0; i < 5; i++) {
		gtk_label_set_text (GTK_LABEL (data->from_label[i]), "");
		gtk_label_set_text (GTK_LABEL (data->to_label[i]), "");
		gtk_label_set_text (GTK_LABEL (data->dash_label[i]), "");
	}

	/* No intervals. */
	if (!ivals) {
		gchar *tmp;

		tmp = g_strconcat ("<i>", _("No working time"), "</i>", NULL);
		gtk_label_set_markup (GTK_LABEL (data->from_label[0]), tmp);
		g_free (tmp);
	}

	i = 0;
	for (l = ivals; l; l = l->next) {
		ival = l->data;

		mrp_interval_get_absolute (ival, 0, &start, &end);

		str = mrp_time_format (_("%H:%M"), start);
		gtk_label_set_text (GTK_LABEL (data->from_label[i]), str);
		g_free (str);

		str = mrp_time_format (_("%H:%M"), end);
		gtk_label_set_text (GTK_LABEL (data->to_label[i]), str);
		g_free (str);

		gtk_label_set_text (GTK_LABEL (data->dash_label[i]), "-");

		if (i++ > 5) {
			break;
		}
	}
}

static void
default_week_dialog_weekday_selected_cb (GtkComboBoxText *combotext,
					 DialogData      *data)
{
	gint    weekday;
	MrpDay *day;

	weekday = default_week_dialog_get_selected_weekday (data);
	day = mrp_calendar_get_default_day (data->calendar, weekday);

	default_week_dialog_set_selected_day (data, day);

	default_week_dialog_update_labels (data);
}

static void
default_week_dialog_day_selected_cb (GtkComboBox *combo,
				     DialogData  *data)
{
	default_week_dialog_update_labels (data);
}

static void
default_week_dialog_setup_daytype_combobox (GtkComboBox *combo,
					    MrpProject  *project,
					    MrpCalendar *calendar)
{
	GtkListStore    *store;
	GtkTreeIter      iter;
	GList           *days, *l;

	store = gtk_list_store_new (2, MRP_TYPE_DAY, G_TYPE_BOOLEAN);
	gtk_combo_box_set_model (combo, GTK_TREE_MODEL (store));
	g_object_unref (store);

	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter,
			    0, mrp_day_get_work (),
			    1, TRUE,
			    -1);
	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter,
			    0, mrp_day_get_nonwork (),
			    1, TRUE,
			    -1);
	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter,
			    0, mrp_day_get_use_base (),
			    1, !(mrp_calendar_get_parent (calendar) == mrp_project_get_root_calendar (project)),
			    -1);

	days = mrp_day_get_all (project);

	for (l = days; l; l = l->next) {
		gtk_list_store_append (store, &iter);
		gtk_list_store_set (store, &iter,
				    0, MRP_DAY (l->data),
				    1, TRUE,
				    -1);
	}

	/* Need to unref the days here? */
	g_list_free (days);
}

static gint
default_week_dialog_get_selected_weekday (DialogData *data)
{
	return days[gtk_combo_box_get_active (GTK_COMBO_BOX (data->weekday_combobox))].day;
}

static void
default_week_dialog_setup_weekday_combobox (GtkComboBoxText *combotext)
{
	gint i;

	for (i = 0; i < G_N_ELEMENTS(days); i++) {
		gtk_combo_box_text_append_text (combotext, _(days[i].name));
	}
	gtk_combo_box_set_active (GTK_COMBO_BOX (combotext), 0);
}

static MrpDay *
default_week_dialog_get_selected_day (DialogData *data)
{
	GtkTreeIter  iter;
	MrpDay      *day;

	gtk_combo_box_get_active_iter (GTK_COMBO_BOX (data->daytype_combobox), &iter);
	gtk_tree_model_get (gtk_combo_box_get_model (GTK_COMBO_BOX (data->daytype_combobox)),
			    &iter,
			    0, &day,
			    -1);
	return day;
}

static void
default_week_dialog_set_selected_day (DialogData *data,
				      MrpDay     *day)
{
	GtkComboBox  *combo;
	GtkTreeModel *model;
	GtkTreeIter   iter;
	MrpDay       *iter_day;

	combo = GTK_COMBO_BOX (data->daytype_combobox);
	model = gtk_combo_box_get_model (combo);
	gtk_tree_model_get_iter_first (model, &iter);

	do {
		gtk_tree_model_get (model, &iter, 0, &iter_day, -1);
		if (iter_day == day) {
			gtk_combo_box_set_active_iter (combo, &iter);
			return;
		}
	} while (gtk_tree_model_iter_next (model, &iter));
}

static gboolean
is_day_builtin (MrpDay *day)
{
	if (day == mrp_day_get_work ()) {
		return TRUE;
	}
	else if (day == mrp_day_get_nonwork ()) {
		return TRUE;
	}
	else if (day == mrp_day_get_use_base ()) {
		return TRUE;
	} else {
		return FALSE;
	}
}

static gboolean
default_week_cmd_edit_do (PlannerCmd *cmd_base)
{
	DefaultWeekCmdEdit *cmd;
	MrpDay             *day;

	cmd = (DefaultWeekCmdEdit *) cmd_base;

	day = mrp_calendar_get_default_day (cmd->calendar, cmd->weekday);

	if (is_day_builtin (day)) {
		cmd->old_day = day;
	} else {
		cmd->old_day = NULL;
		cmd->old_day_id = mrp_day_get_id (day);
	}

	if (cmd->day) {
		day = cmd->day;
	} else {
		day = mrp_project_get_calendar_day_by_id (cmd->project, cmd->day_id);
	}

	mrp_calendar_set_default_days (cmd->calendar,
				       cmd->weekday, day,
				       -1);

	return TRUE;
}

static void
default_week_cmd_edit_undo (PlannerCmd *cmd_base)
{
	DefaultWeekCmdEdit *cmd;
	MrpDay             *day;

	cmd = (DefaultWeekCmdEdit *) cmd_base;

	if (is_day_builtin (cmd->old_day)) {
		day = cmd->old_day;
	} else {
		day = mrp_project_get_calendar_day_by_id (cmd->project, cmd->old_day_id);
	}

	mrp_calendar_set_default_days (cmd->calendar,
				       cmd->weekday, day,
				       -1);
}

static void
default_week_cmd_edit_free (PlannerCmd *cmd_base)
{
/* FIXME: cmd_base not freed
	DefaultWeekCmdEdit *cmd;

	cmd = (DefaultWeekCmdEdit *) cmd_base;
*/
}

static PlannerCmd *
default_week_cmd_edit (DialogData *data,
		       gint        weekday,
		       MrpDay     *day)
{
	PlannerCmd         *cmd_base;
	DefaultWeekCmdEdit *cmd;

	cmd_base = planner_cmd_new (DefaultWeekCmdEdit,
				    _("Edit default week"),
 				    default_week_cmd_edit_do,
				    default_week_cmd_edit_undo,
				    default_week_cmd_edit_free);

	cmd = (DefaultWeekCmdEdit *) cmd_base;

	cmd->project = data->project;
	cmd->calendar = data->calendar;
	cmd->weekday = weekday;

	if (is_day_builtin (day)) {
		cmd->day = day;
	} else {
		cmd->day_id = mrp_day_get_id (day);
	}

	planner_cmd_manager_insert_and_do (planner_window_get_cmd_manager (data->main_window),
					   cmd_base);

	return cmd_base;
}
