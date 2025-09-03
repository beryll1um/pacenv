# Copyright (c) 2025 The Pacenv developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or https://opensource.org/license/mit/.

# Initialize Meson with project specific properties.
project('pacenv', 'c', default_options : ['c_std=c23', 'warning_level=3'],
  license : 'MIT', license_files : ['LICENSE'],
  # Meson version can be downgraded, I didn't rely on any version specifics
  # when choosing it. It's just the one that was installed on my system.
  meson_version : '>=1.8.2', version : '0.1.0')

# Create new executable to run the utility compilation.
executable('pacenv',
  [
    'src/pacenv/glob.c',
    'src/pacenv/main.c',
    'src/pacenv/util.c'
  ],
  include_directories : [ 'src/' ],
  install : true,
  dependencies : [
    # Find external dependencies in the system.
    dependency('json-c', required : true, version : '>=0.18'),
    dependency('libalpm', required : true, version : '>=15.0.0')
  ])

# vim: set ts=2 sw=2 expandtab:
