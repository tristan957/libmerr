/* SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 * SPDX-FileCopyrightText: Copyright 2015 Micron Technology, Inc.
 * SPDX-FileCopyrightText: 2023 Tristan Partin <tristan@partin.io>
 */

#ifndef MERR_H
#define MERR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#ifndef __has_attribute
#error "__has_attribute must be provided by the compiler"
#endif

#if !__has_attribute(aligned)
#error "Copmiler must support the aligned attribute"
#endif

#if !__has_attribute(section)
#error "Compiler must support the section attribute"
#endif

#if __has_attribute(used)
#define MERR_USED __attribute__((used))
#else
#define MERR_USED
#endif

#if __has_attribute(warn_unused_result)
#define MERR_UNUSED_RESULT __attribute__((warn_unused_result))
#else
#define MERR_UNUSED_RESULT
#endif

#if __has_attribute(always_inline)
#define MERR_ALWAYS_INLINE inline __attribute__((always_inline))
#else
#define MERR_ALWAYS_INLINE inline
#endif

// Alignment of _merr_file in section "merr"
#define MERR_ALIGN (1 << 6)

#define _merr_section    __attribute__((section("merr")))
#define _merr_attributes _merr_section __attribute__((aligned(MERR_ALIGN)))

static char _merr_file[MERR_ALIGN] _merr_attributes MERR_USED = __BASE_FILE__;

/* Layout of merr_t:
 *
 *   Field   #bits  Description
 *   ------  -----  ----------
 *   63..48   16    signed offset of (_merr_file - merr_base) / MERR_ALIGN
 *   47..32   16    line number
 *   31..16   16    error context
 *   15..0    16    errno value
 */

#define MERR_FILE_SHIFT (48U)
#define MERR_LINE_SHIFT (32U)
#define MERR_CTX_SHIFT  (16U)

#define MERR_FILE_MASK  (0xffff000000000000ULL)
#define MERR_LINE_MASK  (0x0000ffff00000000ULL)
#define MERR_CTX_MASK   (0x00000000ffff0000ULL)
#define MERR_ERRNO_MASK (0x000000000000ffffULL)

/**
 * @brief The error value type.
 */
typedef uint64_t merr_t;

/**
 * @brief Function which returns a string given an error context.
 *
 * Returning NULL from your implementation will cause the context to not be
 * printed in merr_strerror().
 *
 * @param ctx Error context value.
 * @returns NULL-terminated statically allocated string.
 */
typedef const char *
merr_stringify(unsigned int ctx);

/**
 * @brief Pack given errno and call-site info into an merr_t.
 *
 * @param _errnum Errno value.
 * @returns An merr_t.
 */
#define merr(_errnum) merr_pack((_errnum), 0, _merr_file, __LINE__)

/**
 * @brief Pack given errno, error context, and call-site info into a merr_t.
 *
 * @param _errnum Errno value.
 * @returns An merr_t.
 */
#define merrx(_errnum, _ctx) merr_pack((_errnum), (_ctx), _merr_file, __LINE__)

/**
 * @brief Get the context of the error.
 *
 * @param err Error.
 * @returns File name.
 */
static MERR_ALWAYS_INLINE unsigned int MERR_USED MERR_UNUSED_RESULT
merr_ctx(const merr_t err)
{
    return (err & MERR_CTX_MASK) >> MERR_CTX_SHIFT;
}

/**
 * @brief Get @p errno value of error.
 *
 * @param err Error.
 * @returns @p Errno value.
 */
static MERR_ALWAYS_INLINE int MERR_USED MERR_UNUSED_RESULT
merr_errno(const merr_t err)
{
    return err & MERR_ERRNO_MASK;
}

/**
 * @brief Get file name error was generated in.
 *
 * @param err Error.
 * @returns File name.
 */
const char *
merr_file(merr_t err) MERR_UNUSED_RESULT;

/**
 * @brief Get line number error was generated on.
 *
 * @param err Error.
 * @returns Line number.
 */
static MERR_ALWAYS_INLINE unsigned int MERR_USED MERR_UNUSED_RESULT
merr_lineno(const merr_t err)
{
    return (err & MERR_LINE_MASK) >> MERR_LINE_SHIFT;
}

/**
 * @brief Format file, line, ctx, and errno from an merr_t into a buffer.
 *
 * @param err Error.
 * @param buf Buffer for storing the string.
 * @param buf_sz Size of @p buf.
 * @param ctx_stringify Function to turn a context into a string.
 * @returns Needed size of buffer.
 */
size_t
merr_strerror(merr_t err, char *buf, size_t buf_sz, merr_stringify ctx_stringify);

// This is not public API. DO NOT USE.
merr_t MERR_UNUSED_RESULT
merr_pack(int errnum, unsigned int ctx, const char *file, unsigned int line);

#ifdef __cplusplus
}
#endif

#endif
