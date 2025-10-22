project('xf86-input-libinput', 'c',
        version: '1.5.0',  # bump version in configure.ac
        default_options: ['warning_level=2'],
        meson_version: '>= 0.50.0')

driver_version = meson.project_version().split('.')

dir_pkgconf	= get_option('prefix') / get_option('libdir') / 'pkgconfig'
dir_man4	= get_option('prefix') / get_option('mandir') / 'man4'

cc = meson.get_compiler('c')
cflags = [
	'-Wno-unused-parameter',
	'-Wno-sign-compare',	 # lots of work to get rid of this
	'-Wmissing-prototypes',
	'-Wstrict-prototypes',
	'-Wlogical-op',
	'-Wpointer-arith',
	'-Wuninitialized',
	'-Winit-self',
	'-Wstrict-prototypes',
	'-Wimplicit-fallthrough',
	'-Wredundant-decls',
	'-Wincompatible-pointer-types',
	'-Wformat=2',
	'-Wno-missing-field-initializers',
	'-Wmissing-declarations',

	'-fvisibility=hidden',
]
add_project_arguments(cc.get_supported_arguments(cflags), language: 'c')

pkgconfig = import('pkgconfig')
dep_xserver = dependency('xorg-server', version: '>= 1.19')
dep_xproto = dependency('xproto')
dep_inputproto = dependency('inputproto', version: '>= 2.2')
dep_libinput = dependency('libinput', version: '>= 1.11.0')

config_h = configuration_data()
config_h.set('PACKAGE_VERSION_MAJOR', driver_version[0])
config_h.set('PACKAGE_VERSION_MINOR', driver_version[1])
config_h.set('PACKAGE_VERSION_PATCHLEVEL', driver_version[2])

if dep_inputproto.version().version_compare('>= 2.3.99.1')
	config_h.set('HAVE_INPUTPROTO24', 1)
endif

if cc.has_function('libinput_device_config_scroll_get_button_lock',
		   dependencies: dep_libinput)
	config_h.set('HAVE_LIBINPUT_SCROLL_BUTTON_LOCK', 1)
endif
if cc.has_function('libinput_event_pointer_get_scroll_value_v120',
	           dependencies: dep_libinput)
	config_h.set('HAVE_LIBINPUT_AXIS_VALUE_V120', 1)
endif
if cc.has_function('libinput_config_accel_create',
		   dependencies: dep_libinput)
	config_h.set('HAVE_LIBINPUT_CUSTOM_ACCEL', 1)
endif
if cc.has_function('libinput_tablet_tool_config_pressure_range_set',
		   dependencies: dep_libinput)
	config_h.set('HAVE_LIBINPUT_PRESSURE_RANGE', 1)
endif
if cc.has_function('libinput_device_config_click_set_clickfinger_button_map',
		   dependencies: dep_libinput)
	config_h.set('HAVE_LIBINPUT_CLICKFINGER_BUTTON_MAP', 1)
endif
if cc.has_header_symbol('libinput.h', 'LIBINPUT_LED_COMPOSE',
			dependencies: dep_libinput)
	config_h.set('HAVE_LIBINPUT_COMPOSE_AND_KANA', 1)
endif
if cc.has_function('libinput_plugin_system_load_plugins', dependencies: dep_libinput)
	config_h.set('HAVE_LIBINPUT_PLUGINS', 1)
endif

dir_headers = get_option('sdkdir')
if dir_headers == ''
	dir_headers = dep_xserver.get_pkgconfig_variable('sdkdir')
endif

dir_xorg_module = get_option('xorg-module-dir')
if dir_xorg_module == ''
    dir_xorg_module = dep_xserver.get_pkgconfig_variable('moduledir') / 'input'
endif

dir_xorg_conf = get_option('xorg-conf-dir')
if dir_xorg_conf == ''
    dir_xorg_conf = dep_xserver.get_pkgconfig_variable('sysconfigdir')
endif

libbezier = static_library('bezier', ['src/bezier.c', 'src/bezier.h'])
dep_libbezier = declare_dependency(link_with: libbezier)

libdraglock = static_library('draglock', ['src/draglock.c', 'src/draglock.h'])
dep_libdraglock = declare_dependency(link_with: libdraglock)

dep_drivers = [
  dep_xserver,
  dep_xproto,
  dep_inputproto,
  dep_libinput,
  dep_libbezier,
  dep_libdraglock,
]

driver_src = ['src/xf86libinput.c', 'src/util-strings.c']
driver_lib = shared_module(
	'libinput_drv',
	driver_src,
	name_prefix: '', # we want libinput_drv.so, not liblibinput_drv.so
	include_directories: include_directories('include'),
	dependencies: dep_drivers,
	install: true,
	install_dir: dir_xorg_module,
)

test_bezier = executable('test-bezier',
			 ['test/test-bezier.c'],
			 dependencies: dep_libbezier,
			 include_directories: include_directories('src'),
			 install: false)
test('test-bezier', test_bezier)

test_draglock = executable('test-draglock',
			 ['test/test-draglock.c'],
			 dependencies: dep_libdraglock,
			 include_directories: include_directories('src'),
			 install: false)
test('test-draglock', test_draglock)


conf_pkgconf = configuration_data()
conf_pkgconf.set('PACKAGE_VERSION', meson.project_version())
conf_pkgconf.set('sdkdir', dir_headers)

configure_file(
	input: 'xorg-libinput.pc.in',
	output: 'xorg-libinput.pc',
	configuration: conf_pkgconf,
	install_dir: dir_pkgconf,
)

config_man = configuration_data()
config_man.set('VERSION', '@0@ @1@'.format(meson.project_name(), meson.project_version()))

configure_file(
	input: 'man/libinput.man',
	output: 'libinput.4',
	configuration: config_man,
	install_dir: dir_man4
)

install_headers('include/libinput-properties.h', install_dir: dir_headers)

install_data('conf/40-libinput.conf', install_dir: dir_xorg_conf)

# Now generate config.h
configure_file(output: 'config.h', configuration: config_h)
