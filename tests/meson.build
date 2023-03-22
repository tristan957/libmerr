# SPDX-License-Identifier: Apache-2.0 OR MIT
#
# SPDX-FileCopyrightText: 2023 Tristan Partin <tristan@partin.io>

# Version which made TAP the default
glib_dep = dependency('glib-2.0', version: '>= 2.62', required: true)

test_env = environment({
    'G_TEST_SRCDIR': meson.current_source_dir(),
    'G_TEST_BUILDDIR': meson.current_build_dir()
})

tests = [
    'merr-test',
]

foreach t : tests
    e = executable(t, '@0@.c'.format(t), dependencies: [libmerr_dep, glib_dep])

    test(t, e, env: test_env, protocol: 'tap')
endforeach
