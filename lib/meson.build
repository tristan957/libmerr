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

cc_has_aligned = cc.has_function_attribute('aligned')
cc_has_section = cc.has_function_attribute('section')
if get_option('plain').disabled() and (not cc_has_aligned or not cc_has_section)
    error('Compiler must support both aligned' +
        ' and section attributes if -Dplain=disabled')
endif

c_args = []
if get_option('plain').enabled() or not cc_has_aligned or not cc_has_section
    c_args += '-DMERR_PLAIN'
endif

libmerr = static_library(
    'merr',
    'merr.c',
    c_args: c_args,
    include_directories: libmerr_includes,
    install: true,
    gnu_symbol_visibility: 'hidden'
)

variables = {
    # Keep in sync with typedef
    'merr_type': 'int64_t',
}

libmerr_dep = declare_dependency(
    compile_args: c_args,
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
        extra_cflags: c_args,
        variables: variables
    )
endif
