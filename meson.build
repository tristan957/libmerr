# SPDX-License-Identifier: Apache-2.0 OR MIT
#
# SPDX-FileCopyrightText: Copyright 2022 Micron Technology, Inc.
# SPDX-FileCopyrightText: 2023 Tristan Partin <tristan@partin.io>

project(
    'libmerr',
    'c',
    version: files('VERSION'),
    license: 'Apache-2.0 OR MIT',
        default_options: [
        'buildtype=debugoptimized',
        'c_std=c99',
        'warning_level=3',
    ],
    meson_version: '>= 0.59.0'
)

fs = import('fs', required: get_option('tests'))
pkg = import('pkgconfig', required: false)

cc = meson.get_compiler('c')
python = find_program('python3')

# Compute the relative path used by compiler invocations.
relative_dir = run_command(
    python,
    '-c',
    'import os; print("{}/".format(os.path.relpath("@0@", "@1@")))'.format(
        meson.project_source_root(),
        meson.global_build_root(),
    ),
    check: true
).stdout().strip()

# Strip relative path prefixes from the code if possible, otherwise hide them.
# The following snippet is inspired by the SwayWM Project under the MIT license.
add_project_arguments(
    cc.first_supported_argument(
        '-fmacro-prefix-map=@0@='.format(relative_dir),
        '-DMERR_REL_SRC_DIR="@0@"'.format(relative_dir)
    ),
    language: 'c'
)

add_project_arguments(cc.get_supported_arguments(
        '-D_POSIX_C_SOURCE=200112L',
        '-Wdeclaration-after-statement',
        '-Wconversion'
    ),
    language: 'c'
)

subdir('include')
subdir('lib')
if get_option('tests')
    subdir('tests')
endif
