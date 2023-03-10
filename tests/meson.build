# SPDX-License-Identifier: MIT
#
# SPDX-FileCopyrightText: 2023 Tristan Partin <tristan@partin.io>

# Version which made TAP the default
glib_dep = dependency('glib-2.0', version: '>= 2.62', required: true)

test_env = environment({
    'G_TEST_SRCDIR': meson.current_source_dir(),
    'G_TEST_BUILDDIR': meson.current_build_dir()
})

tests = [
    'merr-test.c',
]

foreach t : tests
    name = fs.stem(t)

    e = executable(name, t, dependencies: [libmerr_dep, glib_dep])

    test(name, e, env: test_env, protocol: 'tap')
endforeach
