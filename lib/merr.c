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

#ifdef WITH_LIBBSD
#include <bsd/string.h>
#endif

#include <merr.h>

#if __has_attribute(unused)
#define MERR_UNUSED __attribute__((unused))
#else
#define MERR_UNUSED
#endif

char merr_base[MERR_ALIGN] _merr_attributes = "merr_base";
char merr_bug0[MERR_ALIGN] _merr_attributes = "merr_bug0";
char merr_bug1[MERR_ALIGN] _merr_attributes = "merr_bug1";
char merr_bug2[MERR_ALIGN] _merr_attributes = "merr_bug2";

extern uint8_t __start_merr;
extern uint8_t __stop_merr;

const char *
merr_file(const merr_t err)
{
    int64_t off;
    const char *file;

    if (err == 0)
        return NULL;

    off = (int64_t)(err & MERR_FILE_MASK) >> MERR_FILE_SHIFT;
    if (off == 0)
        return NULL;

    file = (char *)merr_base + (off * MERR_ALIGN);
    if (file < (char *)&__start_merr || file >= (char *)&__stop_merr)
        return merr_bug2;

#ifdef MERR_REL_SRC_DIR
    if ((uintptr_t)file == (uintptr_t)merr_bug0 || (uintptr_t)file == (uintptr_t)merr_bug1 ||
        (uintptr_t)file == (uintptr_t)merr_bug2)
    {
        return file;
    }

    /* Point the file pointer past the prefix in order to retrieve the file
     * path relative to the root of the source tree.
     */
    file += sizeof(MERR_REL_SRC_DIR) - 1;
#endif

    return file;
}

merr_t
merr_pack(const int errnum, const unsigned int ctx, const char *file, const unsigned int line)
{
    int64_t off;
    merr_t err = 0;

    if (errnum == 0)
        return 0;

    if (errnum > INT16_MAX || ctx > UINT16_MAX)
        return merr(EINVAL);

    if (file < (char *)&__start_merr || file >= (char *)&__stop_merr) {
        file = merr_bug0;
    } else if ((uintptr_t)file % MERR_ALIGN != 0) {
        file = merr_bug1;
    }

    off = (file - merr_base) / MERR_ALIGN;

    if (((int64_t)((uint64_t)off << MERR_FILE_SHIFT) >> MERR_FILE_SHIFT) == off)
        err = (uint64_t)off << MERR_FILE_SHIFT;

    err |= ((uint64_t)line << MERR_LINE_SHIFT) & MERR_LINE_MASK;
    err |= (ctx << MERR_CTX_SHIFT) & MERR_CTX_MASK;
    err |= (uint64_t)errnum & MERR_ERRNO_MASK;

    return err;
}

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
merr_strerror(const merr_t err, char * const buf, size_t buf_sz, merr_stringify ctx_stringify)
{
    int ret = 0;
    size_t sz = 0;
    const char *file = NULL;
    const unsigned int ctx = merr_ctx(err);

    if (!buf && buf_sz > 0)
        buf_sz = 0;

    if (err) {
        file = merr_file(err);

        if (file) {
            ret = snprintf(buf, buf_sz, "%s:%d: ", file, merr_lineno(err));
            if (ret < 0) {
                sz = strlcpy(buf, "<failed to format the error message>", buf_sz);
                goto out;
            }

            sz += (size_t)ret;
        }

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
    } else {
        sz = strlcpy(buf, "success", buf_sz);
    }

out:
    return sz;
}
