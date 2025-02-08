#include <glib.h>
#include <locale.h>
#include "mrp-time.h"

static void
test_mrp_time_old_test() {
  mrptime t;

  /* setlocale (LC_ALL, ""); */

  t = mrp_time_compose (2002, 3, 31, 0, 0, 0);
  g_assert_cmpstr (mrp_time_to_string (t), ==, "20020331T000000Z");

  /* Test mrp_time_from_string */
  g_assert_cmpstr (mrp_time_to_string (mrp_time_from_string ("20020329")), ==, "20020329T000000Z");
  g_assert_cmpstr (mrp_time_to_string (mrp_time_from_string ("19991231")), ==, "19991231T000000Z");
  g_assert_cmpstr (mrp_time_to_string (mrp_time_from_string ("invalid")), ==, "19700101T000000Z");
}

static void
test_mrp_time_from_string() {
  int year, month, day;
  mrptime t;

  /* Test YYYYMMDD */
  t = mrp_time_from_string ("19700102");

  mrp_time_decompose (t, &year, &month, &day, NULL, NULL, NULL);

  g_assert_cmpint (1970, ==, year);
  g_assert_cmpint (1, ==, month);
  g_assert_cmpint (2, ==, day);

  /* Test YYYMMDDTHHMMSS */
  int hour, minute, sec;
  t = mrp_time_from_string ("20200203T101112");

  mrp_time_decompose (t, &year, &month, &day, &hour, &minute, &sec);

  g_assert_cmpint (2020, ==, year);
  g_assert_cmpint (2, ==, month);
  g_assert_cmpint (3, ==, day);
  g_assert_cmpint (10, ==, hour);
  g_assert_cmpint (11, ==, minute);
  g_assert_cmpint (12, ==, sec);

  /* Test YYYMMDDTHHMMSSZ */
  t = mrp_time_from_string ("20100304T202122Z");

  mrp_time_decompose (t, &year, &month, &day, &hour, &minute, &sec);

  g_assert_cmpint (2010, ==, year);
  g_assert_cmpint (3, ==, month);
  g_assert_cmpint (4, ==, day);
  g_assert_cmpint (20, ==, hour);
  g_assert_cmpint (21, ==, minute);
  g_assert_cmpint (22, ==, sec);
}

static void
test_mrp_time_from_string_fails() {
  mrptime t;

  /* End symbol is != Z */
  t = mrp_time_from_string ("20100304T202122Y");
  g_assert_cmpint (t, ==, 0);

  /* unknown characters */
  t = mrp_time_from_string ("202010i0");
  g_assert_cmpint (t, ==, 0);

  t = mrp_time_from_string ("20100304T202i22Z");
  g_assert_cmpint (t, ==, 0);

  /* wrong date and time separator */
  t = mrp_time_from_string ("20100304H202122Z");
  g_assert_cmpint (t, ==, 0);
}

static void
test_mrp_time_to_string() {
  mrptime  t;
  gchar   *str;

  t = mrp_time_compose (2021, 6, 23, 0, 0, 0);
  str = mrp_time_to_string (t);
  g_assert_cmpstr (str, ==, "20210623T000000Z");
  g_free (str);

  t = mrp_time_compose (2022, 7, 15, 13, 2, 9);
  str = mrp_time_to_string (t);
  g_assert_cmpstr (str, ==, "20220715T130209Z");
  g_free (str);
}

static void
test_mrp_time_align_previous() {
  mrptime t;
  int hour, min, sec, year, month, day;

  t = mrp_time_compose (2000, 10, 5, 22, 1, 15);

  /* align hour */
  t = mrp_time_align_prev (t, MRP_TIME_UNIT_HOUR);
  mrp_time_decompose (t, &year, &month, &day, &hour, &min, &sec);

  g_assert_cmpint (hour, ==, 22);
  g_assert_cmpint (min, ==, 0);
  g_assert_cmpint (sec, ==, 0);

  /* align two hours */
  t = mrp_time_align_prev (t, MRP_TIME_UNIT_TWO_HOURS);
  mrp_time_decompose (t, &year, &month, &day, &hour, &min, &sec);

  g_assert_cmpint (hour, ==, 22);
  g_assert_cmpint (min, ==, 0);
  g_assert_cmpint (sec, ==, 0);

  t = mrp_time_compose (2000, 10, 5, 21, 59, 59);
  t = mrp_time_align_prev (t, MRP_TIME_UNIT_TWO_HOURS);
  mrp_time_decompose (t, &year, &month, &day, &hour, &min, &sec);

  g_assert_cmpint (hour, ==, 20);
  g_assert_cmpint (min, ==, 0);
  g_assert_cmpint (sec, ==, 0);

  /* align half day */
  t = mrp_time_align_prev (t, MRP_TIME_UNIT_HALFDAY);
  mrp_time_decompose (t, &year, &month, &day, &hour, &min, &sec);

  g_assert_cmpint (hour, ==, 12);
  g_assert_cmpint (min, ==, 0);
  g_assert_cmpint (sec, ==, 0);

  /* align day */
  t = mrp_time_align_prev (t, MRP_TIME_UNIT_DAY);
  mrp_time_decompose (t, &year, &month, &day, &hour, &min, &sec);

  g_assert_cmpint (hour, ==, 0);
  g_assert_cmpint (min, ==, 0);
  g_assert_cmpint (sec, ==, 0);
  g_assert_cmpint (year, ==, 2000);
  g_assert_cmpint (month, ==, 10);
  g_assert_cmpint (day, ==, 5);

  /* align week - currently hardcoded to monday */
  t = mrp_time_align_prev (t, MRP_TIME_UNIT_WEEK);
  mrp_time_decompose (t, &year, &month, &day, &hour, &min, &sec);

  g_assert_cmpint (hour, ==, 0);
  g_assert_cmpint (min, ==, 0);
  g_assert_cmpint (sec, ==, 0);
  g_assert_cmpint (year, ==, 2000);
  g_assert_cmpint (month, ==, 10);
  g_assert_cmpint (day, ==, 2);

  /* align month */
  t = mrp_time_align_prev (t, MRP_TIME_UNIT_MONTH);
  mrp_time_decompose (t, &year, &month, &day, &hour, &min, &sec);

  g_assert_cmpint (hour, ==, 0);
  g_assert_cmpint (min, ==, 0);
  g_assert_cmpint (sec, ==, 0);
  g_assert_cmpint (year, ==, 2000);
  g_assert_cmpint (month, ==, 10);
  g_assert_cmpint (day, ==, 1);

  /* align quarter */
  t = mrp_time_compose (2000, 11, 5, 17, 5, 10);
  t = mrp_time_align_prev (t, MRP_TIME_UNIT_QUARTER);
  mrp_time_decompose (t, &year, &month, &day, &hour, &min, &sec);

  g_assert_cmpint (hour, ==, 0);
  g_assert_cmpint (min, ==, 0);
  g_assert_cmpint (sec, ==, 0);
  g_assert_cmpint (year, ==, 2000);
  g_assert_cmpint (month, ==, 10);
  g_assert_cmpint (day, ==, 1);

  /* align half year */
  t = mrp_time_align_prev (t, MRP_TIME_UNIT_HALFYEAR);
  mrp_time_decompose (t, &year, &month, &day, &hour, &min, &sec);

  g_assert_cmpint (hour, ==, 0);
  g_assert_cmpint (min, ==, 0);
  g_assert_cmpint (sec, ==, 0);
  g_assert_cmpint (year, ==, 2000);
  g_assert_cmpint (month, ==, 7);
  g_assert_cmpint (day, ==, 1);

  /* align year */
  t = mrp_time_align_prev (t, MRP_TIME_UNIT_YEAR);
  mrp_time_decompose (t, &year, &month, &day, &hour, &min, &sec);

  g_assert_cmpint (hour, ==, 0);
  g_assert_cmpint (min, ==, 0);
  g_assert_cmpint (sec, ==, 0);
  g_assert_cmpint (year, ==, 2000);
  g_assert_cmpint (month, ==, 1);
  g_assert_cmpint (day, ==, 1);

  /* assert */
  if (g_test_subprocess ()) {
    t = mrp_time_align_prev (t, MRP_TIME_UNIT_NONE);
    return;
  }

  g_test_trap_subprocess (NULL, 0, 0);
  g_test_trap_assert_failed ();
}

static void
test_mrp_time_align_next() {
  mrptime t;
  int hour, min, sec, year, month, day;

  t = mrp_time_compose (2000, 10, 5, 22, 1, 15);

  /* align hour */
  t = mrp_time_align_next (t, MRP_TIME_UNIT_HOUR);
  mrp_time_decompose (t, &year, &month, &day, &hour, &min, &sec);

  g_assert_cmpint (hour, ==, 23);
  g_assert_cmpint (min, ==, 0);
  g_assert_cmpint (sec, ==, 0);

  /* align two hours */
  t = mrp_time_align_next (t, MRP_TIME_UNIT_TWO_HOURS);
  mrp_time_decompose (t, &year, &month, &day, &hour, &min, &sec);

  g_assert_cmpint (hour, ==, 0);
  g_assert_cmpint (min, ==, 0);
  g_assert_cmpint (sec, ==, 0);

  /* align half day */
  t = mrp_time_align_next (t, MRP_TIME_UNIT_HALFDAY);
  mrp_time_decompose (t, &year, &month, &day, &hour, &min, &sec);

  g_assert_cmpint (hour, ==, 12);
  g_assert_cmpint (min, ==, 0);
  g_assert_cmpint (sec, ==, 0);
  g_assert_cmpint (year, ==, 2000);
  g_assert_cmpint (month, ==, 10);
  g_assert_cmpint (day, ==, 6);

  /* align day */
  t = mrp_time_align_next (t, MRP_TIME_UNIT_DAY);
  mrp_time_decompose (t, &year, &month, &day, &hour, &min, &sec);

  g_assert_cmpint (hour, ==, 0);
  g_assert_cmpint (min, ==, 0);
  g_assert_cmpint (sec, ==, 0);
  g_assert_cmpint (year, ==, 2000);
  g_assert_cmpint (month, ==, 10);
  g_assert_cmpint (day, ==, 7);

  /* align week - currently hardcoded to monday */
  t = mrp_time_align_next (t, MRP_TIME_UNIT_WEEK);
  mrp_time_decompose (t, &year, &month, &day, &hour, &min, &sec);

  g_assert_cmpint (hour, ==, 0);
  g_assert_cmpint (min, ==, 0);
  g_assert_cmpint (sec, ==, 0);
  g_assert_cmpint (year, ==, 2000);
  g_assert_cmpint (month, ==, 10);
  g_assert_cmpint (day, ==, 9);

  /* align month */
  t = mrp_time_align_next (t, MRP_TIME_UNIT_MONTH);
  mrp_time_decompose (t, &year, &month, &day, &hour, &min, &sec);

  g_assert_cmpint (hour, ==, 0);
  g_assert_cmpint (min, ==, 0);
  g_assert_cmpint (sec, ==, 0);
  g_assert_cmpint (year, ==, 2000);
  g_assert_cmpint (month, ==, 11);
  g_assert_cmpint (day, ==, 1);

  /* align quarter */
  t = mrp_time_compose (2000, 8, 5, 18, 58, 59);
  t = mrp_time_align_next (t, MRP_TIME_UNIT_QUARTER);
  mrp_time_decompose (t, &year, &month, &day, &hour, &min, &sec);

  g_assert_cmpint (hour, ==, 0);
  g_assert_cmpint (min, ==, 0);
  g_assert_cmpint (sec, ==, 0);
  g_assert_cmpint (year, ==, 2000);
  g_assert_cmpint (month, ==, 10);
  g_assert_cmpint (day, ==, 1);

  /* align half year */
  t = mrp_time_align_next (t, MRP_TIME_UNIT_HALFYEAR);
  mrp_time_decompose (t, &year, &month, &day, &hour, &min, &sec);

  g_assert_cmpint (hour, ==, 0);
  g_assert_cmpint (min, ==, 0);
  g_assert_cmpint (sec, ==, 0);
  g_assert_cmpint (year, ==, 2001);
  g_assert_cmpint (month, ==, 1);
  g_assert_cmpint (day, ==, 1);

  /* align year */
  t = mrp_time_align_next (t, MRP_TIME_UNIT_YEAR);
  mrp_time_decompose (t, &year, &month, &day, &hour, &min, &sec);

  g_assert_cmpint (hour, ==, 0);
  g_assert_cmpint (min, ==, 0);
  g_assert_cmpint (sec, ==, 0);
  g_assert_cmpint (year, ==, 2002);
  g_assert_cmpint (month, ==, 1);
  g_assert_cmpint (day, ==, 1);

  /* assert */
  if (g_test_subprocess ()) {
    t = mrp_time_align_next (t, MRP_TIME_UNIT_NONE);
    return;
  }

  g_test_trap_subprocess (NULL, 0, 0);
  g_test_trap_assert_failed ();
}

static void
test_mrp_time_compose_decompose (void)
{
  mrptime t;
  int year, month, day, hour, minute, sec;

  t = mrp_time_compose (2010, 5, 6, 16, 30, 1);

  g_assert_cmpint (t, ==, 1273163401);

  mrp_time_decompose (t, &year, &month, &day, &hour, &minute, &sec);

  g_assert_cmpint (year, ==, 2010);
  g_assert_cmpint (month, ==, 5);
  g_assert_cmpint (day, ==, 6);
  g_assert_cmpint (hour, ==, 16);
  g_assert_cmpint (minute, ==, 30);
  g_assert_cmpint (sec, ==, 1);
}

static void
test_mrp_time_format (void)
{
  mrptime t = mrp_time_compose (2010, 12, 15, 12, 0, 0);

  gchar *formatstr = mrp_time_format ("%Y-%m-%d %H:%M:%S", t);
  g_assert_cmpstr (formatstr, ==, "2010-12-15 12:00:00");
  g_free (formatstr);

  formatstr = mrp_time_format ("%b", t);
  g_assert_cmpstr (formatstr, ==, "Dec");
  g_free (formatstr);

  formatstr = mrp_time_format ("%B", t);
  g_assert_cmpstr (formatstr, ==, "December");
  g_free (formatstr);

  formatstr = mrp_time_format ("%e", t);
  g_assert_cmpstr (formatstr, ==, "15");
  g_free (formatstr);

  formatstr = mrp_time_format ("%R", t);
  g_assert_cmpstr (formatstr, ==, "12:00");
  g_free (formatstr);

  formatstr = mrp_time_format ("%a", t);
  g_assert_cmpstr (formatstr, ==, "Wed");
  g_free (formatstr);

  formatstr = mrp_time_format ("%A", t);
  g_assert_cmpstr (formatstr, ==, "Wednesday");
  g_free (formatstr);
}

gint
main (gint   argc,
      gchar *argv[])
{
  setlocale (LC_ALL, "C");
  g_test_init(&argc, &argv, NULL);

  g_test_add_func ("/libplanner/mrp-time/from_string", test_mrp_time_from_string);
  g_test_add_func ("/libplanner/mrp-time/from_string_fails", test_mrp_time_from_string_fails);
  g_test_add_func ("/libplanner/mrp-time/to_string", test_mrp_time_to_string);
  g_test_add_func ("/libplanner/mrp-time/align_previous", test_mrp_time_align_previous);
  g_test_add_func ("/libplanner/mrp-time/align_next", test_mrp_time_align_next);

  g_test_add_func ("/libplanner/mrp-time/compose_decompose", test_mrp_time_compose_decompose);
  g_test_add_func ("/libplanner/mrp-time/format", test_mrp_time_format);

  g_test_add_func ("/libplanner/old-time-test", test_mrp_time_old_test);

  return g_test_run ();
}
