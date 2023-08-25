<!--
SPDX-License-Identifier: Apache-2.0 OR MIT

SPDX-FileCopyrightText: 2023 Tristan Partin <tristan@partin.io>
-->

# `libmerr`

`libmerr` is a C99+ static library that is meant to be embedded in other
libraries or executables for the purposes of tracking error information.

C is a notoriously bad language when it comes to error handling. Many
system/POSIX/UNIX APIs will return various values on error or set
[`errno(3)`](https://linux.die.net/man/3/errno). An integer error value can
generally tell you the reason an error occurred, but it doesn't provide extra
information like file, line number, or any extra context.

To overcome this issue, the authors of the
[Heterogeneous-Memory Storage Engine](https://github.com/hse-project/hse) (HSE)
came up with `merr`. An `merr_t` is an error value which can encode all this
information in just a 64-bit unsigned integer. This repository is essentially a
copy and paste of what exists within HSE. Because of that you will see the
original `Micron Technology, Inc.` copyright in addition to the dual licensing
of this code as `Apache-2.0` or `MIT` depending on what best fits within your
constraints.

## Building/Installing

`libmerr` uses the [Meson](https://mesonbuild.com) build system.

```shell
meson setup build
meson compile -C build
meson install -C build
```

## Exposing `merr_t`

Exposing `merr_t` directly in public APIs can be an issue if your library can be
shared. If your library can only be consumed statically, you can ignore the
following paragraph, though it is still a good practice instead of making
`merr_t` viral. If you choose this route, remember to tell consumers of your
library to also depend on `libmerr`.

```meson
libmerr_dep = dependency('libmerr')
example_includes = include_directories('.')

lib = static_library(
  'example',
  'libexample.c',
  include_directions: example_includes,
  dependencies: libmerr_dep
)
lib_dep = declare_dependency(
  link_with: lib,
  include_directories: example_includes,
  dependencies: libmerr_dep
)

pkg = import('pkgconfig')
pkg.generate(lib, requires: 'libmerr')
```

---

The first thing a consumer should do is create their own error type. Your error
type must be compatible with `merr_t`. `libmerr` stores the type as a pkg-config
variable. Or you can just know that it is always an `int64_t`.

<!-- Keep the above type in sync! -->

The Meson way of retrieving the variable and using it:

```meson
libmerr_dep = dependency('libmerr')
merr_type = libmerr_dep.get_variable('merr_type')

configure_file(
  input: 'libexample.h.in',
  output: 'libexample.h',
  configuration: {
    'MERR_TYPE': merr_type,
  }
)
```

The command line way of retrieving the variable:

```shell
pkg-config --variable=merr_type libmerr
```

The C (or C++) boilerplate for creating your own error type should be something
like the following:

```c
// libexample.h

#include <stddef.h>
#include <stdint.h>

// You could just use int64_t here.
typedef @MERR_TYPE@ example_err_t;

int16_t
example_err_ctx(example_err_t err);

int
example_err_errno(example_err_t err);

const char *
example_err_file(example_err_t err);

uint16_t
example_err_lineno(example_err_t err);

size_t
example_err_strerror(example_err_t err, char *buf, size_t buf_sz);
```

```c
// libexample.c

#include <stddef.h>
#include <stdint.h>

#include <merr.h>

#include <libexample.h>

static const char *
ctx_strerror(const int ctx)
{
  switch (ctx) {
  case 0:
    /// ...
  }

  return NULL;
}

int16_t
example_err_ctx(example_err_t err)
{
  return merr_ctx(err);
}

int
example_err_errno(example_err_t err)
{
  return merr_errno(err);
}

const char *
example_err_file(example_err_t err)
{
  return merr_file(err);
}

uint16_t
example_err_lineno(example_err_t err)
{
  return merr_lineno(err);
}

size_t
example_err_strerror(example_err_t err, char *buf, size_t buf_sz)
{
  return merr_sterrorx(err, buf, buf_sz, ctx_strerror);
}
```

Remember that you can change return types in your functions. For instance, you
could return an `enum` instead of an `int16_t` for `example_err_ctx()`.

## Limits

`libmerr` cannot be used reliably if any of the following conditions are met:

- Source files longer than `UINT16_MAX` lines.
- More than 1024 files in the consuming project (This limit is dependent on max
  path length, `(1 << bit width of file offset) / MERR_MAX_PATH_LENGTH`).
- Paths longer than `MERR_MAX_PATH_LENGTH` characters (Recommended to use
  `-fmacro-prefix-map` or an equivalent).
- Error values other than those in `errno(3)`. This could be changed in the
  future to be generic if anyone ever wanted this capability.

## Warnings

- Do not try to use this library as a shared library. It will not work.
  - For Meson users, I highly suggest using a wrap file to integrate this into
    your project.

## Contributing

All commits must be signed (`git commit --signoff`) indicating that you agree to
the [Developer Certificate of Origin](http://developercertificate.org).
