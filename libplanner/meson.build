pkg = import('pkgconfig')

libplanner_srcs = [
  'mrp-application.c',
  'mrp-assignment.c',
  'mrp-calendar.c',
  'mrp-day.c',
  'mrp-error.c',
  'mrp-file-module.c',
  'mrp-group.c',
  'mrp-object.c',
  'mrp-paths-gnome.c',
  'mrp-project.c',
  'mrp-property.c',
  'mrp-relation.c',
  'mrp-resource.c',
  'mrp-storage-module-factory.c',
  'mrp-storage-module.c',
  'mrp-task-manager.c',
  'mrp-task.c',
  'mrp-time.c',
  'mrp-types.c',
]

libplanner_inc = include_directories('.')
mrp_marshal_srcs = gnome.genmarshal('mrp-marshal',
  sources: 'mrp-marshal.list',
  prefix: 'mrp_marshal'
)
libplanner_library = shared_library('planner-1',
  [libplanner_srcs, mrp_marshal_srcs],
  dependencies: [libplanner_deps],
  include_directories: [toplevel_inc, libplanner_inc],
  install: true,
  install_dir: planner_pkglibdir,
  soversion: '1',
  version: '1.0.0',
)

libplanner_dep = declare_dependency(
  link_with: libplanner_library,
  include_directories: [libplanner_inc],
  dependencies: [libplanner_deps],
)
libstorage_mrproject_srcs = [
  'mrp-storage-mrproject.c',
  'mrp-parser.c',
]
libstorage_mrproject_module = shared_module('storage-mrproject-1', [libstorage_mrproject_srcs],
  dependencies: [libplanner_dep],
  include_directories: [toplevel_inc],
  install: true,
  install_dir: planner_storagemoduledir,
)

if gda_dep.found()
  libstorage_sql_srcs = [ 'mrp-storage-sql.c', 'mrp-sql.c']
  libstorage_sql_module = shared_module('storage-sql', [libstorage_sql_srcs],
    dependencies: [libplanner_dep, gda_dep],
    include_directories: [toplevel_inc],
    install: true,
    install_dir: planner_storagemoduledir,
  )
endif

libmrp_xml_srcs = [
  'mrp-xml.c',
  'mrp-old-xml.c',
]
libmrp_xml_module = shared_module('mrp-xml', [libmrp_xml_srcs],
  dependencies: [libplanner_dep],
  include_directories: [toplevel_inc],
  install: true,
  install_dir: planner_filemoduledir,
)
libmrp_xsl_srcs = [
  'mrp-xsl.c',
]
libmrp_xsl_module = shared_module('mrp-xsl', [libmrp_xsl_srcs],
  dependencies: [libplanner_dep] + [xslt_dep, exslt_dep],
  include_directories: [toplevel_inc],
  install: true,
  install_dir: planner_filemoduledir,
)
