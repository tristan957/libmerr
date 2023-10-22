/* SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 * SPDX-FileCopyrightText: Copyright 2015 Micron Technology, Inc.
 * SPDX-FileCopyrightText: 2023 Tristan Partin <tristan@partin.io>
 */

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <tester.h>

#include <glib.h>

#include <merr.h>

#if __has_attribute(unused) && defined(MERR_PLAIN)
#define MERR_PLAIN_UNUSED __attribute__((unused))
#else
#define MERR_PLAIN_UNUSED
#endif

extern char merr_base[];
extern char merr_bug0[];
extern char merr_bug1[];
extern char merr_bug2[];

extern uint8_t __start_merr;
extern uint8_t __stop_merr;

static const char * MERR_CONST
ctx_stringify(const int ctx)
{
    (void)ctx;

    return "My context";
}

#ifndef MERR_PLAIN
static void
test_merr_bad_file(void)
{
    merr_t err;

    err = merr_pack(EAGAIN, 0, (char *)&__stop_merr, 123);
    g_assert_cmpstr(merr_file(err), ==, merr_bug0);

    err = merr_pack(EAGAIN, 0, (char *)&__start_merr + 1, 123);
    g_assert_cmpstr(merr_file(err), ==, merr_bug1);

    err = merr(EAGAIN);
    err &= ((intptr_t)(merr_base - __start_merr) << MERR_FILE_SHIFT);
    g_assert_cmpstr(merr_file(err), ==, merr_bug2);
}

static void
test_merr_long_path(void)
{
    merr_t err;
    size_t found_sz, expected_sz;
    char found[512], expected[512];

    err = tester_generate_err();
    g_assert_cmpint(err, !=, 0);

    expected_sz = (size_t)snprintf(
        expected, sizeof(expected), "%*s:%d: %s (%d)", (int)MERR_MAX_PATH_LENGTH, merr_file(err),
        merr_lineno(err), strerror(merr_errno(err)), merr_errno(err));
    found_sz = merr_strerrorx(err, found, sizeof(found), ctx_stringify);
    g_assert_cmpuint(found_sz, ==, expected_sz);
    g_assert_cmpstr(found, ==, expected);
}
#endif

static void
test_merr_with_context(void)
{
    merr_t err;
    int line MERR_PLAIN_UNUSED;
    size_t found_sz, expected_sz;
    char found[512], expected[512];
    const char *file MERR_PLAIN_UNUSED = __FILE__;

#ifndef MERR_PLAIN
    err = merrx(ENOENT, INT16_MAX + 1);
    g_assert_cmpint(merr_errno(err), ==, EINVAL);

    err = merrx(ENOENT, INT16_MIN - 1);
    g_assert_cmpint(merr_errno(err), ==, EINVAL);
#endif

#ifdef MERR_REL_SRC_DIR
    /* Point the file pointer past the prefix in order to retrieve the file
     * path relative to the root of the source tree.
     */
    file += sizeof(MERR_REL_SRC_DIR) - 1;
#endif

    // clang-format off
    err = merrx(ENOENT, 2); line = __LINE__;
#ifndef MERR_PLAIN
    expected_sz = (size_t)snprintf(
        expected, sizeof(expected), "%s:%d: %s (%d): %s (%d)", file, merr_lineno(err),
        strerror(merr_errno(err)), merr_errno(err), ctx_stringify(merr_ctx(err)), merr_ctx(err));
#else
    expected_sz = (size_t)snprintf(
        expected, sizeof(expected), "%s (%d): %s (%d)",
        strerror(merr_errno(err)), merr_errno(err), ctx_stringify(merr_ctx(err)), merr_ctx(err));
#endif

    // clang-format on
    found_sz = merr_strerrorx(err, found, sizeof(found), ctx_stringify);
    g_assert_cmpint(merr_ctx(err), ==, 2);
#ifndef MERR_PLAIN
    g_assert_cmpint(merr_lineno(err), ==, line);
    g_assert_cmpstr(merr_file(err), ==, file);
#endif
    g_assert_cmpuint(found_sz, ==, expected_sz);
    g_assert_cmpstr(found, ==, expected);
    found_sz = merr_strerrorx(err, NULL, 0, ctx_stringify);
    g_assert_cmpuint(found_sz, ==, expected_sz);
    found_sz = merr_strerrorx(err, NULL, sizeof(found), ctx_stringify);
    g_assert_cmpuint(found_sz, ==, expected_sz);
}

static void
test_merr_none(void)
{
    merr_t err;
    char found[512];
    size_t found_sz;

    err = merr(0);
    found_sz = merr_strerrorx(err, found, sizeof(found), NULL);
    g_assert_cmpint(err, ==, 0);
    g_assert_cmpint(merr_errno(err), ==, 0);
    g_assert_cmpint(merr_ctx(err), ==, 0);
#ifndef MERR_PLAIN
    g_assert_cmpint(merr_lineno(err), ==, 0);
    g_assert_null(merr_file(err));
#endif
    g_assert_cmpuint(found_sz, ==, 7);
    g_assert_cmpstr(found, ==, "Success");
}

static void
test_merr_without_context(void)
{
    merr_t err;
    int line MERR_PLAIN_UNUSED;
    size_t found_sz, expected_sz;
    char found[512], expected[512];
    const char *file MERR_PLAIN_UNUSED = __FILE__;

#ifdef MERR_REL_SRC_DIR
    /* Point the file pointer past the prefix in order to retrieve the file
     * path relative to the root of the source tree.
     */
    file += sizeof(MERR_REL_SRC_DIR) - 1;
#endif

    // clang-format off
    err = merr(ENOENT); line = __LINE__;

#ifndef MERR_PLAIN
    expected_sz = (size_t)snprintf(
        expected, sizeof(expected), "%s:%d: %s (%d)", file, merr_lineno(err),
        strerror(merr_errno(err)), merr_errno(err));
#else
    expected_sz = (size_t)snprintf(
        expected, sizeof(expected), "%s (%d)",
        strerror(merr_errno(err)), merr_errno(err));
#endif

    // clang-format on
    found_sz = merr_strerror(err, found, sizeof(found));
    g_assert_cmpint(merr_errno(err), ==, 2);
    g_assert_cmpint(merr_ctx(err), ==, 0);
#ifndef MERR_PLAIN
    g_assert_cmpint(merr_lineno(err), ==, line);
    g_assert_cmpstr(merr_file(err), ==, file);
#endif
    g_assert_cmpuint(found_sz, ==, expected_sz);
    g_assert_cmpstr(found, ==, expected);
    found_sz = merr_strerror(err, NULL, 0);
    g_assert_cmpuint(found_sz, ==, expected_sz);
    found_sz = merr_strerror(err, NULL, sizeof(found));
    g_assert_cmpuint(found_sz, ==, expected_sz);
}

int
main(int argc, char *argv[])
{
    g_test_init(&argc, &argv, NULL);

#ifndef MERR_PLAIN
    g_test_add_func("/merr/bad-file", test_merr_bad_file);
    g_test_add_func("/merr/long-path", test_merr_long_path);
#endif
    g_test_add_func("/merr/none", test_merr_none);
    g_test_add_func("/merr/with-context", test_merr_with_context);
    g_test_add_func("/merr/without-context", test_merr_without_context);

    return g_test_run();
}
