# SPDX-License-Identifier: MIT
#
# SPDX-FileCopyrightText: Copyright 2022 Micron Technology, Inc.
# SPDX-FileCopyrightText: 2023 Tristan Partin <tristan@partin.io>

# TODO: Determine when either libbsd supported pkgconfig OR when strlcpy was
# added.
libbsd_dep = dependency(
    'libbsd',
    required: not cc.has_header_symbol('strlcpy', 'string.h'),
    default_options: [
        'default_library=static',
        'warning_level=0',
        'werror=false',
    ]
)

libmerr = static_library(
    'merr',
    'merr.c',
    include_directories: libmerr_includes,
    dependencies: libbsd_dep,
    install: true
)

libmerr_dep = declare_dependency(
    link_with: libmerr,
    include_directories: libmerr_includes
)

meson.override_dependency(meson.project_name(), libmerr_dep)

if pkg.found()
    pkg.generate(
        libmerr,
        name: meson.project_name(),
        description: 'C library for error information'
    )
endif
