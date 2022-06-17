/*	$OpenBSD: uaccess.h,v 1.7 2022/02/01 04:09:14 jsg Exp $	*/
/*
 * Copyright (c) 2015 Mark Kettenis
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * Copyright (c) 2015 François Tigeot
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

#ifndef _LINUX_UACCESS_H
#define _LINUX_UACCESS_H

#include <sys/param.h>
#include <sys/systm.h>
// #include <asm/uaccess.h>
#if defined(__OpenBSD__)
#include <uvm/uvm_extern.h>
#else
#include <vm/vm_page.h>
#endif

#include <linux/sched.h>

static inline unsigned long 
__copy_to_user(void *to, const void *from, unsigned long len)
{
	if (copyout(from, to, len))
		return len;
	return 0;
}

static inline unsigned long
copy_to_user(void *to, const void *from, unsigned long len)
{
	return __copy_to_user(to, from, len);
}

static inline unsigned long
__copy_from_user(void *to, const void *from, unsigned long len)
{
	if (copyin(from, to, len))
		return len;
	return 0;
}

static inline unsigned long
copy_from_user(void *to, const void *from, unsigned long len)
{
	return __copy_from_user(to, from, len);
}

/* function prototypes previously had __user in DragonFly */
static inline unsigned long 
__copy_from_user_inatomic(void *dst, const void *src, unsigned long len);

static inline unsigned long 
__copy_to_user_inatomic(void *to, const void *from, unsigned long len);


#if defined(__OpenBSD__)
#define get_user(x, ptr)	-copyin(ptr, &(x), sizeof(x))
#else
#define get_user(x, ptr)	-copyin(ptr, &(x), sizeof(*(ptr)))
#endif

#if defined(__OpenBSD__)
#define put_user(x, ptr) ({				\
	__typeof((x)) __tmp = (x);			\
	-copyout(&(__tmp), ptr, sizeof(__tmp));		\
})
#else /* not sure but API says 0 or -EFAULT on return values */
#define put_user(x, ptr)						\
({									\
	__typeof(*(ptr)) __tmp = (x);					\
	__copy_to_user_inatomic(ptr, &(__tmp), sizeof(__tmp));	\
})
#endif
#define __get_user(x, ptr)	get_user((x), (ptr))
#define __put_user(x, ptr)	put_user((x), (ptr))

#if defined(__OpenBSD__)
#define unsafe_put_user(x, ptr, err) ({				\
	__typeof((x)) __tmp = (x);				\
	if (copyout(&(__tmp), ptr, sizeof(__tmp)) != 0)		\
		goto err;					\
})
#else
#define unsafe_put_user(x, ptr, err) do { \
	if (unlikely(__put_user(x, ptr))) \
		goto err; \
} while (0)
#endif

/* not sure, ported from OpenBSD */
static inline int
access_ok(const void *addr, unsigned long size)
{
#if defined(__OpenBSD__)
	vaddr_t startva = (vaddr_t)addr;
	vaddr_t endva = ((vaddr_t)addr) + size;
	return (startva >= VM_MIN_ADDRESS && endva >= VM_MIN_ADDRESS) &&
	    (startva <= VM_MAXUSER_ADDRESS && endva <= VM_MAXUSER_ADDRESS);
#else /* not sure */
	vm_offset_t startva = (vm_offset_t)addr;
	vm_offset_t endva = ((vm_offset_t)addr) + size;
	return (startva >= VM_MIN_ADDRESS && endva >= VM_MIN_ADDRESS) &&
	    (startva <= VM_MAX_USER_ADDRESS && endva <= VM_MAX_USER_ADDRESS);
#endif
}

/* decision required API question */
#if defined(__OpenBSD__)
#define user_access_begin(addr, size)	access_ok(addr, size)
#else
#define user_access_begin()
#endif
#define user_access_end()

/* not sure*/
#define user_write_access_begin(addr, size)	access_ok(addr, size)
#define user_write_access_end()

#if defined(__i386__) || defined(__amd64__)

static inline void
pagefault_disable(void)
{
#if defined(__OpenBSD__)
	curcpu()->ci_inatomic++;
	KASSERT(curcpu()->ci_inatomic > 0);
#else
	atomic_set_int(&curthread->td_flags, TDF_NOFAULT);
#endif
}

static inline void
pagefault_enable(void)
{
#if defined(__OpenBSD__)
	KASSERT(curcpu()->ci_inatomic > 0);
	curcpu()->ci_inatomic--;
#else
	atomic_clear_int(&curthread->td_flags, TDF_NOFAULT);
#endif
}

static inline int
pagefault_disabled(void)
{
#if defined(__OpenBSD__)
	return curcpu()->ci_inatomic;
#else
	return (curthread->td_flags & TDF_NOFAULT);
#endif
}

static inline unsigned long 
__copy_to_user_inatomic(void *to, const void *from, unsigned long len)
{
#if defined(__OpenBSD__)
	struct cpu_info *ci = curcpu();
	int inatomic = ci->ci_inatomic;
	int error;

	ci->ci_inatomic = 1;
	error = copyout(from, to, len);
	ci->ci_inatomic = inatomic;

	return (error ? len : 0);
#else
	return (copyout_nofault(from, to, len) != 0 ? len : 0);
#endif
}

static inline unsigned long 
__copy_from_user_inatomic(void *to, const void *from, unsigned long len)
{
#if defined(__OpenBSD__)
	struct cpu_info *ci = curcpu();
	int inatomic = ci->ci_inatomic;
	int error;

	ci->ci_inatomic = 1;
	error = copyin(from, to, len);
	ci->ci_inatomic = inatomic;

	return (error ? len : 0);
#else
	if (copyin_nofault(from, to, len))
		return len;
	return 0;
#endif
}

static inline unsigned long
__copy_from_user_inatomic_nocache(void *to, const void *from, unsigned long len)
{
#if defined(__OpenBSD__)
	return __copy_from_user_inatomic(to, from, len);
#else
	/*
	 * XXXKIB.  Equivalent Linux function is implemented using
	 * MOVNTI for aligned moves.  For unaligned head and tail,
	 * normal move is performed.  As such, it is not incorrect, if
	 * only somewhat slower, to use normal copyin.  All uses
	 * except shmem_pwrite_fast() have the destination mapped WC.
	 */
	return ((copyin_nofault(__DECONST(void *, from), to, len) != 0 ? len : 0));
#endif
}

#endif /* defined(__i386__) || defined(__amd64__) */

#endif
