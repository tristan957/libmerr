# SPDX-License-Identifier: Apache-2.0 OR MIT
#
# SPDX-FileCopyrightText: Copyright 2022 Micron Technology, Inc.
# SPDX-FileCopyrightText: 2023 Tristan Partin <tristan@partin.io>

with_libbsd = not cc.has_header_symbol('strlcpy', 'string.h')

if with_libbsd
    # TODO: Determine when either libbsd supported pkgconfig OR when strlcpy was
    # added.
    libbsd_dep = dependency(
        'libbsd',
        required: with_libbsd,
        default_options: [
            'default_library=static',
            'warning_level=0',
            'werror=false',
        ]
    )
endif

configure_file(
    input: 'config.h.in',
    output: 'config.h',
    configuration: {
        'WITH_LIBBSD': with_libbsd,
    }
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
