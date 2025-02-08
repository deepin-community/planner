/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2001-2002 CodeFactory AB
 * Copyright (C) 2001-2002 Richard Hult <richard@imendio.com>
 * Copyright (C) 2001-2002 Mikael Hallendal <micke@imendio.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#pragma once

#include <glib-object.h>
#include <time.h>

#include <libplanner/mrp-object.h>
#include <libplanner/mrp-types.h>
#include <libplanner/mrp-time.h>

G_BEGIN_DECLS

#define MRP_TYPE_CALENDAR		(mrp_calendar_get_type ())

G_DECLARE_FINAL_TYPE (MrpCalendar, mrp_calendar, MRP, CALENDAR, MrpObject)

#define MRP_TYPE_INTERVAL               (mrp_interval_get_type ())

/**
 * MrpCalendar:
 *
 * Object representing a calendar in the project.
 */


/**
 * MrpInterval:
 *
 * Represents a time interval.
 */
typedef struct _MrpInterval         MrpInterval;

#include <libplanner/mrp-day.h>

/* Used for saving calendar data. */

/**
 * MrpDayWithIntervals:
 * @day: a day type.
 * @intervals: a list of time intervals.
 *
 * Used for saving calendar data.
 *
 * Associate a @day type and time @intervals.
 * A day type is composed of working periods.
 */
typedef struct {
	MrpDay *day;
	GList *intervals;
} MrpDayWithIntervals;

/**
 * MrpDateWithDay:
 * @date: a date.
 * @day: a day type.
 *
 * Associate a @date and a @day type.
 * Certain dates can have a peculiar day type.
 */
typedef struct {
	mrptime date;
	MrpDay *day;
} MrpDateWithDay;

enum {
	MRP_CALENDAR_DAY_SUN,
	MRP_CALENDAR_DAY_MON,
	MRP_CALENDAR_DAY_TUE,
	MRP_CALENDAR_DAY_WED,
	MRP_CALENDAR_DAY_THU,
	MRP_CALENDAR_DAY_FRI,
	MRP_CALENDAR_DAY_SAT
};

MrpCalendar *mrp_calendar_new                      (const gchar *name,
						    MrpProject  *project);
void         mrp_calendar_add                      (MrpCalendar *calendar,
						    MrpCalendar *parent);
MrpCalendar *mrp_calendar_copy                     (const gchar *name,
						    MrpCalendar *calendar);
MrpCalendar *mrp_calendar_derive                   (const gchar *name,
						    MrpCalendar *parent);
void         mrp_calendar_reparent                 (MrpCalendar *new_parent,
						    MrpCalendar *child);
void         mrp_calendar_remove                   (MrpCalendar *calendar);
const gchar *mrp_calendar_get_name                 (MrpCalendar *calendar);
void         mrp_calendar_set_name                 (MrpCalendar *calendar,
						    const gchar *name);
void         mrp_calendar_day_set_intervals        (MrpCalendar *calendar,
						    MrpDay      *day,
						    GList       *intervals);
GList *      mrp_calendar_day_get_intervals        (MrpCalendar *calendar,
						    MrpDay      *day,
						    gboolean     check_ancestors);
gint         mrp_calendar_day_get_total_work       (MrpCalendar *calendar,
						    MrpDay      *day);
MrpDay *     mrp_calendar_get_day                  (MrpCalendar *calendar,
						    mrptime      date,
						    gboolean     check_ancestors);
MrpDay *     mrp_calendar_get_default_day          (MrpCalendar *calendar,
						    gint         week_day);
void         mrp_calendar_set_default_days         (MrpCalendar *calendar,
						    gint         week_day,
						    ...);
void         mrp_calendar_set_days                 (MrpCalendar *calendar,
						    mrptime      date,
						    ...);
MrpCalendar *mrp_calendar_get_parent               (MrpCalendar *calendar);
GList *      mrp_calendar_get_children             (MrpCalendar *calendar);
GList *      mrp_calendar_get_overridden_days      (MrpCalendar *calendar);
GList *      mrp_calendar_get_all_overridden_dates (MrpCalendar *calendar);

/* Interval */
GType        mrp_interval_get_type                 (void) G_GNUC_CONST;
MrpInterval *mrp_interval_new                      (mrptime      start,
						    mrptime      end);
MrpInterval *mrp_interval_copy                     (MrpInterval *interval);
MrpInterval *mrp_interval_ref                      (MrpInterval *interval);
void         mrp_interval_unref                    (MrpInterval *interval);
void         mrp_interval_get_absolute             (MrpInterval *interval,
						    mrptime      offset,
						    mrptime     *start,
						    mrptime     *end);
void         mrp_interval_set_absolute             (MrpInterval *interval,
						    mrptime      offset,
						    mrptime      start,
						    mrptime      end);


G_END_DECLS
