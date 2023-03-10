# SPDX-License-Identifier: Apache-2.0 OR MIT
#
# SPDX-FileCopyrightText: 2023 Tristan Partin <tristan@partin.io>

project('tester', 'c')

cc = meson.get_compiler('c')
python = find_program('python3')

# Compute the relative path used by compiler invocations.
relative_path = run_command(
    python,
    '-c',
    'import os; print("{}/".format(os.path.relpath("@0@", "@1@")))'.format(
        meson.global_source_root(),
        meson.global_build_root(),
    ),
    check: true
).stdout().strip()

if not cc.has_argument('-fmacro-prefix-map=@0@='.format(relative_path))
    error('-fmacro-prefix-map is unsupported on this compiler')
endif

libmerr_dep = dependency('libmerr')

disable_warnings = []
if cc.get_id() == 'gcc'
    disable_warnings += '-w'
elif cc.get_id() == 'clang'
    disable_warnings += '-Wno-everything'
endif

tester = static_library(
    'tester',
    'this-is-a-really-long-filename-in-order-to-test-the-handling-of-the-long-file-name-scenario.c',
    c_args: disable_warnings,
    dependencies: libmerr_dep
)

tester_dep = declare_dependency(
    link_with: tester,
    include_directories: '.',
    dependencies: libmerr_dep
)
