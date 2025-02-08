/* planner-calendar-popover.h
 *
 * Copyright 2021 Mart Raudsepp <mart@leio.tech>
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

#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define PLANNER_TYPE_CALENDAR_POPOVER (planner_calendar_popover_get_type())

G_DECLARE_FINAL_TYPE (PlannerCalendarPopover, planner_calendar_popover, PLANNER, CALENDAR_POPOVER, GtkPopover)

PlannerCalendarPopover *planner_calendar_popover_new (gboolean use_constraint);

G_END_DECLS
