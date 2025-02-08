/* planner-cell-renderer-calendar.c
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

#include "planner-cell-renderer-calendar.h"
#include "planner-calendar-popover.h"
#include "planner-marshal.h"
#include "planner-task-tree.h"

struct _PlannerCellRendererCalendar
{
  GtkCellRendererText  parent_instance;

  GtkWidget           *popover;

  gboolean             use_constraint;
  gboolean             editable;

  gchar               *active_path;
};

G_DEFINE_TYPE (PlannerCellRendererCalendar, planner_cell_renderer_calendar, GTK_TYPE_CELL_RENDERER_TEXT)

enum {
  PROP_0,
  PROP_USE_CONSTRAINT,
  PROP_CONSTRAINT_TYPE,
  PROP_MRPTIME,
  N_PROPS,

  PROP_EDITABLE,
};

static GParamSpec *properties [N_PROPS];

enum {
  CALENDAR_ACTIVATED,
  CALENDAR_MRPTIME_CHANGED,
  CALENDAR_CONSTRAINT_TYPE_CHANGED,
  CALENDAR_CLOSED,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];

PlannerCellRendererCalendar *
planner_cell_renderer_calendar_new (gboolean use_constraint)
{
  return g_object_new (PLANNER_TYPE_CELL_RENDERER_CALENDAR,
                       "use-constraint", use_constraint,
                       NULL);
}

static void
planner_cell_renderer_calendar_finalize (GObject *object)
{
  PlannerCellRendererCalendar *self = (PlannerCellRendererCalendar *)object;

  g_clear_pointer (&self->popover, gtk_widget_destroy);
  g_free (self->active_path);

  G_OBJECT_CLASS (planner_cell_renderer_calendar_parent_class)->finalize (object);
}

static void
planner_cell_renderer_calendar_get_property (GObject    *object,
                                             guint       prop_id,
                                             GValue     *value,
                                             GParamSpec *pspec)
{
  PlannerCellRendererCalendar *self = PLANNER_CELL_RENDERER_CALENDAR (object);

  switch (prop_id)
    {
    case PROP_USE_CONSTRAINT:
      g_value_set_boolean (value, self->use_constraint);
      break;
    case PROP_EDITABLE:
      g_value_set_boolean (value, self->editable);
      break;
    case PROP_CONSTRAINT_TYPE:
      {
        int type;

        g_object_get (self->popover, "constraint-type", &type, NULL);
        g_value_set_int (value, type);
      }
      break;
    case PROP_MRPTIME:
      {
        mrptime t;

        g_object_get (self->popover, "mrptime", &t, NULL);
        g_value_set_int64 (value, t);
      }
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
planner_cell_renderer_calendar_set_property (GObject      *object,
                                             guint         prop_id,
                                             const GValue *value,
                                             GParamSpec   *pspec)
{
  PlannerCellRendererCalendar *self = PLANNER_CELL_RENDERER_CALENDAR (object);

  switch (prop_id)
    {
    case PROP_USE_CONSTRAINT:
      self->use_constraint = g_value_get_boolean (value);
      break;
    case PROP_EDITABLE:
      self->editable = g_value_get_boolean (value);
      if (self->editable)
        g_object_set(self, "mode", GTK_CELL_RENDERER_MODE_ACTIVATABLE, NULL);
      else
        g_object_set(self, "mode", GTK_CELL_RENDERER_MODE_INERT, NULL);
      break;
    case PROP_CONSTRAINT_TYPE:
      g_object_set (self->popover, "constraint-type", g_value_get_int (value), NULL);
      break;
    case PROP_MRPTIME:
      g_object_set (self->popover, "mrptime", g_value_get_int64 (value), NULL);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
planner_cell_renderer_calendar_mrptime_changed_cb (PlannerCellRendererCalendar *self,
                                                   GParamSpec                  *spec,
                                                   PlannerCalendarPopover      *popover)
{
  mrptime t;

  g_object_get (popover, "mrptime", &t, NULL);
  g_signal_emit (self, signals[CALENDAR_MRPTIME_CHANGED], 0, self->active_path, t);
}

static void
planner_cell_renderer_calendar_constraint_type_changed_cb (PlannerCellRendererCalendar *self,
                                                           GParamSpec                  *spec,
                                                           PlannerCalendarPopover      *popover)
{
  MrpConstraintType constraint_type;

  g_object_get (popover, "constraint-type", &constraint_type, NULL);
  g_signal_emit (self, signals[CALENDAR_CONSTRAINT_TYPE_CHANGED], 0, self->active_path, constraint_type);
}

static void
planner_cell_renderer_calendar_popover_closed_cb (PlannerCellRendererCalendar *self,
                                                  GtkPopover                  *popover)
{
  g_signal_emit (self, signals[CALENDAR_CLOSED], 0, self->active_path);
}

static gint
planner_cell_renderer_calendar_activate (GtkCellRenderer      *cell,
                                         GdkEvent             *event,
                                         GtkWidget            *widget,
                                         const char           *path,
                                         const GdkRectangle   *background_area,
                                         const GdkRectangle   *cell_area,
                                         GtkCellRendererState  flags)
{
  PlannerCellRendererCalendar *self = PLANNER_CELL_RENDERER_CALENDAR (cell);
  GdkRectangle rect;
  GtkRequisition header_req;
  GtkAdjustment *hadj;

  g_return_val_if_fail (GTK_IS_TREE_VIEW (widget), FALSE);

  g_free (self->active_path);
  self->active_path = g_strdup (path);

  g_signal_handlers_block_by_func (self->popover, planner_cell_renderer_calendar_mrptime_changed_cb, self);
  g_signal_handlers_block_by_func (self->popover, planner_cell_renderer_calendar_constraint_type_changed_cb, self);
  g_signal_emit (self, signals[CALENDAR_ACTIVATED], 0, path);
  g_signal_handlers_unblock_by_func (self->popover, planner_cell_renderer_calendar_constraint_type_changed_cb, self);
  g_signal_handlers_unblock_by_func (self->popover, planner_cell_renderer_calendar_mrptime_changed_cb, self);

  /* TODO: Better way to position the popover? */
  hadj = gtk_scrollable_get_hadjustment (GTK_SCROLLABLE (widget));
  gtk_widget_get_preferred_size (gtk_tree_view_column_get_button (gtk_tree_view_get_column (GTK_TREE_VIEW (widget), 0)), &header_req, NULL);
  rect.x = cell_area->x - gtk_adjustment_get_value (hadj);
  rect.y = cell_area->y + header_req.height;
  rect.width = cell_area->width;
  rect.height = cell_area->height;

  gtk_popover_set_relative_to (GTK_POPOVER (self->popover), widget);
  gtk_popover_set_pointing_to (GTK_POPOVER (self->popover), &rect);
  gtk_popover_popup (GTK_POPOVER (self->popover));

  return TRUE;
}

static void
planner_cell_renderer_calendar_constructed (GObject *object)
{
  PlannerCellRendererCalendar *self;

  self = PLANNER_CELL_RENDERER_CALENDAR (object);

  G_OBJECT_CLASS (planner_cell_renderer_calendar_parent_class)->constructed (object);

  self->popover = GTK_WIDGET (planner_calendar_popover_new (self->use_constraint));
  gtk_popover_set_constrain_to (GTK_POPOVER (self->popover), GTK_POPOVER_CONSTRAINT_WINDOW);

  g_signal_connect_object (self->popover,
                           "notify::mrptime",
                           G_CALLBACK (planner_cell_renderer_calendar_mrptime_changed_cb),
                           self,
                           G_CONNECT_SWAPPED);
  g_signal_connect_object (self->popover,
                           "notify::constraint-type",
                           G_CALLBACK (planner_cell_renderer_calendar_constraint_type_changed_cb),
                           self,
                           G_CONNECT_SWAPPED);
  g_signal_connect_object (self->popover,
                           "closed",
                           G_CALLBACK (planner_cell_renderer_calendar_popover_closed_cb),
                           self,
                           G_CONNECT_SWAPPED);
}

static void
planner_cell_renderer_calendar_class_init (PlannerCellRendererCalendarClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkCellRendererClass *cell_class = GTK_CELL_RENDERER_CLASS (klass);

  object_class->finalize = planner_cell_renderer_calendar_finalize;
  object_class->get_property = planner_cell_renderer_calendar_get_property;
  object_class->set_property = planner_cell_renderer_calendar_set_property;
  object_class->constructed = planner_cell_renderer_calendar_constructed;

  cell_class->activate = planner_cell_renderer_calendar_activate;

  g_object_class_override_property (object_class, PROP_EDITABLE, "editable");

  properties [PROP_USE_CONSTRAINT] =
    g_param_spec_boolean ("use-constraint",
                          NULL, NULL, TRUE,
                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
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

  signals[CALENDAR_ACTIVATED] =
    g_signal_new ("calendar-activated",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0, NULL, NULL,
                  planner_marshal_VOID__STRING,
                  G_TYPE_NONE, 1,
                  G_TYPE_STRING);

  signals[CALENDAR_MRPTIME_CHANGED] =
    g_signal_new ("calendar-mrptime-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0, NULL, NULL,
                  planner_marshal_VOID__STRING_INT64,
                  G_TYPE_NONE, 2,
                  G_TYPE_STRING,
                  G_TYPE_INT64);

  signals[CALENDAR_CONSTRAINT_TYPE_CHANGED] =
    g_signal_new ("calendar-constraint-type-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0, NULL, NULL,
                  planner_marshal_VOID__STRING_INT,
                  G_TYPE_NONE, 2,
                  G_TYPE_STRING,
                  G_TYPE_INT);

  signals[CALENDAR_CLOSED] =
    g_signal_new ("calendar-closed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0, NULL, NULL,
                  planner_marshal_VOID__STRING,
                  G_TYPE_NONE, 1,
                  G_TYPE_STRING);
}

static void
planner_cell_renderer_calendar_init (PlannerCellRendererCalendar *self)
{
  g_object_set (self, "mode", GTK_CELL_RENDERER_MODE_INERT, NULL);
}
