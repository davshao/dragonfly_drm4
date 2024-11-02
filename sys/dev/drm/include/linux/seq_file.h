/* Public domain. */

/*
 * Copyright (c) 2015 Michael Neumann
 * Copyright (c) 2020 François Tigeot <ftigeot@wolfpond.org>
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

#ifndef _LINUX_SEQ_FILE_H
#define _LINUX_SEQ_FILE_H

// #include <sys/sbuf.h>

#include <linux/types.h>
#include <linux/bug.h>
#include <linux/string.h>
// #include <linux/mutex.h>
// #include <linux/cpumask.h>

struct seq_file {
	char	*buf;
	size_t	size;
};

// void seq_printf(struct seq_file *m, const char *f, ...);

#if defined(__OpenBSD__)
static inline void
seq_printf(struct seq_file *m, const char *fmt, ...) {};
 
static inline void
seq_puts(struct seq_file *m, const char *s) {};
#else
static inline void
seq_printf(struct seq_file *m, const char *fmt, ...)
{
	__va_list ap;

	__va_start(ap, fmt);
	ksnprintf(m->buf, m->size, fmt, ap);
	__va_end(ap);
}

#define seq_puts(m, str)	ksnprintf((m)->buf, (m)->size, str)
#endif

#endif
