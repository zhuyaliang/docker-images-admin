actions_icons = [
  ['16x16', 'docker-images-admin.jpg'],
  ['22x22', 'docker-images-admin.jpg'],
  ['24x24', 'docker-images-admin.jpg'],
  ['32x32', 'docker-images-admin.jpg'],
  ['48x48', 'docker-images-admin.jpg']
]

foreach icon: actions_icons
  install_data(
    join_paths(icon[0], 'apps', icon[1]),
    install_dir: join_paths(get_option('prefix'), 'share/icons', 'hicolor', icon[0], 'apps')
  )
endforeach
