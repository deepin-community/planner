doc_module = 'libplanner'
docpath = planner_datadir / 'gtk-doc' / 'html'
glib_prefix = glib_dep.get_pkgconfig_variable('prefix')
glib_docpath = glib_prefix / 'share' / 'gtk-doc' / 'html'

ignore_headers = [
  'mrp-file-module.h',
  'mrp-marshal.h',
  'mrp-old-xml.h',
  'mrp-parser.h',
  'mrp-paths.h',
  'mrp-private.h',
  'mrp-sql.h',
  'mrp-storage-module-factory.h',
  'mrp-storage-module.h',
  'mrp-storage-module-xml.h',
  'mrp-storage-mrproject.h',
  'mrp-storage-sql.h',
  'mrp-task-manager.h',
]

scan_args = [
  '--rebuild-sections',
  '--rebuild-types',
  '--ignore-headers=' + ' '.join(ignore_headers),
]

mkdb_args = [
  '--xml-mode',
  '--output-format=xml',
]

fixxref_args = [
  '--html-dir=@0@'.format(docpath),
  '--extra-dir=@0@'.format(glib_docpath / 'glib'),
  '--extra-dir=@0@'.format(glib_docpath / 'gobject'),
  '--extra-dir=@0@'.format(glib_docpath / 'gio'),
]

gnome.gtkdoc(
  doc_module,
  main_xml: doc_module + '-docs.xml',
  gobject_typesfile: doc_module + '.types',
  src_dir: libplanner_inc,
  ignore_headers: ignore_headers,
  dependencies: libplanner_dep,
  namespace: doc_module,
  scan_args: scan_args,
  mkdb_args: mkdb_args,
  fixxref_args: fixxref_args,
  install: true,
)
