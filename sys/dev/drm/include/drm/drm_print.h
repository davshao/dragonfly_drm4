/*
 * Copyright (C) 2016 Red Hat
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 * Rob Clark <robdclark@gmail.com>
 */

#ifndef DRM_PRINT_H_
#define DRM_PRINT_H_

#include <linux/compiler.h>
#include <linux/printk.h>
#include <linux/seq_file.h>
#include <linux/device.h>
#include <linux/debugfs.h>

#include <drm/drm.h>

#include <drm/drm_os_linux.h>

extern unsigned int drm_debug;

/* Do *not* use outside of drm_print.[ch]! */
extern unsigned int __drm_debug;

/**
 * DOC: print
 *
 * A simple wrapper for dev_printk(), seq_printf(), etc.  Allows same
 * debug code to be used for both debugfs and printk logging.
 *
 * For example::
 *
 *     void log_some_info(struct drm_printer *p)
 *     {
 *             drm_printf(p, "foo=%d\n", foo);
 *             drm_printf(p, "bar=%d\n", bar);
 *     }
 *
 *     #ifdef CONFIG_DEBUG_FS
 *     void debugfs_show(struct seq_file *f)
 *     {
 *             struct drm_printer p = drm_seq_file_printer(f);
 *             log_some_info(&p);
 *     }
 *     #endif
 *
 *     void some_other_function(...)
 *     {
 *             struct drm_printer p = drm_info_printer(drm->dev);
 *             log_some_info(&p);
 *     }
 */

/**
 * struct drm_printer - drm output "stream"
 *
 * Do not use struct members directly.  Use drm_printer_seq_file(),
 * drm_printer_info(), etc to initialize.  And drm_printf() for output.
 */
struct drm_printer {
	/* private: */
	void (*printfn)(struct drm_printer *p, struct va_format *vaf);
	void (*puts)(struct drm_printer *p, const char *str);
	void *arg;
	const char *prefix;
};

void __drm_printfn_coredump(struct drm_printer *p, struct va_format *vaf);
void __drm_puts_coredump(struct drm_printer *p, const char *str);
void __drm_printfn_seq_file(struct drm_printer *p, struct va_format *vaf);
void __drm_puts_seq_file(struct drm_printer *p, const char *str);
void __drm_printfn_info(struct drm_printer *p, struct va_format *vaf);
void __drm_printfn_debug(struct drm_printer *p, struct va_format *vaf);
void __drm_printfn_err(struct drm_printer *p, struct va_format *vaf);

__printf(2, 3)
void drm_printf(struct drm_printer *p, const char *f, ...);
void drm_puts(struct drm_printer *p, const char *str);
void drm_print_regset32(struct drm_printer *p, struct debugfs_regset32 *regset);
void drm_print_bits(struct drm_printer *p, unsigned long value,
		    const char * const bits[], unsigned int nbits);

__printf(2, 0)
/**
 * drm_vprintf - print to a &drm_printer stream
 * @p: the &drm_printer
 * @fmt: format string
 * @va: the va_list
 */
static inline void
drm_vprintf(struct drm_printer *p, const char *fmt, va_list *va)
{
	struct va_format vaf = { .fmt = fmt, .va = va };

	p->printfn(p, &vaf);
}

/**
 * drm_printf_indent - Print to a &drm_printer stream with indentation
 * @printer: DRM printer
 * @indent: Tab indentation level (max 5)
 * @fmt: Format string
 */
#define drm_printf_indent(printer, indent, fmt, ...) \
	drm_printf((printer), "%.*s" fmt, (indent), "\t\t\t\t\tX", ##__VA_ARGS__)

/**
 * struct drm_print_iterator - local struct used with drm_printer_coredump
 * @data: Pointer to the devcoredump output buffer
 * @start: The offset within the buffer to start writing
 * @remain: The number of bytes to write for this iteration
 */
struct drm_print_iterator {
	void *data;
	ssize_t start;
	ssize_t remain;
	/* private: */
	ssize_t offset;
};

/**
 * drm_coredump_printer - construct a &drm_printer that can output to a buffer
 * from the read function for devcoredump
 * @iter: A pointer to a struct drm_print_iterator for the read instance
 *
 * This wrapper extends drm_printf() to work with a dev_coredumpm() callback
 * function. The passed in drm_print_iterator struct contains the buffer
 * pointer, size and offset as passed in from devcoredump.
 *
 * For example::
 *
 *	void coredump_read(char *buffer, loff_t offset, size_t count,
 *		void *data, size_t datalen)
 *	{
 *		struct drm_print_iterator iter;
 *		struct drm_printer p;
 *
 *		iter.data = buffer;
 *		iter.start = offset;
 *		iter.remain = count;
 *
 *		p = drm_coredump_printer(&iter);
 *
 *		drm_printf(p, "foo=%d\n", foo);
 *	}
 *
 *	void makecoredump(...)
 *	{
 *		...
 *		dev_coredumpm(dev, THIS_MODULE, data, 0, GFP_KERNEL,
 *			coredump_read, ...)
 *	}
 *
 * RETURNS:
 * The &drm_printer object
 */
static inline struct drm_printer
drm_coredump_printer(struct drm_print_iterator *iter)
{
	struct drm_printer p = {
		.printfn = __drm_printfn_coredump,
		.puts = __drm_puts_coredump,
		.arg = iter,
	};

	/* Set the internal offset of the iterator to zero */
	iter->offset = 0;

	return p;
}

/**
 * drm_seq_file_printer - construct a &drm_printer that outputs to &seq_file
 * @f:  the &struct seq_file to output to
 *
 * RETURNS:
 * The &drm_printer object
 */
static inline struct drm_printer drm_seq_file_printer(struct seq_file *f)
{
	struct drm_printer p = {
		.printfn = __drm_printfn_seq_file,
		.puts = __drm_puts_seq_file,
		.arg = f,
	};
	return p;
}

/**
 * drm_info_printer - construct a &drm_printer that outputs to dev_printk()
 * @dev: the &struct device pointer
 *
 * RETURNS:
 * The &drm_printer object
 */
static inline struct drm_printer drm_info_printer(struct device *dev)
{
	struct drm_printer p = {
		.printfn = __drm_printfn_info,
		.arg = dev,
	};
	return p;
}

/**
 * drm_debug_printer - construct a &drm_printer that outputs to pr_debug()
 * @prefix: debug output prefix
 *
 * RETURNS:
 * The &drm_printer object
 */
static inline struct drm_printer drm_debug_printer(const char *prefix)
{
	struct drm_printer p = {
		.printfn = __drm_printfn_debug,
		.prefix = prefix
	};
	return p;
}

/**
 * drm_err_printer - construct a &drm_printer that outputs to pr_err()
 * @prefix: debug output prefix
 *
 * RETURNS:
 * The &drm_printer object
 */
static inline struct drm_printer drm_err_printer(const char *prefix)
{
	struct drm_printer p = {
		.printfn = __drm_printfn_err,
		.prefix = prefix
	};
	return p;
}

/*
 * The following categories are defined:
 *
 * CORE: Used in the generic drm code: drm_ioctl.c, drm_mm.c, drm_memory.c, ...
 *	 This is the category used by the DRM_DEBUG() macro.
 *
 * DRIVER: Used in the vendor specific part of the driver: i915, radeon, ...
 *	   This is the category used by the DRM_DEBUG_DRIVER() macro.
 *
 * KMS: used in the modesetting code.
 *	This is the category used by the DRM_DEBUG_KMS() macro.
 *
 * PRIME: used in the prime code.
 *	  This is the category used by the DRM_DEBUG_PRIME() macro.
 *
 * ATOMIC: used in the atomic code.
 *	  This is the category used by the DRM_DEBUG_ATOMIC() macro.
 *
 * VBL: used for verbose debug message in the vblank code
 *	  This is the category used by the DRM_DEBUG_VBL() macro.
 *
 * DragonFly-specific categories:
 *
 * PID: used as modifier to include PID number in messages.
 *	  This is the category used by the all debug macros.
 *
 * FIOCTL: used in failed ioctl debugging.
 *	  This is the category used by the DRM_DEBUG_FIOCTL() macro.
 *
 * IOCTL: used in ioctl debugging.
 *	  This is the category used by the DRM_DEBUG_IOCTL() macro.
 *
 * Enabling verbose debug messages is done through the drm.debug parameter,
 * each category being enabled by a bit.
 *
 * drm.debug=0x1 will enable CORE messages
 * drm.debug=0x2 will enable DRIVER messages
 * drm.debug=0x3 will enable CORE and DRIVER messages
 * ...
 * drm.debug=0x3f will enable all messages
 *
 * An interesting feature is that it's possible to enable verbose logging at
 * run-time by using the hw.drm.debug sysctl variable:
 *   # sysctl hw.drm.debug=0xfff
 */

/**
 * enum drm_debug_category - The DRM debug categories
 *
 * Each of the DRM debug logging macros use a specific category, and the logging
 * is filtered by the drm.debug module parameter. This enum specifies the values
 * for the interface.
 *
 * Each DRM_DEBUG_<CATEGORY> macro logs to DRM_UT_<CATEGORY> category, except
 * DRM_DEBUG() logs to DRM_UT_CORE.
 *
 * Enabling verbose debug messages is done through the drm.debug parameter, each
 * category being enabled by a bit:
 *
 *  - drm.debug=0x1 will enable CORE messages
 *  - drm.debug=0x2 will enable DRIVER messages
 *  - drm.debug=0x3 will enable CORE and DRIVER messages
 *  - ...
 *  - drm.debug=0x1ff will enable all messages
 *
 * An interesting feature is that it's possible to enable verbose logging at
 * run-time by echoing the debug value in its sysfs node::
 *
 *   # echo 0xf > /sys/module/drm/parameters/debug
 *
 */
enum drm_debug_category {
	DRM_UT_NONE		= 0x00,
	/**
	 * @DRM_UT_CORE: Used in the generic drm code: drm_ioctl.c, drm_mm.c,
	 * drm_memory.c, ...
	 */
	DRM_UT_CORE		= 0x01,
	/**
	 * @DRM_UT_DRIVER: Used in the vendor specific part of the driver: i915,
	 * radeon, ... macro.
	 */
	DRM_UT_DRIVER		= 0x02,
	/**
	 * @DRM_UT_KMS: Used in the modesetting code.
	 */
	DRM_UT_KMS		= 0x04,
	/**
	 * @DRM_UT_PRIME: Used in the prime code.
	 */
	DRM_UT_PRIME		= 0x08,
	/**
	 * @DRM_UT_ATOMIC: Used in the atomic code.
	 */
	DRM_UT_ATOMIC		= 0x10,
	/**
	 * @DRM_UT_VBL: Used for verbose debug message in the vblank code.
	 */
	DRM_UT_VBL		= 0x20,
	/**
	 * @DRM_UT_STATE: Used for verbose atomic state debugging.
	 */
	DRM_UT_STATE		= 0x40,
	/**
	 * @DRM_UT_LEASE: Used in the lease code.
	 */
	DRM_UT_LEASE		= 0x80,
	/**
	 * @DRM_UT_DP: Used in the DP code.
	 */
	DRM_UT_DP		= 0x100,
	/**
	 * @DRM_UT_DRMRES: Used in the drm managed resources code.
	 */
	DRM_UT_DRMRES		= 0x200,
/* Extra DragonFly debug categories */
	DRM_UT_PID		= 0x400,
	DRM_UT_FIOCTL		= 0x800,
	DRM_UT_IOCTL		= 0x1000,
};

static inline bool drm_debug_enabled(enum drm_debug_category category)
{
	return unlikely(__drm_debug & category);
}

/*
 * struct device based logging
 *
 * Prefer drm_device based logging over device or printk based logging.
 */
#ifdef __DragonFly__
extern __printf(2, 3)
void drm_ut_debug_printk(const char *function_name,
			 const char *format, ...);
extern __printf(2, 3)
void drm_err(const char *func, const char *format, ...);
#endif

#if defined(__OpenBSD__) /* declared in drm_drv.h different from this */
__printf(3, 4)
void drm_dev_printk(const struct device *dev, const char *level,
		    const char *format, ...);
#else
void drm_dev_printk(const struct device *dev, const char *level,
		    unsigned int category, const char *function_name,
		    const char *prefix, const char *format, ...);
void drm_printk(const char *level, unsigned int category,
		const char *format, ...);
#endif

__printf(3, 4)
void drm_dev_dbg(const struct device *dev, enum drm_debug_category category,
		 const char *format, ...);

/*
 * printk based logging
 *
 * Prefer drm_device based logging over device or prink based logging.
 */

__printf(2, 3)
void __drm_dbg(enum drm_debug_category category, const char *format, ...);
__printf(1, 2)
void __drm_err(const char *format, ...);

/* Macros to make printk easier */

#define _DRM_PRINTK(once, level, fmt, ...)				\
	printk##once(KERN_##level "[" DRM_NAME "] " fmt, ##__VA_ARGS__)

#define DRM_INFO(fmt, ...)						\
	_DRM_PRINTK(, INFO, fmt, ##__VA_ARGS__)
#define DRM_NOTE(fmt, ...)						\
	_DRM_PRINTK(, NOTICE, fmt, ##__VA_ARGS__)
#define DRM_WARN(fmt, ...)						\
	_DRM_PRINTK(, WARNING, fmt, ##__VA_ARGS__)

#define DRM_INFO_ONCE(fmt, ...)						\
	_DRM_PRINTK(_once, INFO, fmt, ##__VA_ARGS__)
#define DRM_NOTE_ONCE(fmt, ...)						\
	_DRM_PRINTK(_once, NOTICE, fmt, ##__VA_ARGS__)
#define DRM_WARN_ONCE(fmt, ...)						\
	_DRM_PRINTK(_once, WARNING, fmt, ##__VA_ARGS__)

/**
 * Error output.
 *
 * \param fmt printf() like format string.
 * \param arg arguments
 */
#define DRM_DEV_ERROR(dev, fmt, ...)					\
	drm_dev_printk(dev, KERN_ERR, DRM_UT_NONE, __func__, " *ERROR*",\
		       fmt, ##__VA_ARGS__)
#define DRM_ERROR(fmt, ...)						\
	drm_err(__func__, fmt, ##__VA_ARGS__)

/**
 * Rate limited error output.  Like DRM_ERROR() but won't flood the log.
 *
 * \param fmt printf() like format string.
 * \param arg arguments
 */
#define DRM_DEV_ERROR_RATELIMITED(dev, fmt, ...)			\
({									\
	static DEFINE_RATELIMIT_STATE(_rs,				\
				      DEFAULT_RATELIMIT_INTERVAL,	\
				      DEFAULT_RATELIMIT_BURST);		\
									\
	if (__ratelimit(&_rs))						\
		DRM_DEV_ERROR(dev, fmt, ##__VA_ARGS__);			\
})
#define DRM_ERROR_RATELIMITED(fmt, ...)					\
	DRM_DEV_ERROR_RATELIMITED(NULL, fmt, ##__VA_ARGS__)

#define DRM_DEV_INFO(dev, fmt, ...)					\
	drm_dev_printk(dev, KERN_INFO, DRM_UT_NONE, __func__, "", fmt,	\
		       ##__VA_ARGS__)

#define DRM_DEV_INFO_ONCE(dev, fmt, ...)				\
({									\
	static bool __print_once __read_mostly;				\
	if (!__print_once) {						\
		__print_once = true;					\
		DRM_DEV_INFO(dev, fmt, ##__VA_ARGS__);			\
	}								\
})

/**
 * Debug output.
 *
 * \param fmt printf() like format string.
 * \param arg arguments
 */
#define DRM_DEV_DEBUG(dev, fmt, args...)				\
	drm_dev_printk(dev, KERN_DEBUG, DRM_UT_CORE, __func__, "", fmt,	\
		       ##args)
#define DRM_DEBUG(fmt, args...)						\
	do {								\
		if (unlikely(drm_debug & DRM_UT_CORE))			\
			drm_ut_debug_printk(__func__, fmt, ##args);	\
	} while (0)

#define DRM_DEV_DEBUG_DRIVER(dev, fmt, args...)				\
	drm_dev_printk(dev, KERN_DEBUG, DRM_UT_DRIVER, __func__, "",	\
		       fmt, ##args)
#define DRM_DEBUG_DRIVER(fmt, args...)					\
	do {								\
		if (unlikely(drm_debug & DRM_UT_DRIVER))		\
			drm_ut_debug_printk(__func__, fmt, ##args);	\
	} while (0)

#define DRM_DEV_DEBUG_KMS(dev, fmt, args...)				\
	drm_dev_printk(dev, KERN_DEBUG, DRM_UT_KMS, __func__, "", fmt,	\
		       ##args)
#define DRM_DEBUG_KMS(fmt, args...)					\
	do {								\
		if (unlikely(drm_debug & DRM_UT_KMS))			\
			drm_ut_debug_printk(__func__, fmt, ##args);	\
	} while (0)

#define DRM_DEV_DEBUG_PRIME(dev, fmt, args...)				\
	drm_dev_printk(dev, KERN_DEBUG, DRM_UT_PRIME, __func__, "",	\
		       fmt, ##args)
#define DRM_DEBUG_PRIME(fmt, args...)					\
	do {								\
		if (unlikely(drm_debug & DRM_UT_PRIME))			\
			drm_ut_debug_printk(__func__, fmt, ##args);	\
	} while (0)

#define DRM_DEV_DEBUG_ATOMIC(dev, fmt, args...)				\
	drm_dev_printk(dev, KERN_DEBUG, DRM_UT_ATOMIC, __func__, "",	\
		       fmt, ##args)
#define DRM_DEBUG_ATOMIC(fmt, ...)						\
	do {									\
		if (unlikely(drm_debug & DRM_UT_ATOMIC))			\
			drm_ut_debug_printk(__func__, fmt, ##__VA_ARGS__);	\
	} while(0)

#define DRM_DEV_DEBUG_VBL(dev, fmt, args...)				\
	drm_dev_printk(dev, KERN_DEBUG, DRM_UT_VBL, __func__, "", fmt,	\
		       ##args)
#define DRM_DEBUG_VBL(fmt, args...)					\
	do {								\
		if (unlikely(drm_debug & DRM_UT_VBL))			\
			drm_ut_debug_printk(__func__, fmt, ##args);	\
	} while (0)

#ifdef __DragonFly__
#define DRM_DEBUG_FIOCTL(fmt, args...)					\
	do {								\
		if (unlikely(drm_debug & DRM_UT_FIOCTL))		\
			drm_ut_debug_printk(__func__, fmt, ##args);	\
	} while (0)
#define DRM_DEBUG_IOCTL(fmt, args...)					\
	do {								\
		if (unlikely(drm_debug & DRM_UT_IOCTL))			\
			drm_ut_debug_printk(__func__, fmt, ##args);	\
	} while (0)
#define DRM_DEBUG_VBLANK	DRM_DEBUG_VBL
#endif	/* __DragonFly__ */

#define DRM_DEBUG_LEASE(fmt, ...)					\
	drm_printk(KERN_DEBUG, DRM_UT_LEASE, fmt, ##__VA_ARGS__)

#define _DRM_DEV_DEFINE_DEBUG_RATELIMITED(dev, level, fmt, args...)	\
({									\
	static DEFINE_RATELIMIT_STATE(_rs,				\
				      DEFAULT_RATELIMIT_INTERVAL,	\
				      DEFAULT_RATELIMIT_BURST);		\
	if (__ratelimit(&_rs))						\
		drm_dev_printk(dev, KERN_DEBUG, DRM_UT_ ## level,	\
			       __func__, "", fmt, ##args);		\
})

/**
 * Rate limited debug output. Like DRM_DEBUG() but won't flood the log.
 *
 * \param fmt printf() like format string.
 * \param arg arguments
 */
#define DRM_DEV_DEBUG_RATELIMITED(dev, fmt, args...)			\
	DEV__DRM_DEFINE_DEBUG_RATELIMITED(dev, CORE, fmt, ##args)
#define DRM_DEBUG_RATELIMITED(fmt, args...)				\
	DRM_DEV_DEBUG_RATELIMITED(NULL, fmt, ##args)
#define DRM_DEV_DEBUG_DRIVER_RATELIMITED(dev, fmt, args...)		\
	_DRM_DEV_DEFINE_DEBUG_RATELIMITED(dev, DRIVER, fmt, ##args)
#define DRM_DEBUG_DRIVER_RATELIMITED(fmt, args...)			\
	DRM_DEV_DEBUG_DRIVER_RATELIMITED(NULL, fmt, ##args)
#define DRM_DEV_DEBUG_KMS_RATELIMITED(dev, fmt, args...)		\
	_DRM_DEV_DEFINE_DEBUG_RATELIMITED(dev, KMS, fmt, ##args)
#define DRM_DEBUG_KMS_RATELIMITED(fmt, args...)				\
	DRM_DEV_DEBUG_KMS_RATELIMITED(NULL, fmt, ##args)
#define DRM_DEV_DEBUG_PRIME_RATELIMITED(dev, fmt, args...)		\
	_DRM_DEV_DEFINE_DEBUG_RATELIMITED(dev, PRIME, fmt, ##args)
#define DRM_DEBUG_PRIME_RATELIMITED(fmt, args...)			\
	DRM_DEV_DEBUG_PRIME_RATELIMITED(NULL, fmt, ##args)

#endif /* DRM_PRINT_H_ */
