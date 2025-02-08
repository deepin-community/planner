resources = gnome.compile_resources(
  'app.drey.Planner',
  'app.drey.Planner.gresource.xml',
  c_name: 'resources')

if libeds_dep.found()
  eds_plugin_resources = gnome.compile_resources(
    'app.drey.Planner.plugin.eds',
    'app.drey.Planner.plugin.eds.gresource.xml',
    c_name: 'eds_plugin_resources')
endif

if gda_dep.found()
  sql_plugin_resources = gnome.compile_resources(
    'app.drey.Planner.plugin.sql',
    'app.drey.Planner.plugin.sql.gresource.xml',
    c_name: 'sql_plugin_resources')
  subdir('sql')
endif

subdir('images')
subdir('ui')
subdir('dtd')
subdir('stylesheets')
subdir('mime')
install_man('planner.1')
i18n.merge_file(
  input: 'app.drey.Planner.desktop.in',
  output: 'app.drey.Planner.desktop',
  type: 'desktop',
  po_dir: po_dir,
  install: true,
  install_dir: planner_datadir / 'applications',
)

schema_file = 'app.drey.Planner.gschema.xml'
install_data(
  schema_file,
  install_dir: planner_datadir / 'glib-2.0' / 'schemas',
)

install_data(
  'planner.convert',
  install_dir: planner_datadir / 'GConf' / 'gsettings',
)
