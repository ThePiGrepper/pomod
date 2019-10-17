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

scdoc = find_program('scdoc', required: get_option('man-pages'))

if scdoc.found()
  sh = find_program('sh')

  man_pages = ['pomod.1.scd']

  mandir = get_option('mandir')

  foreach src : man_pages
    topic = src.split('.')[0]
    section = src.split('.')[1]
    output = '@0@.@1@'.format(topic, section)

    custom_target(
      output,
      input: src,
      output: output,
      command: [
        sh, '-c', '@0@ < @INPUT@ > @1@'.format(scdoc.path(), output)
      ],
      install: true,
      install_dir: '@0@/man@1@'.format(mandir, section)
    )
  endforeach
endif
