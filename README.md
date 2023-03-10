<!--
SPDX-License-Identifier: Apache-2.0 OR MIT

SPDX-FileCopyrightText: 2023 Tristan Partin <tristan@partin.io>
-->

# `libmerr`

`libmerr` is a C99 static library that is meant to be embedded in other
libraries or executables for the purposes of tracking error information.

C is a notoriously bad language when it comes to error handling. Many
system/POSIX/UNIX APIs will return various values on error or set `errno(3)`. An
integer error value can generally tell you the reason an error occurred, but it
doesn't provide extra information like file, line number, or any extra context.

To overcome this issue, the authors of the
[Heterogeneous-Memory Storage Engine](https://github.com/hse-project/hse) (HSE)
came up with `merr`. An `merr_t` is an error value which can encode all this
information in just a 64-bit unsigned integer. This repository is essentially a
copy and paste of what exists within HSE. Because of that you will see the
original Micron copyright in addition to the dual licensing of this code as
`Apache-2.0` or `MIT` depending on what best fits within your constraints.

## Building/Installing

`libmerr` uses the [Meson](https://mesonbuild.com) build system. `libmerr` takes
one dependency on `libbsd` for `strlcpy(3)` if the function declaration is not
found in `string.h`. If `libbsd` doesn't exist on your system, then Meson will
build it for you as a part of the `libmerr` build.

```shell
meson setup build
meson compile -C build
meson install -C build
```

## Warnings

- Do not try to use this library as a shared library. It will not work.
  - For Meson users, I highly suggest using a wrap file to integrate this into
    your project.
- `merr` wraps `errno(3)` values. This could be changed in the future to be
  generic if anyone ever wanted this capability.

## Contributing

All commits must be signed (`git commit --signoff`) indicating that you agree to
the [Developer Certificate of Origin](http://developercertificate.org).
