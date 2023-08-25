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

#if __has_attribute(always_inline)
#define MERR_ALWAYS_INLINE inline __attribute__((always_inline))
#else
#define MERR_ALWAYS_INLINE inline
#endif

#if __has_attribute(const)
#define MERR_CONST __attribute__((const))
#else
#define MERR_CONST
#endif

#if __has_attribute(used)
#define MERR_USED __attribute__((used))
#else
#define MERR_USED
#endif

#if __has_attribute(warn_unused_result)
#define MERR_WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#else
#define MERR_WARN_UNUSED_RESULT
#endif

// Alignment of merr_curr_file in section "merr"
#define MERR_MAX_PATH_LENGTH (1U << 6)

#define merr_attributes __attribute__((section("merr"))) __attribute__((aligned(MERR_MAX_PATH_LENGTH)))

static char merr_curr_file[MERR_MAX_PATH_LENGTH] merr_attributes MERR_USED = __BASE_FILE__;

/* Layout of merr_t:
 *
 *   Field   #bits  Description
 *   ------  -----  ----------
 *   63..48   16    signed offset of (merr_curr_file - merr_base) / MERR_MAX_PATH_LENGTH
 *   47..32   16    line number
 *   31..16   16    context
 *   15..0    16    error value
 */

#define MERR_FILE_SHIFT 48
#define MERR_LINE_SHIFT 32
#define MERR_CTX_SHIFT  16

#define MERR_FILE_MASK  0xffff000000000000LL
#define MERR_LINE_MASK  0x0000ffff00000000LL
#define MERR_CTX_MASK   0x00000000ffff0000LL
#define MERR_ERRNO_MASK 0x000000000000ffffLL

/**
 * @brief The error value type.
 */
typedef int64_t merr_t;

/**
 * @brief Function which returns a string given a number.
 *
 * Returning NULL from your implementation will cause the string to not be
 * printed in merr_strerror().
 *
 * @param num Error number.
 * @returns NULL-terminated statically allocated string.
 */
typedef const char *
merr_stringify(int num);

/**
 * @brief Helper macro for casting a function to merr_stringify.
 *
 * @param _func Stringify function.
 */
#define MERR_STRINGIFY(_func) (merr_stringify)(_func)

/**
 * @brief Pack given error number and call-site info into an merr_t.
 *
 * @param _errnum Error number.
 * @returns An merr_t.
 */
#define merr(_errnum) merrx((_errnum), 0)

/**
 * @brief Pack given error number, call-site info, and context into an merr_t.
 *
 * @param _errnum Error number.
 * @param _ctx Context.
 * @returns An merr_t.
 */
#define merrx(_errnum, _ctx) merr_pack((_errnum), (_ctx), merr_curr_file, __LINE__)

/**
 * @brief Get the context.
 *
 * @param err Error.
 * @returns Error context.
 */
static MERR_ALWAYS_INLINE int16_t MERR_CONST MERR_USED MERR_WARN_UNUSED_RESULT
merr_ctx(const merr_t err)
{
    return (int16_t)((err & MERR_CTX_MASK) >> MERR_CTX_SHIFT);
}

/**
 * @brief Get the error value.
 *
 * @param err Error.
 * @returns Error number.
 */
static MERR_ALWAYS_INLINE int MERR_CONST MERR_USED MERR_WARN_UNUSED_RESULT
merr_errno(const merr_t err)
{
    return err & MERR_ERRNO_MASK;
}

/**
 * @brief Get the file name the error was generated in.
 *
 * @warning If file names were too long during compilation, it is possible that
 *          this string is not NUL-terminated.
 *
 * @param err Error.
 * @returns File name.
 */
const char *
merr_file(merr_t err) MERR_CONST MERR_WARN_UNUSED_RESULT;

/**
 * @brief Get the line number the error was generated on.
 *
 * @param err Error.
 * @returns Line number.
 */
static MERR_ALWAYS_INLINE uint16_t MERR_CONST MERR_USED MERR_WARN_UNUSED_RESULT
merr_lineno(const merr_t err)
{
    return (uint16_t)((err & MERR_LINE_MASK) >> MERR_LINE_SHIFT);
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
merr_strerrorx(merr_t err, char *buf, size_t buf_sz, merr_stringify ctx_stringify);

/**
 * @brief Format file, line, and errno from an merr_t into a buffer.
 *
 * @param err Error.
 * @param buf Buffer for storing the string.
 * @param buf_sz Size of @p buf.
 * @returns Needed size of buffer.
 */
#define merr_strerror(_err, _buf, _buf_sz) merr_strerrorx((_err), (_buf), (_buf_sz), NULL)

// This is not public API. DO NOT USE.
merr_t
merr_pack(int errnum, int ctx, const char *file, uint16_t line) MERR_CONST MERR_WARN_UNUSED_RESULT;

#ifdef __cplusplus
}
#endif

#endif
