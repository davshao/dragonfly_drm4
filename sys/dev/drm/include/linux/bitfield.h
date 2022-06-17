/* Public domain. */

#ifndef _LINUX_BITFIELD_H
#define _LINUX_BITFIELD_H

#if defined(__OpenBSD__)

#include <asm/byteorder.h>
#include <linux/build_bug.h>
#else
#include <machine/cpufunc.h>
#endif

#define __bf_shf(x) (__builtin_ffsll(x) - 1)

#define FIELD_GET(_m, _v) \
    ((typeof(_m))(((_v) & (_m)) >> __bf_shf(_m)))

#define FIELD_PREP(_m, _v) \
    (((typeof(_m))(_v) << __bf_shf(_m)) & (_m))

#endif
