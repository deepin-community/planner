subdir('libplanner')

libselfcheck_srcs = [ 'self-check.c' ]
#test_inc = include_directories('.')
libselfcheck_library = static_library('selfcheck',
  [libselfcheck_srcs],
  dependencies: [libplanner_app_dep, libplanner_dep],
  include_directories: [toplevel_inc],
  install: false,
)
libselfcheck_dep = declare_dependency(
  link_with: libselfcheck_library,
  dependencies: [libplanner_app_dep, libplanner_dep],
  include_directories: [toplevel_inc],
)

test_env = [
  'PLANNER_STORAGEMODULEDIR=@0@/libplanner'.format(meson.project_build_root()),
  'PLANNER_FILEMODULESDIR=@0@/libplanner'.format(meson.project_build_root()),
  'PLANNER_PLUGINDIR=@0@/src'.format(meson.project_build_root()),
  'PLANNER_DATADIR=@0@/data'.format(meson.project_source_root()),
]
scheduler_test = executable('scheduler-test', 'scheduler-test.c',
  c_args: [
    '-DG_LOG_DOMAIN="Planner"',
    '-DEXAMPLESDIR="@0@/tests/files"'.format(meson.project_source_root()),
  ],
  dependencies: [libselfcheck_dep],
  include_directories: [toplevel_inc],
)
test('scheduler-test', scheduler_test, env: test_env)

time_test = executable('time-test', 'time-test.c',
  dependencies: [libselfcheck_dep],
)
test('time-test', time_test, env: test_env)

calendar_test = executable('calendar-test', 'calendar-test.c',
  dependencies: [libselfcheck_dep],
)
test('calendar-test', calendar_test, env: test_env)

task_test = executable('task-test', 'task-test.c',
  dependencies: [libselfcheck_dep],
)
test('task-test', task_test, env: test_env)

cmd_manager_test = executable('cmd-manager-test', 'cmd-manager-test.c',
  dependencies: [libselfcheck_dep],
)
test('cmd-manager-test', cmd_manager_test, env: test_env)
