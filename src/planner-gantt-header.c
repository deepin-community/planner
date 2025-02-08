/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2001-2002 CodeFactory AB
 * Copyright (C) 2001-2002 Richard Hult <richard@imendio.com>
 * Copyright (C) 2001-2002 Mikael Hallendal <micke@imendio.com>
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
#include <string.h>
#include <time.h>
#include <math.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <libplanner/mrp-time.h>
#include "planner-marshal.h"
#include "planner-gantt-header.h"
#include "planner-gantt-model.h"
#include "planner-scale-utils.h"


struct _PlannerGanttHeaderPriv {
	GdkWindow          *bin_window;

	GtkAdjustment      *hadjustment;
	guint               hscroll_policy : 1;
	guint               vscroll_policy : 1;

	PangoLayout        *layout;

	MrpTimeUnit    major_unit;
	PlannerScaleFormat  major_format;

	MrpTimeUnit    minor_unit;
	PlannerScaleFormat  minor_format;

	gdouble             hscale;

	gint                width;
	gint                height;

	gdouble             x1;
	gdouble             x2;

	gchar              *date_hint;
};

/* Properties */
enum {
	PROP_0,
	PROP_HEIGHT,
	PROP_X1,
	PROP_X2,
	PROP_SCALE,
	PROP_ZOOM,
	/* Scrollable interface */
	PROP_HADJUSTMENT,
	PROP_VADJUSTMENT,
	PROP_HSCROLL_POLICY,
	PROP_VSCROLL_POLICY
};

enum {
	DATE_HINT_CHANGED,
	LAST_SIGNAL
};

static void     gantt_header_class_init          (PlannerGanttHeaderClass *klass);
static void     gantt_header_init                (PlannerGanttHeader      *header);
static void     gantt_header_finalize            (GObject                 *object);
static void     gantt_header_set_property        (GObject                 *object,
						  guint                    prop_id,
						  const GValue            *value,
						  GParamSpec              *pspec);
static void     gantt_header_get_property        (GObject                 *object,
						  guint                    prop_id,
						  GValue                  *value,
						  GParamSpec              *pspec);
static void     gantt_header_map                 (GtkWidget               *widget);
static void     gantt_header_realize             (GtkWidget               *widget);
static void     gantt_header_unrealize           (GtkWidget               *widget);
static void     gantt_header_size_allocate       (GtkWidget               *widget,
						  GtkAllocation           *allocation);
static gboolean gantt_header_draw                (GtkWidget               *widget,
						  cairo_t                 *cr);
static gboolean gantt_header_motion_notify_event (GtkWidget               *widget,
						  GdkEventMotion          *event);
static gboolean gantt_header_leave_notify_event  (GtkWidget	          *widget,
						  GdkEventCrossing        *event);
static void     gantt_header_set_hadjustment     (PlannerGanttHeader      *header,
						  GtkAdjustment           *hadj);
static void     gantt_header_adjustment_changed  (GtkAdjustment           *adjustment,
						  PlannerGanttHeader      *header);



static GtkWidgetClass *parent_class = NULL;
static guint           signals[LAST_SIGNAL];


GType
planner_gantt_header_get_type (void)
{
	static GType planner_gantt_header_type = 0;

	if (!planner_gantt_header_type) {
		static const GTypeInfo planner_gantt_header_info = {
			sizeof (PlannerGanttHeaderClass),
			NULL,		/* base_init */
			NULL,		/* base_finalize */
			(GClassInitFunc) gantt_header_class_init,
			NULL,		/* class_finalize */
			NULL,		/* class_data */
			sizeof (PlannerGanttHeader),
			0,              /* n_preallocs */
			(GInstanceInitFunc) gantt_header_init
		};

		static const GInterfaceInfo scrollable_info = {
			(GInterfaceInitFunc) NULL,
			NULL,
			NULL
		};

		planner_gantt_header_type = g_type_register_static (
			GTK_TYPE_WIDGET,
			"PlannerGanttHeader",
			&planner_gantt_header_info,
			0);

		g_type_add_interface_static (planner_gantt_header_type,
					     GTK_TYPE_SCROLLABLE,
					     &scrollable_info);
	}

	return planner_gantt_header_type;
}

static void
gantt_header_class_init (PlannerGanttHeaderClass *class)
{
	GObjectClass      *o_class;
	GtkWidgetClass    *widget_class;

	parent_class = g_type_class_peek_parent (class);

	o_class = (GObjectClass *) class;
	widget_class = (GtkWidgetClass *) class;

	/* GObject methods. */
	o_class->set_property = gantt_header_set_property;
	o_class->get_property = gantt_header_get_property;
	o_class->finalize = gantt_header_finalize;

	/* GtkWidget methods. */
	widget_class->map = gantt_header_map;;
	widget_class->realize = gantt_header_realize;
	widget_class->unrealize = gantt_header_unrealize;
	widget_class->size_allocate = gantt_header_size_allocate;
	widget_class->draw = gantt_header_draw;
	widget_class->leave_notify_event = gantt_header_leave_notify_event;

	widget_class->motion_notify_event = gantt_header_motion_notify_event;

	/* GtkScrollable interface properties */
	g_object_class_override_property (o_class, PROP_HADJUSTMENT,    "hadjustment");
	g_object_class_override_property (o_class, PROP_VADJUSTMENT,    "vadjustment");
	g_object_class_override_property (o_class, PROP_HSCROLL_POLICY, "hscroll-policy");
	g_object_class_override_property (o_class, PROP_VSCROLL_POLICY, "vscroll-policy");

	/* Properties. */
	g_object_class_install_property (
		o_class,
		PROP_HEIGHT,
		g_param_spec_int ("height",
				  NULL,
				  NULL,
				  0, G_MAXINT, 0,
				  G_PARAM_READWRITE));

	g_object_class_install_property (
		o_class,
		PROP_X1,
		g_param_spec_double ("x1",
				     NULL,
				     NULL,
				     -1, G_MAXDOUBLE, -1,
				     G_PARAM_READWRITE));

	g_object_class_install_property (
		o_class,
		PROP_X2,
		g_param_spec_double ("x2",
				     NULL,
				     NULL,
				     -1, G_MAXDOUBLE, -1,
				     G_PARAM_READWRITE));

	g_object_class_install_property (
		o_class,
		PROP_SCALE,
		g_param_spec_double ("scale",
				     NULL,
				     NULL,
				     0.000001, G_MAXDOUBLE, 1.0,
				     G_PARAM_WRITABLE));

	g_object_class_install_property (
		o_class,
		PROP_ZOOM,
		g_param_spec_double ("zoom",
				     NULL,
				     NULL,
				     -G_MAXDOUBLE, G_MAXDOUBLE, 7,
				     G_PARAM_WRITABLE));

	signals[DATE_HINT_CHANGED] =
		g_signal_new ("date-hint-changed",
			      G_TYPE_FROM_CLASS (class),
			      G_SIGNAL_RUN_LAST,
			      0,
			      NULL, NULL,
			      planner_marshal_VOID__STRING,
			      G_TYPE_NONE, 1,
			      G_TYPE_STRING);
}

static void
gantt_header_init (PlannerGanttHeader *header)
{
	PlannerGanttHeaderPriv *priv;

	gtk_widget_set_redraw_on_allocate (GTK_WIDGET (header), FALSE);

	priv = g_new0 (PlannerGanttHeaderPriv, 1);
	header->priv = priv;

	priv->hscale = 1.0;
	priv->x1 = 0;
	priv->x2 = 0;
	priv->height = -1;
	priv->width = -1;

	priv->major_unit = MRP_TIME_UNIT_MONTH;
	priv->minor_unit = MRP_TIME_UNIT_WEEK;

	priv->layout = gtk_widget_create_pango_layout (GTK_WIDGET (header),
						       NULL);
}

static void
gantt_header_set_zoom (PlannerGanttHeader *header, gdouble zoom)
{
	PlannerGanttHeaderPriv *priv;
	gint                   level;

	priv = header->priv;

	level = planner_scale_clamp_zoom (zoom);

	priv->major_unit = planner_scale_conf[level].major_unit;
	priv->major_format = planner_scale_conf[level].major_format;

	priv->minor_unit = planner_scale_conf[level].minor_unit;
	priv->minor_format = planner_scale_conf[level].minor_format;
}

static void
gantt_header_set_property (GObject      *object,
			   guint         prop_id,
			   const GValue *value,
			   GParamSpec   *pspec)
{
	PlannerGanttHeader     *header;
	PlannerGanttHeaderPriv *priv;
	gdouble                 tmp;
	gint                    width;
	gdouble                 tmp_scale;
	gboolean                change_width = FALSE;
	gboolean                change_height = FALSE;
	gboolean                change_scale = FALSE;

	header = PLANNER_GANTT_HEADER (object);
	priv = header->priv;

	switch (prop_id) {
	case PROP_HEIGHT:
		priv->height = g_value_get_int (value);
		change_height = TRUE;
		break;
	case PROP_X1:
		tmp = g_value_get_double (value);
		if (tmp != priv->x1) {
			priv->x1 = tmp;
			change_width = TRUE;
		}
		break;
	case PROP_X2:
		tmp = g_value_get_double (value);
		if (tmp != priv->x2) {
			priv->x2 = tmp;
			change_width = TRUE;
		}
		break;
	case PROP_SCALE:
		tmp_scale = g_value_get_double (value);
		if (tmp_scale != priv->hscale) {
			priv->hscale = tmp_scale;
			change_scale = TRUE;
		}
		break;
	case PROP_ZOOM:
		gantt_header_set_zoom (header, g_value_get_double (value));
		break;
	case PROP_HADJUSTMENT:
		gantt_header_set_hadjustment (header, g_value_get_object (value));
		break;
	case PROP_VADJUSTMENT:
		/* TODO: g_assert_not_reached (); */
		break;
	case PROP_HSCROLL_POLICY:
		priv->hscroll_policy = g_value_get_enum (value);
		gtk_widget_queue_resize (GTK_WIDGET (header));
		break;
	case PROP_VSCROLL_POLICY:
		priv->hscroll_policy = g_value_get_enum (value);
		gtk_widget_queue_resize (GTK_WIDGET (header));
		break;
	default:
		break;
	}

	if (change_width) {
		if (priv->x1 > 0 && priv->x2 > 0) {
			width = floor (priv->x2 - priv->x1 + 0.5);

			/* If both widths aren't set yet, this can happen: */
			if (width < -1) {
				width = -1;
			}
		} else {
			width = -1;
		}
		priv->width = width;
	}

	if (change_width || change_height) {
		gtk_widget_set_size_request (GTK_WIDGET (header),
					     priv->width,
					     priv->height);
	}

	if ((change_width || change_height || change_scale) && gtk_widget_get_realized (GTK_WIDGET (header))) {
		gdk_window_invalidate_rect (priv->bin_window,
					    NULL,
					    FALSE);
	}
}

static void
gantt_header_get_property (GObject    *object,
			   guint       prop_id,
			   GValue     *value,
			   GParamSpec *pspec)
{
	PlannerGanttHeader     *header;

	header = PLANNER_GANTT_HEADER (object);

	switch (prop_id) {
	case PROP_HADJUSTMENT:
		g_value_set_object (value, header->priv->hadjustment);
		break;
	case PROP_VADJUSTMENT:
		/* TODO: g_assert_not_reached (); */
		break;
	case PROP_HSCROLL_POLICY:
		g_value_set_enum (value, header->priv->hscroll_policy);
		break;
	case PROP_VSCROLL_POLICY:
		g_value_set_enum (value, header->priv->vscroll_policy);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gantt_header_finalize (GObject *object)
{
	PlannerGanttHeader *header = PLANNER_GANTT_HEADER (object);

	g_object_unref (header->priv->layout);

	g_free (header->priv->date_hint);

	g_free (header->priv);

	if (G_OBJECT_CLASS (parent_class)->finalize) {
		(* G_OBJECT_CLASS (parent_class)->finalize) (object);
	}
}

static void
gantt_header_map (GtkWidget *widget)
{
	PlannerGanttHeader *header;

	header = PLANNER_GANTT_HEADER (widget);

	gtk_widget_set_mapped (widget, TRUE);

	gdk_window_show (header->priv->bin_window);
	gdk_window_show (gtk_widget_get_window (widget));
}

static void
gantt_header_realize (GtkWidget *widget)
{
	PlannerGanttHeader *header;
	GtkAllocation       allocation;
	GdkWindowAttr       attributes;
	gint                attributes_mask;
	GdkWindow          *window;

	header = PLANNER_GANTT_HEADER (widget);

	gtk_widget_set_realized (widget, TRUE);
	gtk_widget_get_allocation (widget, &allocation);

	/* Create the main, clipping window. */
	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.x = allocation.x;
	attributes.y = allocation.y;
	attributes.width = allocation.width;
	attributes.height = allocation.height;
	attributes.wclass = GDK_INPUT_OUTPUT;
	attributes.visual = gtk_widget_get_visual (widget);
	attributes.event_mask = GDK_VISIBILITY_NOTIFY_MASK;

	attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL;

	window = gdk_window_new (gtk_widget_get_parent_window (widget),
				 &attributes, attributes_mask);
	gtk_widget_set_window (widget, window);
	gdk_window_set_user_data (window, widget);

	/* Bin window. */
	attributes.x = 0;
	attributes.y = header->priv->height;
	attributes.width = header->priv->width;
	gtk_widget_get_allocation (widget, &allocation);
	attributes.height = allocation.height;
	attributes.event_mask = GDK_EXPOSURE_MASK | /* TODO: Do we need exposure mask anymore? */
		GDK_SCROLL_MASK |
		GDK_POINTER_MOTION_MASK |
		GDK_ENTER_NOTIFY_MASK |
		GDK_LEAVE_NOTIFY_MASK |
		GDK_BUTTON_PRESS_MASK |
		GDK_BUTTON_RELEASE_MASK |
		gtk_widget_get_events (widget);

	header->priv->bin_window = gdk_window_new (gtk_widget_get_window (widget),
						   &attributes,
						   attributes_mask);
	gdk_window_set_user_data (header->priv->bin_window, widget);
}

static void
gantt_header_unrealize (GtkWidget *widget)
{
	PlannerGanttHeader *header;

	header = PLANNER_GANTT_HEADER (widget);

	gdk_window_set_user_data (header->priv->bin_window, NULL);
	gdk_window_destroy (header->priv->bin_window);
	header->priv->bin_window = NULL;

	if (GTK_WIDGET_CLASS (parent_class)->unrealize) {
		(* GTK_WIDGET_CLASS (parent_class)->unrealize) (widget);
	}
}

static void
gantt_header_size_allocate (GtkWidget     *widget,
			    GtkAllocation *allocation)
{
	PlannerGanttHeader *header;

	header = PLANNER_GANTT_HEADER (widget);

	gtk_widget_set_allocation (widget, allocation);

	if (gtk_widget_get_realized (widget)) {
		gdk_window_move_resize (gtk_widget_get_window (widget),
					allocation->x, allocation->y,
					allocation->width, allocation->height);
		gdk_window_move_resize (header->priv->bin_window,
					- (gint) gtk_adjustment_get_value (header->priv->hadjustment),
					0,
					MAX (header->priv->width, allocation->width),
					allocation->height);
	}
}

static gboolean
gantt_header_draw (GtkWidget *widget,
                   cairo_t   *cr)
{
	PlannerGanttHeader     *header;
	PlannerGanttHeaderPriv *priv;
	GdkWindow              *bin_window;
	GdkRectangle            clip;
	gint                    height;
	gdouble                 hscale;
	gint                    x, tr_x, tr_y;
	mrptime                 t0;
	mrptime                 t1;
	mrptime                 t;
	gchar                  *str;
	gint                    minor_width;
	gint                    major_width;
	GdkRGBA text_color = { 0.0, 0.0, 0.0, 1.0 }; /* Fallback to black */
	GdkRGBA insensitive_fg_color = { 0.980392, 0.976471, 0.972549, 1.000000 }; /* Fallback to #faf9f8 */

	header = PLANNER_GANTT_HEADER (widget);
	priv = header->priv;
	hscale = priv->hscale;

	bin_window = priv->bin_window;
	if (!gtk_cairo_should_draw_window (cr, bin_window)) {
		return FALSE;
	}
	gdk_window_get_position (bin_window, &tr_x, &tr_y);
	cairo_translate (cr, tr_x, tr_y);

	gdk_cairo_get_clip_rectangle (cr, &clip);

	cairo_set_line_width (cr, 1.0);
	cairo_set_line_cap (cr, CAIRO_LINE_CAP_SQUARE);

	gtk_style_context_lookup_color (gtk_widget_get_style_context (widget),
					"theme_text_color", &text_color);
	/* TODO: Use same theming methods as GtkTreeView column headers do (they end up different shade of grey) */
	gtk_style_context_lookup_color (gtk_widget_get_style_context (widget),
					"insensitive_fg_color", &insensitive_fg_color);

	t0 = floor ((priv->x1 + clip.x) / hscale + 0.5);
	t1 = floor ((priv->x1 + clip.x + clip.width) / hscale + 0.5);

	height = gdk_window_get_height (bin_window);

	/* Draw background. We only draw over the exposed area, padding with +/-
	 * 5 so we don't mess up the header with button edges all over.
	 */
	/* TODO: Just leaving out this step seems to render OK result too - investigate */
	gtk_render_background (gtk_widget_get_style_context (widget),
			       cr,
			       clip.x - 5,
			       0,
			       clip.width + 10,
			       height);

	// Horizontal full width line between week number and date rows
	gdk_cairo_set_source_rgba (cr, &insensitive_fg_color);
	cairo_move_to (cr, clip.x + 0.5, height / 2 + 0.5);
	cairo_line_to (cr, clip.x + clip.width + 0.5, height / 2 + 0.5);
	cairo_stroke (cr);

	/* Get the widths of major/minor ticks so that we know how wide to make
	 * the clip region.
	 */
	major_width = hscale * (mrp_time_align_next (t0, priv->major_unit) -
				mrp_time_align_prev (t0, priv->major_unit));

	minor_width = hscale * (mrp_time_align_next (t0, priv->minor_unit) -
				mrp_time_align_prev (t0, priv->minor_unit));

	/* Draw the major scale. */
	if (major_width < 2 || priv->major_unit == MRP_TIME_UNIT_NONE) {
		/* Unless it's too thin to make sense. */
		goto minor_ticks;
	}

	t = mrp_time_align_prev (t0, priv->major_unit);

	while (t <= t1) {
		x = floor (t * hscale - priv->x1 + 0.5);

		// Vertical lines between different weeks
		gdk_cairo_set_source_rgba (cr, &insensitive_fg_color);
		cairo_move_to (cr, x + 0.5, 0.5);
		cairo_line_to (cr, x + 0.5, height / 2 + 1);
		cairo_stroke (cr);

		str = planner_scale_format_time (t,
					    priv->major_unit,
					    priv->major_format);
		pango_layout_set_text (priv->layout,
				       str,
				       -1);
		g_free (str);

		cairo_save (cr);
		cairo_rectangle (cr, x, 0, major_width, height);
		cairo_clip (cr);

		gdk_cairo_set_source_rgba (cr, &text_color);
		cairo_move_to (cr, x + 3, 2);
		pango_cairo_show_layout (cr, priv->layout);
		cairo_restore (cr);

		t = mrp_time_align_next (t, priv->major_unit);
	}

 minor_ticks:

	/* Draw the minor scale. */
	if (minor_width < 2 || priv->major_unit == MRP_TIME_UNIT_NONE) {
		/* Unless it's too thin to make sense. */
		goto done;
	}

	t = mrp_time_align_prev (t0, priv->minor_unit);

	while (t <= t1) {
		x = floor (t * hscale - priv->x1 + 0.5);

		// NOTE: Vertical lines between dates
		gdk_cairo_set_source_rgba (cr, &insensitive_fg_color);
		cairo_move_to (cr, x + 0.5, height / 2 + 0.5);
		cairo_line_to (cr, x + 0.5, height + 0.5);
		cairo_stroke (cr);

		str = planner_scale_format_time (t,
					    priv->minor_unit,
					    priv->minor_format);
		pango_layout_set_text (priv->layout,
				       str,
				       -1);
		g_free (str);

		cairo_save (cr);
		cairo_rectangle (cr, x, 0, minor_width, height);
		cairo_clip (cr);
		gdk_cairo_set_source_rgba (cr, &text_color);
		cairo_move_to (cr, x + 3, height / 2 + 2);
		pango_cairo_show_layout (cr, priv->layout);
		cairo_restore (cr);

		t = mrp_time_align_next (t, priv->minor_unit);
	}

 done:
	return TRUE;
}

static gboolean
gantt_header_motion_notify_event (GtkWidget	 *widget,
				  GdkEventMotion *event)
{
	PlannerGanttHeader     *header;
	PlannerGanttHeaderPriv *priv;
	mrptime                 t;
	char                   *str;

	header = PLANNER_GANTT_HEADER (widget);
	priv = header->priv;

	t = floor ((priv->x1 + event->x) / priv->hscale + 0.5);
	str = mrp_time_format (_("%a, %e %b %Y"), t);

	if (!priv->date_hint || strcmp (str, priv->date_hint) != 0) {
		g_signal_emit (widget, signals[DATE_HINT_CHANGED], 0, str);

		g_free (priv->date_hint);
		priv->date_hint = str;
	} else {
		g_free (str);
	}

	return FALSE;
}

static gboolean
gantt_header_leave_notify_event (GtkWidget	  *widget,
				 GdkEventCrossing *event)
{
	PlannerGanttHeader     *header;
	PlannerGanttHeaderPriv *priv;

	header = PLANNER_GANTT_HEADER (widget);
	priv = header->priv;

	if (priv->date_hint) {
		g_signal_emit (widget, signals[DATE_HINT_CHANGED], 0, NULL);

		g_free (priv->date_hint);
		priv->date_hint = NULL;
	}

	return FALSE;
}

/* Callbacks */
static void
gantt_header_set_hadjustment (PlannerGanttHeader *header,
			      GtkAdjustment      *hadj)
{
	if (hadj && header->priv->hadjustment == hadj)
		return;

	if (hadj)
		g_return_if_fail (GTK_IS_ADJUSTMENT (hadj));
	else
		hadj = GTK_ADJUSTMENT (gtk_adjustment_new (0.0, 0.0, 0.0, 0.0, 0.0, 0.0));

	if (header->priv->hadjustment != NULL) {
		g_signal_handlers_disconnect_matched (header->priv->hadjustment,
						      G_SIGNAL_MATCH_DATA,
						      0, 0, NULL, NULL,
						      header);
		g_object_unref (header->priv->hadjustment);
	}

	g_signal_connect (hadj,
			  "value-changed",
			  G_CALLBACK (gantt_header_adjustment_changed),
			  header);
	header->priv->hadjustment = g_object_ref_sink (hadj);
	/* TODO: Do we need to set the initial hadj values? */

	g_object_notify (G_OBJECT (header), "hadjustment");
}

static void
gantt_header_adjustment_changed (GtkAdjustment *adjustment,
				 PlannerGanttHeader *header)
{
	if (gtk_widget_get_realized (GTK_WIDGET (header))) {
		gdk_window_move (header->priv->bin_window,
				 - gtk_adjustment_get_value (header->priv->hadjustment),
				 0);
	}
}

GtkWidget *
planner_gantt_header_new (void)
{
	return g_object_new (PLANNER_TYPE_GANTT_HEADER, NULL);
}

