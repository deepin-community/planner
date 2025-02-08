src_inc = include_directories('.')
subdir('libgnomecanvas')

# UI files of widget implementations go into this resource, while legacy pending
# refactoring ones are in the data/ui/ one until modernized
ui_resources = gnome.compile_resources(
  'ui-resources',
  'ui.gresource.xml',
  c_name: 'ui',
)

libplanner_app_srcs = [
  'eel-canvas-rect.c',
  'dummy-canvas-item.c',
  'planner-application.c',
  'planner-assignment-model.c',
  'planner-conf-gconf.c',
  'planner-calendar-dialog.c',
  'planner-calendar-popover.c',
  'planner-calendar-selector.c',
  'planner-canvas-line.c',
  'planner-cell-renderer-calendar.c',
  'planner-cmd-manager.c',
  'planner-column-dialog.c',
  'planner-day-type-dialog.c',
  'planner-default-week-dialog.c',
  'planner-format.c',
  'planner-group-dialog.c',
  'planner-group-model.c',
  'planner-list-model.c',
  'planner-phase-dialog.c',
  'planner-plugin.c',
  'planner-plugin-loader.c',
  'planner-predecessor-model.c',
  'planner-print-dialog.c',
  'planner-print-job.c',
  'planner-project-properties.c',
  'planner-property-dialog.c',
  'planner-property-model.c',
  'planner-resource-cmd.c',
  'planner-resource-dialog.c',
  'planner-resource-input-dialog.c',
  'planner-scale-utils.c',
  'planner-sidebar.c',
  'planner-table-print-sheet.c',
  'planner-task-cmd.c',
  'planner-task-dialog.c',
  'planner-task-input-dialog.c',
  'planner-util.c',
  'planner-view.c',
  'planner-window.c',
  'planner-working-time-dialog.c',
]

gantt_task_srcs = [
  'planner-gantt-background.c',
  'planner-gantt-header.c',
  'planner-gantt-model.c',
  'planner-task-tree.c',
  'planner-task-popup.c',
  'planner-relation-arrow.c',
  'planner-gantt-chart.c',
  'planner-gantt-print.c',
  'planner-gantt-row.c',
  'planner-gantt-view.c',
  'planner-task-view.c',
]

resource_srcs = [
  'planner-resource-view.c',
]

usage_srcs = [
  'planner-usage-view.c',
  'planner-usage-chart.c',
  'planner-usage-model.c',
  'planner-usage-row.c',
  'planner-usage-tree.c',
]

planner_marshal_srcs = gnome.genmarshal('planner-marshal',
  sources: 'planner-marshal.list',
  prefix: 'planner_marshal',
)

libplanner_app_library = static_library('libplannerapp',
  [libplanner_app_srcs, gantt_task_srcs, resource_srcs, usage_srcs, planner_marshal_srcs],
  dependencies: [planner_deps, m_dep],
  include_directories: [toplevel_inc, src_inc],
  link_with: gnomecanvas_lib,
  install: false,
)
libplanner_app_dep = declare_dependency(
  link_with: libplanner_app_library,
  include_directories: src_inc,
  dependencies: [planner_deps, libplanner_dep],
)
planner_app = executable('planner',
  ['planner-main.c', resources, ui_resources],
  dependencies: [libplanner_app_dep, planner_deps, libplanner_dep],
  export_dynamic: true,
  include_directories: [toplevel_inc],
  install: true,
  install_rpath: planner_pkglibdir,
)

if gda_dep.found()
  libsql_plugin_srcs = [ 'planner-sql-plugin.c' ] + sql_plugin_resources
  libsql_plugin_module = shared_module('sql-plugin', [libsql_plugin_srcs],
    dependencies: [planner_deps, gda_dep],
    include_directories: [toplevel_inc],
    install: true,
    install_dir: planner_plugindir,
  )
endif

if libeds_dep.found()
  libeds_plugin_srcs = [ 'planner-eds-plugin.c' ] + eds_plugin_resources
  libeds_plugin_module = shared_module('eds-plugin', [libeds_plugin_srcs],
    dependencies: [planner_deps, libeds_dep],
    include_directories: [toplevel_inc],
    install: true,
    install_dir: planner_plugindir,
  )
endif

libhtml_plugin_srcs = [ 'planner-html-plugin.c' ]
libhtml_plugin_module = shared_module('html-plugin',
  [libhtml_plugin_srcs],
  dependencies: [planner_deps],
  include_directories: [toplevel_inc],
  install: true,
  install_dir: planner_plugindir,
)

libmsp_plugin_srcs = [ 'planner-msp-plugin.c' ]
libmsp_plugin_module = shared_module('msp-plugin',
  [libmsp_plugin_srcs],
  dependencies: [planner_deps, xslt_dep, exslt_dep],
  include_directories: [toplevel_inc],
  install: true,
  install_dir: planner_plugindir,
)
