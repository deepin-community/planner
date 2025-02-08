libgnomecanvas_srcs = [
  'gailcanvas.c',
  'gailcanvasgroup.c',
  'gailcanvasgroupfactory.c',
  'gailcanvasitem.c',
  'gailcanvasitemfactory.c',
  'gailcanvastext.c',
  'gailcanvastextfactory.c',
  'gailcanvaswidget.c',
  'gailcanvaswidgetfactory.c',
  'gnome-canvas-line.c',
  'gnome-canvas-pixbuf.c',
  'gnome-canvas-rect.c',
  'gnome-canvas-text.c',
  'gnome-canvas-util.c',
  'gnome-canvas-widget.c',
  'gnome-canvas.c',
  'libgnomecanvastypes.c',
]

gnomecanvas_lib = static_library(
  'gnomecanvas',
  sources: libgnomecanvas_srcs,
  include_directories: [toplevel_inc, src_inc],
  dependencies: [gtk_dep, gail_dep],
  c_args: '-DG_LOG_DOMAIN="GnomeCanvas"',
)
