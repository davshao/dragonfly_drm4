/*	$OpenBSD: i2c.h,v 1.6 2022/03/01 11:50:37 jsg Exp $	*/
/*
 * Copyright (c) 2017 Mark Kettenis
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
 * Copyright (c) 2013-2020 François Tigeot <ftigeot@wolfpond.org>
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

#ifndef _LINUX_I2C_H
#define _LINUX_I2C_H

#include <linux/mod_devicetable.h>
#include <linux/device.h>	/* for struct device */
#include <linux/sched.h>	/* for completion */
#include <linux/mutex.h>
#include <linux/of.h>		/* for struct device_node */
// #include <uapi/linux/i2c.h>

#include <bus/iicbus/iic.h>

struct i2c_algorithm;

#if defined(__OpenBSD__)

#define I2C_FUNC_I2C			0
#define I2C_FUNC_SMBUS_EMUL		0
#define I2C_FUNC_SMBUS_READ_BLOCK_DATA	0
#define I2C_FUNC_SMBUS_BLOCK_PROC_CALL	0
#define I2C_FUNC_10BIT_ADDR		0

#else

#define I2C_M_TEN		0x0010
#define I2C_M_RECV_LEN		0x0400
#define I2C_M_NO_RD_ACK		0x0800
#define I2C_M_IGNORE_NAK	0x1000
#define I2C_M_REV_DIR_ADDR	0x2000

#define I2C_FUNC_I2C			0x00000001
#define I2C_FUNC_10BIT_ADDR		0x00000002
#define I2C_FUNC_PROTOCOL_MANGLING	0x00000004
#define I2C_FUNC_SMBUS_PEC		0x00000008
#define I2C_FUNC_NOSTART		0x00000010
#define I2C_FUNC_SMBUS_BLOCK_PROC_CALL	0x00008000
#define I2C_FUNC_SMBUS_QUICK		0x00010000
#define I2C_FUNC_SMBUS_READ_BYTE	0x00020000
#define I2C_FUNC_SMBUS_WRITE_BYTE	0x00040000
#define I2C_FUNC_SMBUS_READ_BYTE_DATA	0x00080000
#define I2C_FUNC_SMBUS_WRITE_BYTE_DATA	0x00100000
#define I2C_FUNC_SMBUS_READ_WORD_DATA	0x00200000
#define I2C_FUNC_SMBUS_WRITE_WORD_DATA	0x00400000
#define I2C_FUNC_SMBUS_PROC_CALL	0x00800000
#define I2C_FUNC_SMBUS_READ_BLOCK_DATA	0x01000000
#define I2C_FUNC_SMBUS_WRITE_BLOCK_DATA 0x02000000
#define I2C_FUNC_SMBUS_READ_I2C_BLOCK	0x04000000
#define I2C_FUNC_SMBUS_WRITE_I2C_BLOCK	0x08000000

#define I2C_FUNC_SMBUS_BYTE		(I2C_FUNC_SMBUS_READ_BYTE | \
					 I2C_FUNC_SMBUS_WRITE_BYTE)

#define I2C_FUNC_SMBUS_BYTE_DATA	(I2C_FUNC_SMBUS_READ_BYTE_DATA | \
					 I2C_FUNC_SMBUS_WRITE_BYTE_DATA)

#define I2C_FUNC_SMBUS_WORD_DATA	(I2C_FUNC_SMBUS_READ_WORD_DATA | \
					 I2C_FUNC_SMBUS_WRITE_WORD_DATA)

#define I2C_FUNC_SMBUS_I2C_BLOCK	(I2C_FUNC_SMBUS_READ_I2C_BLOCK | \
					 I2C_FUNC_SMBUS_WRITE_I2C_BLOCK)

#define I2C_FUNC_SMBUS_EMUL		(I2C_FUNC_SMBUS_QUICK | \
					 I2C_FUNC_SMBUS_BYTE | \
					 I2C_FUNC_SMBUS_BYTE_DATA | \
					 I2C_FUNC_SMBUS_WORD_DATA | \
					 I2C_FUNC_SMBUS_PROC_CALL | \
					 I2C_FUNC_SMBUS_WRITE_BLOCK_DATA | \
					 I2C_FUNC_SMBUS_I2C_BLOCK | \
					 I2C_FUNC_SMBUS_PEC)

#define I2C_SMBUS_BLOCK_MAX	32

#endif

#define I2C_AQ_COMB			0
#define I2C_AQ_COMB_SAME_ADDR		0
#define I2C_AQ_NO_ZERO_LEN		0

struct i2c_lock_operations;

struct i2c_adapter_quirks {
	unsigned int flags;
	uint16_t max_read_len;
	uint16_t max_write_len;
	uint16_t max_comb_1st_msg_len;
	uint16_t max_comb_2nd_msg_len;
};

struct i2c_adapter {
	char name[48];
	const struct i2c_algorithm *algo;
	void *algo_data;

	int retries;
	const struct i2c_lock_operations *lock_ops;
	const struct i2c_adapter_quirks *quirks;
	void *data;
	int timeout;
	struct device dev;
	void *private_data;
};

struct i2c_lock_operations {
	void (*lock_bus)(struct i2c_adapter *, unsigned int);
	int (*trylock_bus)(struct i2c_adapter *, unsigned int);
	void (*unlock_bus)(struct i2c_adapter *, unsigned int);
};

#define I2C_NAME_SIZE	20

struct i2c_msg {
	uint16_t addr;
	uint16_t flags;
	uint16_t len;
	uint8_t *buf;
};

#if defined(__OpenBSD__)
#define I2C_M_RD	0x0001
#define I2C_M_NOSTART	0x0002
#define I2C_M_STOP	0x0004
#else
#define I2C_M_RD	IIC_M_RD
/* Not sure */
#define I2C_M_NOSTART	IIC_M_NOSTART
// #define I2C_M_STOP	IIC_M_NOSTART
#endif

struct i2c_client {
	unsigned short flags;
	unsigned short addr;
	char name[I2C_NAME_SIZE];
	struct i2c_adapter *adapter;
};

struct i2c_algorithm {
	int (*master_xfer)(struct i2c_adapter *, struct i2c_msg *, int);
	uint32_t (*functionality)(struct i2c_adapter *);
};

#if defined(__OpenBSD__)
extern       struct i2c_algorithm i2c_bit_algo;
#else
extern const struct i2c_algorithm i2c_bit_algo;
#endif

#if defined(__OpenBSD__)
struct i2c_algo_bit_data {
	struct i2c_controller ic;
};
#else
struct i2c_algo_bit_data {
	void *data;
	void (*setsda) (void *data, int state);
	void (*setscl) (void *data, int state);
	int  (*getsda) (void *data);
	int  (*getscl) (void *data);
	int  (*pre_xfer)  (struct i2c_adapter *);
	void (*post_xfer) (struct i2c_adapter *);

	int udelay;
	int timeout;
};
#endif

int __i2c_transfer(struct i2c_adapter *, struct i2c_msg *, int);
int i2c_transfer(struct i2c_adapter *, struct i2c_msg *, int);

extern int i2c_add_adapter(struct i2c_adapter *);
extern void i2c_del_adapter(struct i2c_adapter *);

static inline void *
i2c_get_adapdata(const struct i2c_adapter *adap)
{
	return adap->data;
}

static inline void
i2c_set_adapdata(struct i2c_adapter *adap, void *data)
{
	adap->data = data;
}

int i2c_bit_add_bus(struct i2c_adapter *);


struct i2c_board_info {
	char		type[I2C_NAME_SIZE];
	unsigned short	addr;
};

struct i2c_client *
i2c_new_device(struct i2c_adapter *adap, struct i2c_board_info const *info);

#define I2C_LOCK_SEGMENT      BIT(1)


#endif
