subdir('icon')
subdir('hicolor')

i18n = import('i18n')

i18n.merge_file(
  'docker-images-idmin.desktop',
  input: 'docker-images-admin.desktop.in',
  output: 'docker-images-admin.desktop',
  install: true,
  po_dir: join_paths(meson.source_root(), 'po'),
  install_dir : join_paths(get_option('prefix'), 'share/applications'),
  type: 'desktop',
)

