/* SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 * SPDX-FileCopyrightText: 2023 Tristan Partin <tristan@partin.io>
 */

#include <errno.h>

#include <merr.h>

#include "tester.h"

merr_t
tester_generate_err(void)
{
    return merr(ENOENT);
}
