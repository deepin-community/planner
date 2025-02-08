/* planner-calendar-popover.c
 *
 * Copyright 2021-2022 Mart Raudsepp <mart@leio.tech>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "planner-calendar-popover.h"
#include "libplanner/mrp-types.h"
#include "libplanner/mrp-time.h"

struct _PlannerCalendarPopover
{
  GtkPopover         parent_instance;

  gboolean           use_constraint;
  MrpConstraintType  constraint_type;

  GtkWidget         *calendar;
  GtkWidget         *today_button;
  GtkWidget         *constraint_box;
  GtkWidget         *constraint_combo;
};

G_DEFINE_TYPE (PlannerCalendarPopover, planner_calendar_popover, GTK_TYPE_POPOVER)

enum {
  PROP_0,
  PROP_USE_CONSTRAINT,
  PROP_CONSTRAINT_TYPE,
  PROP_MRPTIME,
  N_PROPS
};

enum {
  CONSTRAINT_COMBO_ASAP = 0,
  CONSTRAINT_COMBO_SNET,
  CONSTRAINT_COMBO_MSO,
};

static GParamSpec *properties [N_PROPS];

static gboolean
constraint_type_to_calendar_sensitivity (GBinding     *binding,
                                         const GValue *from_value,
                                         GValue       *to_value,
                                         gpointer      user_data)
{
  MrpConstraintType type;
  gboolean sensitive;
  PlannerCalendarPopover *self = user_data;

  type = g_value_get_int (from_value);
  sensitive = !self->use_constraint || (type != MRP_CONSTRAINT_ASAP &&
                                        type != MRP_CONSTRAINT_ALAP);

  g_value_set_boolean (to_value, sensitive);

  return TRUE;
}

static void
update_constraint_combo_value (PlannerCalendarPopover *self)
{
  switch (self->constraint_type)
    {
    case MRP_CONSTRAINT_ASAP:
      gtk_combo_box_set_active (GTK_COMBO_BOX (self->constraint_combo),
                                CONSTRAINT_COMBO_ASAP);
      break;
    case MRP_CONSTRAINT_SNET:
      gtk_combo_box_set_active (GTK_COMBO_BOX (self->constraint_combo),
                                CONSTRAINT_COMBO_SNET);
      break;
    case MRP_CONSTRAINT_MSO:
      gtk_combo_box_set_active (GTK_COMBO_BOX (self->constraint_combo),
                                CONSTRAINT_COMBO_MSO);
      break;
    default:
      g_assert_not_reached ();
    }
}

static void
on_calendar_day_selected (PlannerCalendarPopover *self,
                          GtkCalendar            *calendar)
{
  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_MRPTIME]);
}

static void
on_calendar_day_double_clicked (GtkPopover  *popover,
                                GtkCalendar *calendar)
{
  gtk_popover_popdown (popover);
}

static void
on_today_clicked (PlannerCalendarPopover *self,
                  GtkWidget              *button)
{
  gint    year, month, day;
  mrptime today;

  today = mrp_time_current_time ();
  mrp_time_decompose (today,
                      &year, &month, &day,
                      NULL, NULL, NULL);

  gtk_calendar_select_month (GTK_CALENDAR (self->calendar),
                             month - 1, year);
  gtk_calendar_select_day (GTK_CALENDAR (self->calendar), day);
}

static void
on_constraint_changed_cb (PlannerCalendarPopover *self,
                          GtkComboBox *combo)
{
  switch (gtk_combo_box_get_active (combo))
    {
    case CONSTRAINT_COMBO_ASAP:
      self->constraint_type = MRP_CONSTRAINT_ASAP;
      break;
    case CONSTRAINT_COMBO_SNET:
      self->constraint_type = MRP_CONSTRAINT_SNET;
      break;
    case CONSTRAINT_COMBO_MSO:
      self->constraint_type = MRP_CONSTRAINT_MSO;
      break;
    default:
      g_assert_not_reached ();
    }

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_CONSTRAINT_TYPE]);
}

PlannerCalendarPopover *
planner_calendar_popover_new (gboolean use_constraint)
{
  return g_object_new (PLANNER_TYPE_CALENDAR_POPOVER,
                       "use-constraint", use_constraint,
                       NULL);
}

static void
planner_calendar_popover_get_property (GObject    *object,
                                       guint       prop_id,
                                       GValue     *value,
                                       GParamSpec *pspec)
{
  PlannerCalendarPopover *self = PLANNER_CALENDAR_POPOVER (object);

  switch (prop_id)
    {
    case PROP_USE_CONSTRAINT:
      g_value_set_boolean (value, self->use_constraint);
      break;
    case PROP_CONSTRAINT_TYPE:
      g_value_set_int (value, self->constraint_type);
      break;
    case PROP_MRPTIME:
      {
        mrptime t;
        gint year, month, day;

        g_object_get (self->calendar,
                      "year", &year,
                      "month", &month,
                      "day", &day,
                      NULL);
        t = mrp_time_compose (year, month + 1, day, 0, 0, 0);
        g_value_set_int64 (value, t);
      }
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
planner_calendar_popover_set_property (GObject      *object,
                                       guint         prop_id,
                                       const GValue *value,
                                       GParamSpec   *pspec)
{
  PlannerCalendarPopover *self = PLANNER_CALENDAR_POPOVER (object);

  switch (prop_id)
    {
    case PROP_USE_CONSTRAINT:
      self->use_constraint = g_value_get_boolean (value);
      break;
    case PROP_CONSTRAINT_TYPE:
      self->constraint_type = g_value_get_int (value);
      update_constraint_combo_value (self);
      break;
    case PROP_MRPTIME:
      {
        mrptime time;
        gint year, month, day;

        time = g_value_get_int64 (value);
        mrp_time_decompose (time, &year, &month, &day, NULL, NULL, NULL);

        g_object_set (self->calendar,
                      "year", year,
                      "month", month - 1,
                      "day", day,
                      NULL);
      }
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
planner_calendar_popover_constructed (GObject *object)
{
  PlannerCalendarPopover *self;

  self = PLANNER_CALENDAR_POPOVER (object);

  G_OBJECT_CLASS (planner_calendar_popover_parent_class)->constructed (object);

  g_object_bind_property (self, "use-constraint",
                          self->constraint_box, "visible",
                          G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
  g_object_bind_property_full (self, "constraint-type",
                               self->calendar, "sensitive",
                               G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE,
                               constraint_type_to_calendar_sensitivity, NULL,
                               self, NULL);
  g_object_bind_property_full (self, "constraint-type",
                               self->today_button, "sensitive",
                               G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE,
                               constraint_type_to_calendar_sensitivity, NULL,
                               self, NULL);
}

static void
planner_calendar_popover_class_init (PlannerCalendarPopoverClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = planner_calendar_popover_get_property;
  object_class->set_property = planner_calendar_popover_set_property;
  object_class->constructed = planner_calendar_popover_constructed;

  properties [PROP_USE_CONSTRAINT] =
    g_param_spec_boolean ("use-constraint",
                          NULL, NULL, FALSE,
                          /* constraint_type_to_calendar_sensitivity assumes this is locked after construction */
                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
  /* TODO: Convert MrpConstraintType into a GEnum and use spec_enum */
  properties [PROP_CONSTRAINT_TYPE] =
    g_param_spec_int ("constraint-type",
                      NULL, NULL,
                      MRP_CONSTRAINT_ASAP, MRP_CONSTRAINT_MSO,
                      MRP_CONSTRAINT_MSO,
                      G_PARAM_READWRITE);
  properties [PROP_MRPTIME] =
    mrp_param_spec_time ("mrptime",
                         NULL, NULL,
                         G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, N_PROPS, properties);

  gtk_widget_class_set_template_from_resource (widget_class, "/app/drey/Planner/ui/planner-calendar-popover.ui");

  gtk_widget_class_bind_template_child (widget_class, PlannerCalendarPopover, calendar);
  gtk_widget_class_bind_template_child (widget_class, PlannerCalendarPopover, today_button);
  gtk_widget_class_bind_template_child (widget_class, PlannerCalendarPopover, constraint_box);
  gtk_widget_class_bind_template_child (widget_class, PlannerCalendarPopover, constraint_combo);

  gtk_widget_class_bind_template_callback (widget_class, on_calendar_day_selected);
  gtk_widget_class_bind_template_callback (widget_class, on_calendar_day_double_clicked);
  gtk_widget_class_bind_template_callback (widget_class, on_today_clicked);
  gtk_widget_class_bind_template_callback (widget_class, on_constraint_changed_cb);
}

static void
planner_calendar_popover_init (PlannerCalendarPopover *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  self->constraint_type = MRP_CONSTRAINT_MSO;
  update_constraint_combo_value (self);
}
