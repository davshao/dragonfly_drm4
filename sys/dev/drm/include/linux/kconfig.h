/* Public domain. */

/*
 * Copyright (c) 2015-2019 François Tigeot <ftigeot@wolfpond.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice unmodified, this list of conditions, and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _LINUX_KCONFIG_H
#define _LINUX_KCONFIG_H

#include <sys/endian.h>

#include <generated/autoconf.h>

#if defined(__OpenBSD__)

#define __NEWARG1			__newarg,
#define __is_defined(x)			__is_defined2(x)
#define __is_defined2(x)		__is_defined3(__NEWARG##x)
#define __is_defined3(x)		__is_defined4(x 1, 0)
#define __is_defined4(a, b, ...)	b

#define IS_ENABLED(x)		__is_defined(x)
#define IS_REACHABLE(x)		__is_defined(x)
#define IS_BUILTIN(x)		__is_defined(x)
#define IS_MODULE(x)		0

#else

#define __kconfig_value1	1,

/*
 * Clever code to always return either 0 or 1 even if x is not defined
 */

#define IS_ENABLED(x)			IS_ENABLED2(x)
#define IS_ENABLED2(x)			IS_ENABLED3(__kconfig_value##x)
#define IS_ENABLED3(x)			IS_ENABLED4(x 1, 0)
#define IS_ENABLED4(one, two, ...)	two

#define config_enabled(x)		IS_ENABLED(x)

#define IS_REACHABLE(x)		IS_ENABLED(x)
#define IS_BUILTIN(x)		IS_ENABLED(x)
#define IS_MODULE(x)		0

#endif

#if defined(__OpenBSD__)

#if BYTE_ORDER == BIG_ENDIAN
#define __BIG_ENDIAN
#else
#define __LITTLE_ENDIAN
#endif

#else

/* Byteorder compat layer */
#if _BYTE_ORDER == _BIG_ENDIAN
#define	__BIG_ENDIAN 4321
#else
#define	__LITTLE_ENDIAN 1234
#endif

#endif

#endif
