mime_xml_file = 'app.drey.Planner.xml'

planner_mimedir = planner_datadir / 'mime' / 'packages'

i18n.merge_file(
  input: mime_xml_file + '.in',
  output: mime_xml_file,
  po_dir: po_dir,
  data_dirs: its_dir,
  install: true,
  install_dir: planner_mimedir,
)
