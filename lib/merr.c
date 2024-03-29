/* SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 * SPDX-FileCopyrightText: Copyright 2015 Micron Technology, Inc.
 * SPDX-FileCopyrightText: 2023 Tristan Partin <tristan@partin.io>
 */

#include "config.h"

#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <merr.h>

#if __has_attribute(unused)
#define MERR_UNUSED __attribute__((unused))
#else
#define MERR_UNUSED
#endif

#ifndef MERR_PLAIN
char merr_base[MERR_MAX_PATH_LENGTH] merr_attributes = "merr_base";
char merr_bug0[MERR_MAX_PATH_LENGTH] merr_attributes = "merr_bug0";
char merr_bug1[MERR_MAX_PATH_LENGTH] merr_attributes = "merr_bug1";
char merr_bug2[MERR_MAX_PATH_LENGTH] merr_attributes = "merr_bug2";

extern uint8_t __start_merr;
extern uint8_t __stop_merr;
#endif

#ifndef HAVE_STRLCPY

static size_t
strlcpy(char * const dst, const char * const src, const size_t sz)
{
    size_t len;

    if (!dst || sz == 0)
        return strlen(src);

    for (len = 0; len < sz; len++) {
        const char c = src[len];

        if (len == sz - 1 || c == '\0') {
            dst[len] = '\0';
            break;
        }

        dst[len] = c;
    }

    return len;
}

#endif

#ifndef MERR_PLAIN

const char *
merr_file(const merr_t err)
{
    int64_t off;
    const char *file;

    if (err == 0)
        return NULL;

    off = (int64_t)((uint64_t)err & MERR_FILE_MASK) >> MERR_FILE_SHIFT;
    if (off == 0)
        return NULL;

    file = (char *)merr_base + (off * MERR_MAX_PATH_LENGTH);
    if (file < (char *)&__start_merr || file >= (char *)&__stop_merr)
        return merr_bug2;

#ifdef MERR_REL_SRC_DIR
    if (file == merr_bug0 || file == merr_bug1 || file == merr_bug2)
        return file;

    /* Point the file pointer past the prefix in order to retrieve the file
     * path relative to the root of the source tree.
     */
    file += sizeof(MERR_REL_SRC_DIR) - 1;
#endif

    return file;
}

merr_t
merr_pack(const int errnum, const int ctx, const char *file, const uint16_t line)
{
    int64_t off;
    merr_t err = 0;

    if (errnum == 0)
        return 0;

    if (errnum < INT16_MIN || errnum > INT16_MAX || ctx < INT16_MIN || ctx > INT16_MAX)
        return merr(EINVAL);

    if (file < (char *)&__start_merr || file >= (char *)&__stop_merr) {
        file = merr_bug0;
    } else if ((uintptr_t)file % MERR_MAX_PATH_LENGTH != 0) {
        file = merr_bug1;
    }

    off = (file - merr_base) / MERR_MAX_PATH_LENGTH;

    if (((off << MERR_FILE_SHIFT) >> MERR_FILE_SHIFT) == off)
        err = off << MERR_FILE_SHIFT;

    err |= ((int64_t)line << MERR_LINE_SHIFT) & MERR_LINE_MASK;
    err |= (ctx << MERR_CTX_SHIFT) & MERR_CTX_MASK;
    err |= (int64_t)errnum & MERR_ERRNO_MASK;

    return err;
}

#else

merr_t
merr_pack(const int errnum, const int ctx)
{
    merr_t err = 0;

    if (errnum == 0)
        return 0;

    if (errnum < INT32_MIN || errnum > INT32_MAX || ctx < INT32_MIN || ctx > INT32_MAX)
        return merr(EINVAL);

    err |= ((int64_t)ctx << MERR_CTX_SHIFT);
    err |= (int64_t)errnum & MERR_ERRNO_MASK;

    return err;
}

#endif

static size_t
strerror_safe(const merr_t err, char * const buf, const size_t buf_sz)
{
    int errnum;
    int rc MERR_UNUSED;
    char err_str[1024];

    errnum = merr_errno(err);

    // XSI/POSIX
    errno = 0;
    rc = strerror_r(errnum, err_str, sizeof(err_str));
    assert(rc == 0);

    if (!buf)
        return strlen(err_str);

    return strlcpy(buf, err_str, buf_sz);
}

size_t
merr_strerrorx(const merr_t err, char * const buf, size_t buf_sz, merr_stringify ctx_stringify)
{
    int ret;
    size_t sz = 0;
    MERR_CTX_TYPE ctx;

    if (!buf && buf_sz > 0)
        buf_sz = 0;

    if (!err)
        return strlcpy(buf, "Success", buf_sz);

#ifndef MERR_PLAIN
    {
        const char *file;

        file = merr_file(err);
        if (file) {
            const char *ptr;

            // Protect against files that may be too long.
            ptr = memchr(file, '\0', MERR_MAX_PATH_LENGTH);
            ret = snprintf(
                buf, buf_sz, "%*s:%d: ", (int)(ptr ? ptr - file : MERR_MAX_PATH_LENGTH), file,
                merr_lineno(err));

            if (ret < 0) {
                sz = strlcpy(buf, "<failed to format the error message>", buf_sz);
                goto out;
            }

            sz += (size_t)ret;
        }
    }
#endif

    if (sz >= buf_sz) {
        sz += strerror_safe(err, NULL, 0);
    } else {
        sz += strerror_safe(err, buf + sz, buf_sz - sz);
    }

    if (sz >= buf_sz) {
        ret = snprintf(NULL, 0, " (%d)", merr_errno(err));
    } else {
        ret = snprintf(buf + sz, buf_sz - sz, " (%d)", merr_errno(err));
    }

    if (ret < 0) {
        // Try to just return what we already have.
        if (buf_sz > 0)
            buf[sz - 1] = '\000';
        goto out;
    }

    sz += (size_t)ret;

    ctx = merr_ctx(err);
    if (ctx != 0 && ctx_stringify) {
        const char *msg = ctx_stringify(ctx);

        if (msg) {
            if (sz >= buf_sz) {
                ret = snprintf(NULL, 0, ": %s (%d)", msg, ctx);
            } else {
                ret = snprintf(buf + sz, buf_sz - sz, ": %s (%d)", msg, ctx);
            }

            if (ret < 0) {
                // Try to just return what we already have.
                buf[sz] = '\000';
                goto out;
            }

            sz += (size_t)ret;
        }
    }

out:
    return sz;
}
