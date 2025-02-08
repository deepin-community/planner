/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2005 Imendio AB
 * Copyright (C) 2002 CodeFactory AB
 * Copyright (C) 2002 Richard Hult <richard@imendio.com>
 * Copyright (C) 2002 Mikael Hallendal <micke@imendio.com>
 * Copyright (C) 2002 Alvaro del Castillo <acs@barrapunto.com>
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

#include <time.h>
#include <glib.h>
#include <glib-object.h> /* GParam{Flags,Spec} */

/**
 * mrptime:
 *
 * Number of seconds that have elapsed since 1970-01-01 00:00:00 UTC.
 */
typedef gint64 mrptime;

/**
 * MRP_TIME_INVALID:
 *
 * Represents an invalid #mrptime value.
 */
#define MRP_TIME_INVALID G_GINT64_CONSTANT(0)
/**
 * MRP_TIME_MIN:
 *
 * Represents the minimum value for #mrptime.
 */
#define MRP_TIME_MIN G_GINT64_CONSTANT(0)
/**
 * MRP_TIME_MAX:
 *
 * Represents the maximum value for #mrptime.
 */
#define MRP_TIME_MAX G_GINT64_CONSTANT(253402300799)

/**
 * MrpTimeUnit:
 * @MRP_TIME_UNIT_NONE: none.
 * @MRP_TIME_UNIT_YEAR: year.
 * @MRP_TIME_UNIT_HALFYEAR: half year.
 * @MRP_TIME_UNIT_QUARTER: quarter.
 * @MRP_TIME_UNIT_MONTH: month.
 * @MRP_TIME_UNIT_WEEK: week.
 * @MRP_TIME_UNIT_DAY: day.
 * @MRP_TIME_UNIT_HALFDAY: half day.
 * @MRP_TIME_UNIT_TWO_HOURS: two hours.
 * @MRP_TIME_UNIT_HOUR: hour.
 *
 * Time granularity meaningful to humans.
 */
typedef enum {
	MRP_TIME_UNIT_NONE,
	MRP_TIME_UNIT_YEAR,
	MRP_TIME_UNIT_HALFYEAR,
	MRP_TIME_UNIT_QUARTER,
	MRP_TIME_UNIT_MONTH,
	MRP_TIME_UNIT_WEEK,
	MRP_TIME_UNIT_DAY,
	MRP_TIME_UNIT_HALFDAY,
	MRP_TIME_UNIT_TWO_HOURS,
	MRP_TIME_UNIT_HOUR
} MrpTimeUnit;

mrptime      mrp_time_current_time       (void);
mrptime      mrp_time_compose            (gint          year,
					  gint          month,
					  gint          day,
					  gint          hour,
					  gint          minute,
					  gint          second);
gboolean     mrp_time_decompose          (mrptime       t,
					  gint         *year,
					  gint         *month,
					  gint         *day,
					  gint         *hour,
					  gint         *minute,
					  gint         *second);
mrptime      mrp_time_from_string        (const gchar  *str);
gchar *      mrp_time_to_string          (mrptime       t);
mrptime      mrp_time_align_day          (mrptime       t);
mrptime      mrp_time_align_prev         (mrptime       t,
					  MrpTimeUnit   unit);
mrptime      mrp_time_align_next         (mrptime       t,
					  MrpTimeUnit   unit);
gint         mrp_time_day_of_week        (mrptime       t);
gchar *      mrp_time_format             (const gchar  *format,
					  mrptime       t);
gchar *      mrp_time_format_locale      (mrptime       t);
void         mrp_time_debug_print        (mrptime       t);
GParamSpec * mrp_param_spec_time         (const gchar  *name,
					  const gchar  *nick,
					  const gchar  *blurb,
					  GParamFlags   flags);
