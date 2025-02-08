sql_data = [
  'database-0.13.sql',
  'database-0.11.sql',
  'database.sql',
  'upgrade-0.11-0.13.sql',
  'upgrade-0.6.x-0.11.sql',
]
install_data(sql_data,
  install_dir: planner_pkgdatadir / 'sql'
)

install_data(
  'app.drey.planner.plugins.sql.gschema.xml',
  install_dir: get_option('datadir') / 'glib-2.0' / 'schemas',
)
