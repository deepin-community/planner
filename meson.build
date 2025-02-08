project('planner',
  'c',
  version: '0.14.92',
  license: 'GPL-2+',
  meson_version: '>= 0.56',
)

planner_prefix = get_option('prefix')
planner_libdir = planner_prefix / get_option('libdir')
planner_pkglibdir = planner_libdir / 'planner'
planner_plugindir = planner_pkglibdir / 'plugins'
planner_storagemoduledir = planner_pkglibdir / 'storage-modules'
planner_filemoduledir = planner_pkglibdir / 'file-modules'
planner_datadir = planner_prefix / get_option('datadir')
planner_pkgdatadir = planner_datadir / 'planner'
planner_localedir = planner_prefix / get_option('localedir')
planner_sysconfdir = get_option('sysconfdir')
its_dir = meson.project_source_root() / 'gettext'
po_dir = meson.project_source_root() / 'po'

cc = meson.get_compiler('c')

i18n = import('i18n')
gnome = import('gnome')

conf_data = configuration_data()
conf_data.set_quoted('GETTEXT_PACKAGE', meson.project_name())
conf_data.set_quoted('VERSION', meson.project_version())
conf_data.set_quoted('GNOMELOCALEDIR', planner_localedir)
conf_data.set_quoted('PLUGINDIR', planner_plugindir)
conf_data.set_quoted('STORAGEMODULEDIR', planner_storagemoduledir)
conf_data.set_quoted('FILEMODULESDIR', planner_filemoduledir)
conf_data.set_quoted('DATADIR', planner_pkgdatadir)
conf_data.set('WITH_SIMPLE_PRIORITY_SCHEDULING', get_option('simple-priority-scheduling'))

configure_file(
  output: 'config.h',
  configuration: conf_data,
)
toplevel_inc = include_directories('.')
libplanner_inc = include_directories('libplanner')

glib_req_version = '2.56'
glib_req = '>= @0@'.format(glib_req_version)
gtk_req_version = '3.22'
gtk_req = '>= @0@'.format(gtk_req_version)
eds_req = '>= 3.6'

glib_dep = dependency('glib-2.0', version: glib_req)
gmodule_dep = dependency('gmodule-2.0')
gobject_dep = dependency('gobject-2.0')
gio_dep = dependency('gobject-2.0')
gtk_dep = dependency('gtk+-3.0', version: gtk_req)
gail_dep = dependency('gail-3.0', version: gtk_req)
libxml_dep = dependency('libxml-2.0', version: '>= 2.6.27')
xslt_dep = dependency('libxslt', version: '>= 1.1.23')
exslt_dep = dependency('libexslt')
m_dep = cc.find_library('m', required: false)

gda_dep = dependency('libgda-5.0', version: '>= 1.0', required: get_option('database-gda'))
libeds_dep = dependency('libebook-1.2', version: eds_req, required: get_option('eds'))

libplanner_deps = [glib_dep, gmodule_dep, gobject_dep, libxml_dep, m_dep]
planner_deps = [glib_dep, gobject_dep, gmodule_dep, gio_dep, gtk_dep]

glib_version_arr = glib_req_version.split('.')
glib_major_version = glib_version_arr[0]
glib_minor_version = glib_version_arr[1]

gtk_version_arr = gtk_req_version.split('.')
gtk_major_version = gtk_version_arr[0]
gtk_minor_version = gtk_version_arr[1]

add_project_arguments([
  '-DGLIB_VERSION_MIN_REQUIRED=GLIB_VERSION_@0@_@1@'.format(glib_major_version, glib_minor_version),
  '-DGLIB_VERSION_MAX_ALLOWED=GLIB_VERSION_@0@_@1@'.format(glib_major_version, glib_minor_version),
  '-DGDK_VERSION_MIN_REQUIRED=GDK_VERSION_@0@_@1@'.format(3, 8), # TODO: Move back to dynamic after 3.10 deprecations are handled (GtkUIManager/GtkAction vs GAction)
  '-DGDK_VERSION_MAX_ALLOWED=GDK_VERSION_@0@_@1@'.format(gtk_major_version, gtk_minor_version),
], language: 'c')

subdir('po')
subdir('libplanner')
subdir('data')
subdir('src')
subdir('examples')
subdir('tests')
subdir('docs')

meson.add_install_script('meson_post_install.sh')

status = [
  '', '',
  'planner @0@'.format(meson.project_version()),
  '',
]

status += [
  'Planner prefix               : @0@'.format(planner_prefix),
  'Build API docs               : @0@'.format(get_option('gtk_doc')),
  'Simple priority scheduling   : @0@'.format(get_option('simple-priority-scheduling')),
  'Database/GDA support         : @0@'.format(gda_dep.found()),
  'Evolution Data Server import : @0@'.format(libeds_dep.found()),
  #'Evolution Data Server backend: @0@'
  '',
]

message('\n  '.join(status))

if get_option('simple-priority-scheduling')
  message('*** Note: The use simple priority tasks scheduling is experimental. Don\'t use it in production systems ***')
endif

#if get_option('eds-backend')
#  message('*** Note: The Evolution backend is experimental. Don\'t use it in production systems ***')
#endif
