/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2022 Mart Raudsepp <mart@leio.tech>
 * Copyright (C) 2005 Imendio AB
 * Copyright (C) 2002-2003 CodeFactory AB
 * Copyright (C) 2002-2003 Richard Hult <richard@imendio.com>
 * Copyright (C) 2002 Mikael Hallendal <micke@imendio.com>
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

/**
 * SECTION:mrp-time
 * @Short_Description: represents date and time.
 * @Title: MrpTime
 * @include: libplanner/mrp-time.h
 *
 * ISO 8601 representation is done in the basic format.
 * Less separators are used:
 * <literal>20150101T000000Z</literal> for <literal>2015-01-01T00:00:00Z</literal>.
 *
 */

#include <config.h>
#include <glib.h>
#include <string.h>
#include "mrp-time.h"


/**
 * mrp_time_compose:
 * @year: the year
 * @month: the month
 * @day: the day
 * @hour: the hour
 * @minute: the minute
 * @second: the second
 *
 * Composes an #mrptime value from the separate components.
 *
 * Return value: an #mrptime value.
 **/
mrptime
mrp_time_compose (gint year,
		  gint month,
		  gint day,
		  gint hour,
		  gint minute,
		  gint second)
{
	GDateTime *datetime;
	mrptime    t;

	datetime = g_date_time_new_utc (year, month, day, hour, minute, second);
	g_return_val_if_fail (datetime, MRP_TIME_INVALID);

	t = g_date_time_to_unix (datetime);
	g_date_time_unref (datetime);

	return t;
}

/**
 * mrp_time_decompose:
 * @t: an #mrptime value to decompose
 * @year: location to store year, or %NULL
 * @month: location to store month, or %NULL
 * @day: location to store day, or %NULL
 * @hour: location to store hour, or %NULL
 * @minute: location to store minute, or %NULL
 * @second: location to store second, or %NULL
 *
 * Splits up an #mrptime value into its components.
 *
 * Return value: %TRUE on success.
 **/
gboolean
mrp_time_decompose (mrptime  t,
		    gint    *year,
		    gint    *month,
		    gint    *day,
		    gint    *hour,
		    gint    *minute,
		    gint    *second)
{
	GDateTime *datetime;

	datetime = g_date_time_new_from_unix_utc (t);
	g_return_val_if_fail (datetime, FALSE);

	g_date_time_get_ymd (datetime, year, month, day);

	if (hour) {
		*hour = g_date_time_get_hour (datetime);
	}
	if (minute) {
		*minute = g_date_time_get_minute (datetime);
	}
	if (second) {
		*second = g_date_time_get_second (datetime);
	}

	return TRUE;
}

/**
 * mrp_time_debug_print:
 * @t: an #mrptime
 *
 * Prints the time on stdout, for debugging purposes.
 **/
void
mrp_time_debug_print (mrptime t)
{
	GDateTime *datetime = g_date_time_new_from_unix_utc (t);
	if (datetime == NULL) {
		g_print ("(invalid time)\n");
		return;
	}

	gchar *str = g_date_time_format (datetime, "%F %T");

	g_print ("%s\n", str);

	g_free (str);
	g_date_time_unref (datetime);
}

/**
 * mrp_time_current_time:
 *
 * Retrieves the current time as an #mrptime value.
 *
 * Return value: Current time.
 **/
mrptime
mrp_time_current_time (void)
{
	GDateTime *datetime;

	datetime = g_date_time_new_now_local ();
	/* This is a hack. Set the timezone to UTC temporarily. */
	/* TODO: The current code isn't timezone-aware, so when retrieving the
	 * current time, we re-interpret it as it were UTC, to get the current
	 * time with timezone ignored, so it shows up as the current date of
	 * the user with everything else being UTC (but really timezone-unaware)
	 * as well. Investigate this situation further. */
	gint64 offset = g_date_time_get_utc_offset (datetime) / 1000 / 1000;
	mrptime t = g_date_time_to_unix (datetime) + offset;

	g_date_time_unref (datetime);
	return t;
}

/**
 * mrp_time_from_string:
 * @str: a string with a time, ISO8601 format
 * @err: Location to store error, or %NULL
 *
 * Parses an ISO8601 time string and converts it to an #mrptime.
 *
 * Return value: Converted time value.
 **/
mrptime
mrp_time_from_string (const gchar  *str)
{
	GDateTime *datetime;
	GTimeZone *tz_utc;
	gchar     *full_str;
	mrptime    t;

	tz_utc = g_time_zone_new_utc ();

	if (strlen (str) <= 10) {
		full_str = g_strdup_printf ("%sT000000Z", str);
	} else {
		full_str = g_strdup (str);
	}

	datetime = g_date_time_new_from_iso8601 (full_str, tz_utc);
	if (datetime == NULL)
		return MRP_TIME_INVALID;

	t = g_date_time_to_unix (datetime);

	g_free (full_str);
	g_date_time_unref (datetime);
	g_time_zone_unref (tz_utc);

	return t;
}

/**
 * mrp_time_to_string:
 * @t: an #mrptime time
 *
 * Converts a time value to an ISO8601 string.
 *
 * Return value: Allocated string that needs to be freed.
 **/
gchar *
mrp_time_to_string (mrptime t)
{
	GDateTime *datetime;
	gchar *res;

	datetime = g_date_time_new_from_unix_utc (t);
	g_return_val_if_fail (datetime, NULL);

	res = g_date_time_format (datetime, "%Y%m%dT%H%M%SZ");
	g_date_time_unref (datetime);

	return res;
}

/**
 * mrp_time_align_day:
 * @t: an #mrptime value
 *
 * Aligns a time value to the start of the day.
 *
 * Return value: Aligned value.
 **/
mrptime
mrp_time_align_day (mrptime t)
{
	return mrp_time_align_prev (t, MRP_TIME_UNIT_DAY);
}

/**
 * mrp_time_align_prev:
 * @t: an #mrptime value
 * @unit: an #MrpTimeUnit
 * @Returns: Aligned value.
 *
 * Align @t to the previous @unit.
 */
mrptime
mrp_time_align_prev (mrptime t, MrpTimeUnit unit)
{
	GDateTime *datetime, *orig;
	gint year, month, day, hour;
	mrptime res;

	orig = g_date_time_new_from_unix_utc (t);
	g_return_val_if_fail (orig, MRP_TIME_INVALID);

	g_date_time_get_ymd (orig, &year, &month, &day);
	hour = g_date_time_get_hour (orig);

	switch (unit) {
	case MRP_TIME_UNIT_HOUR:
		datetime = g_date_time_new_utc (year, month, day, hour, 0, 0);
		break;

	case MRP_TIME_UNIT_TWO_HOURS:
		datetime = g_date_time_new_utc (year, month, day,
						hour - (hour % 2), 0, 0);
		break;

	case MRP_TIME_UNIT_HALFDAY:
		datetime = g_date_time_new_utc (year, month, day,
						hour - (hour % 12), 0, 0);
		break;

	case MRP_TIME_UNIT_DAY:
		datetime = g_date_time_new_utc (year, month, day, 0, 0, 0);
		break;

	case MRP_TIME_UNIT_WEEK: {
		/* FIXME: We currently hardcode monday as week start */
		GDateTime *tmp;
		gint year, month, day;
		int days_to_add;

		days_to_add = -(g_date_time_get_day_of_week (orig) - 1);
		tmp = g_date_time_add_days (orig, days_to_add);
		g_date_time_get_ymd (tmp, &year, &month, &day);
		datetime = g_date_time_new_utc (year, month, day, 0, 0, 0);

		g_date_time_unref (tmp);
		break;
	}

	case MRP_TIME_UNIT_MONTH:
		datetime = g_date_time_new_utc (year, month, 1, 0, 0, 0);
		break;

	case MRP_TIME_UNIT_QUARTER:
		datetime = g_date_time_new_utc (year,
						month - ((month - 1) % 3),
						1,
						0, 0, 0);
		break;

	case MRP_TIME_UNIT_HALFYEAR:
		datetime = g_date_time_new_utc (year,
						month - ((month - 1) % 6),
						1,
						0, 0, 0);
		break;

	case MRP_TIME_UNIT_YEAR:
		datetime = g_date_time_new_utc (year, 1, 1,
						0, 0, 0);
		break;

	case MRP_TIME_UNIT_NONE:
		g_assert_not_reached ();
	}

	g_return_val_if_fail (datetime, MRP_TIME_INVALID);

	res = g_date_time_to_unix (datetime);

	g_date_time_unref (datetime);
	g_date_time_unref (orig);

	return res;
}

/**
 * mrp_time_align_next:
 * @t: an #mrptime value
 * @unit: an #MrpTimeUnit
 * @Returns: Aligned value.
 *
 * Align @t to the next @unit.
 */
mrptime
mrp_time_align_next (mrptime t, MrpTimeUnit unit)
{
	GDateTime *datetime, *prev;
	mrptime res;

	prev = g_date_time_new_from_unix_utc (mrp_time_align_prev (t, unit));

	switch (unit) {
	case MRP_TIME_UNIT_HOUR:
		datetime = g_date_time_add_hours (prev, 1);
		break;

	case MRP_TIME_UNIT_TWO_HOURS:
		datetime = g_date_time_add_hours (prev, 2);
		break;

	case MRP_TIME_UNIT_HALFDAY:
		datetime = g_date_time_add_hours (prev, 12);
		break;

	case MRP_TIME_UNIT_DAY:
		datetime = g_date_time_add_days (prev, 1);
		break;

	case MRP_TIME_UNIT_WEEK:
		datetime = g_date_time_add_days (prev, 7);
		break;

	case MRP_TIME_UNIT_MONTH:
		datetime = g_date_time_add_months (prev, 1);
		break;

	case MRP_TIME_UNIT_QUARTER:
		datetime = g_date_time_add_months (prev, 3);
		break;

	case MRP_TIME_UNIT_HALFYEAR:
		datetime = g_date_time_add_months (prev, 6);
		break;

	case MRP_TIME_UNIT_YEAR:
		datetime = g_date_time_add_years (prev, 1);
		break;

	case MRP_TIME_UNIT_NONE:
	default:
		g_assert_not_reached ();
	}

	g_return_val_if_fail (datetime, MRP_TIME_INVALID);

	res = g_date_time_to_unix (datetime);

	g_date_time_unref (datetime);
	g_date_time_unref (prev);

	return res;
}

/**
 * mrp_time_day_of_week:
 * @t: an #mrptime value
 *
 * Retrieves the day of week of the specified time.
 *
 * Return value: The day of week, in the range 0 to 6, where Sunday is 0.
 **/
gint
mrp_time_day_of_week (mrptime t)
{
	GDateTime *datetime;
	gint day_of_week;

	datetime = g_date_time_new_from_unix_utc (t);
	g_return_val_if_fail (datetime, MRP_TIME_INVALID);

	day_of_week = g_date_time_get_day_of_week (datetime) % 7;
	g_date_time_unref (datetime);

	return day_of_week;
}

/**
 * mrp_param_spec_time:
 * @name: name of the property
 * @nick: nick for the propery
 * @blurb: blurb for the property
 * @flags: flags
 *
 * Convenience function for creating a #GParamSpec carrying an #mrptime value.
 *
 * Return value: Newly created #GParamSpec.
 **/
GParamSpec *
mrp_param_spec_time (const gchar *name,
		     const gchar *nick,
		     const gchar *blurb,
		     GParamFlags flags)
{
	return g_param_spec_int64 (name,
				   nick,
				   blurb,
				   MRP_TIME_MIN, MRP_TIME_MAX, MRP_TIME_MIN,
				   flags);
}

/**
 * mrp_time_format:
 * @format: format string
 * @t: an #mrptime value
 *
 * Formats a string with time values.
 *
 * The allowed format codes are the same as for GDateTime.
 *
 * Return value: Newly created string that needs to be freed.
 **/
gchar *
mrp_time_format (const gchar *format, mrptime t)
{
	GDateTime *datetime;
	gchar   *buffer;

	datetime = g_date_time_new_from_unix_utc (t);
	g_return_val_if_fail (datetime, NULL);

	buffer = g_date_time_format (datetime, format);
	g_date_time_unref (datetime);

	return buffer;
}

/**
 * mrp_time_format_locale:
 * @t: an #mrptime value
 *
 * Formats a string with time values. For format is the preferred for the
 * current locale.
 *
 * Return value: Newly created string that needs to be freed.
 **/
gchar *
mrp_time_format_locale (mrptime t)
{
	return mrp_time_format ("%x", t);
}
