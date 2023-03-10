/* SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 * SPDX-FileCopyrightText: Copyright 2015 Micron Technology, Inc.
 * SPDX-FileCopyrightText: 2023 Tristan Partin <tristan@partin.io>
 */

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <glib.h>

#include <merr.h>

extern char merr_base[];
extern char merr_bug0[];
extern char merr_bug1[];
extern char merr_bug2[];

extern uint8_t __start_merr;
extern uint8_t __stop_merr;

static const char *
ctx_stringify(const unsigned int ctx)
{
    (void)ctx;

    return "My context";
}

static void
test_merr_bad_file(void)
{
    merr_t err;

    err = merr_pack(EAGAIN, 0, (char *)&__stop_merr, 123);
    g_assert_cmpstr(merr_file(err), ==, merr_bug0);

    err = merr_pack(EAGAIN, 0, (char *)&__start_merr + 1, 123);
    g_assert_cmpstr(merr_file(err), ==, merr_bug1);

    err = merr(EAGAIN);
    err &= ((uint64_t)(merr_base - __start_merr) << MERR_FILE_SHIFT);
    g_assert_cmpstr(merr_file(err), ==, merr_bug2);
}

static void
test_merr_full(void)
{
    int line;
    merr_t err;
    const char *file = __FILE__;
    size_t found_sz, expected_sz;
    char found[512], expected[512];

    // clang-format off
    err = merrx(ENOENT, 2); line = __LINE__;
    // clang-format on
    expected_sz = (size_t)snprintf(
        expected, sizeof(expected), "%s:%d: %s (%d): %s (%u)", file, merr_lineno(err),
        strerror(merr_errno(err)), merr_errno(err), ctx_stringify(merr_ctx(err)), merr_ctx(err));
    found_sz = merr_strerror(err, found, sizeof(found), ctx_stringify);
    g_assert_cmpuint(merr_ctx(err), ==, 2);
    g_assert_cmpint(merr_lineno(err), ==, line);
    g_assert_cmpstr(merr_file(err), ==, file);
    g_assert_cmpuint(found_sz, ==, expected_sz);
    g_assert_cmpstr(found, ==, expected);
    found_sz = merr_strerror(err, NULL, 0, ctx_stringify);
    g_assert_cmpuint(found_sz, ==, expected_sz);
    found_sz = merr_strerror(err, NULL, sizeof(found), ctx_stringify);
    g_assert_cmpuint(found_sz, ==, expected_sz);
}

static void
test_merr_none(void)
{
    merr_t err;
    char found[512];
    size_t found_sz;

    err = merr(0);
    found_sz = merr_strerror(err, found, sizeof(found), NULL);
    g_assert_cmpuint(err, ==, 0);
    g_assert_cmpuint(found_sz, ==, 7);
    g_assert_cmpstr(found, ==, "success");
}

static void
test_merr_plain(void)
{
    int line;
    merr_t err;
    const char *file = __FILE__;
    size_t found_sz, expected_sz;
    char found[512], expected[512];

#ifdef MERR_REL_SRC_DIR
    /* Point the file pointer past the prefix in order to retrieve the file
     * path relative to the root of the source tree.
     */
    file += sizeof(MERR_REL_SRC_DIR) - 1;
#endif

    // clang-format off
    err = merrx(ENOENT, 2); line = __LINE__;
    // clang-format on
    expected_sz = (size_t)snprintf(
        expected, sizeof(expected), "%s:%d: %s (%d): %s (%u)", file, merr_lineno(err),
        strerror(merr_errno(err)), merr_errno(err), ctx_stringify(merr_ctx(err)), merr_ctx(err));
    found_sz = merr_strerror(err, found, sizeof(found), ctx_stringify);
    g_assert_cmpuint(merr_ctx(err), ==, 2);
    g_assert_cmpint(merr_lineno(err), ==, line);
    g_assert_cmpstr(merr_file(err), ==, file);
    g_assert_cmpuint(found_sz, ==, expected_sz);
    g_assert_cmpstr(found, ==, expected);
    found_sz = merr_strerror(err, NULL, 0, ctx_stringify);
    g_assert_cmpuint(found_sz, ==, expected_sz);
    found_sz = merr_strerror(err, NULL, sizeof(found), ctx_stringify);
    g_assert_cmpuint(found_sz, ==, expected_sz);
}

int
main(int argc, char *argv[])
{
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/merr/bad-file", test_merr_bad_file);
    g_test_add_func("/merr/full", test_merr_full);
    g_test_add_func("/merr/none", test_merr_none);
    g_test_add_func("/merr/plain", test_merr_plain);

    return g_test_run();
}
