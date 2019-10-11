project('pomod',
  'c',
  version: '0.1',
  license: 'MIT',
  default_options: [
    'c_std=c11',
    'warning_level=2',
    ],
)

libnotify = dependency('libnotify')

cc = meson.get_compiler('c')

src_files = [
  'pomod.c',
  ]

executable(
	meson.project_name(),
  files(src_files),
  dependencies: [
    libnotify,
    ],
)
