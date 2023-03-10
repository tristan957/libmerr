# SPDX-License-Identifier: Apache-2.0 OR MIT
#
# SPDX-FileCopyrightText: Copyright 2022 Micron Technology, Inc.
# SPDX-FileCopyrightText: 2023 Tristan Partin <tristan@partin.io>

configure_file(
    input: 'config.h.in',
    output: 'config.h',
    configuration: {
        'HAVE_STRLCPY': cc.has_header_symbol('strlcpy', 'string.h'),
    }
)

libmerr = static_library(
    'merr',
    'merr.c',
    include_directories: libmerr_includes,
    install: true,
    gnu_symbol_visibility: 'hidden'
)

variables = {
    # Keep in sync with typedef
    'merr_type': 'int64_t',
}

libmerr_dep = declare_dependency(
    link_with: libmerr,
    include_directories: libmerr_includes,
    variables: variables
)

meson.override_dependency(meson.project_name(), libmerr_dep)

if pkg.found()
    pkg.generate(
        libmerr,
        name: meson.project_name(),
        description: 'C99+ library for error information',
        url: 'https://github.com/tristan957/libmerr',
        variables: variables
    )
endif
