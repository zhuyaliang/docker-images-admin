gnome = import('gnome')

sources = [
'main.c',
'docker-images-share.c',
'docker-images-local.c', 
'docker-images-remote.c',  
'docker-images-list.c',   
'docker-images-utils.c',   
]

account_src = gnome.gdbus_codegen(
  'account-generated',
  'org.freedesktop.Accounts.xml',
  interface_prefix : 'org.freedesktop.',
  namespace : 'Admin'
)

account_user_src = gnome.gdbus_codegen(
  'account-user-generated',
  'org.freedesktop.Accounts.User.xml',
  interface_prefix : 'org.freedesktop.',
  namespace : 'Admin'
  )

executable('images-admin',
  sources : sources,
  dependencies : [gtk_deps,curl_deps,json_deps],
  include_directories: top_srcdir,
  install : true,
  install_dir : get_option('bindir')
  )
