exe = executable('mrp-time-test',
  ['mrptime.c'],
  include_directories: libplanner_inc,
  dependencies: [
    libplanner_dep,
    glib_dep
  ]
)

test('mrp-time-test', exe)
