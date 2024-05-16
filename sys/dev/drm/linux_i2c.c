/*
 * Copyright (c) 2016-2020 François Tigeot <ftigeot@wolfpond.org>
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

#include <linux/i2c.h>
#include <linux/i2c-algo-bit.h>

#include <linux/slab.h>

static struct lock i2c_lock;
LOCK_SYSINIT(i2c_lock, &i2c_lock, "i2cl", LK_CANRECURSE);

static void i2c_default_lock_bus(struct i2c_adapter *, unsigned int);
static int i2c_default_trylock_bus(struct i2c_adapter *, unsigned int);
static void i2c_default_unlock_bus(struct i2c_adapter *, unsigned int);

static const struct i2c_lock_operations i2c_default_lock_ops = {
	.lock_bus =    i2c_default_lock_bus,
	.trylock_bus = i2c_default_trylock_bus,
	.unlock_bus =  i2c_default_unlock_bus,
};

int
i2c_add_adapter(struct i2c_adapter *adapter)
{
	/* Linux registers a unique bus number here */

	/* Setup default locking functions */
	if (!adapter->lock_ops)
		adapter->lock_ops = &i2c_default_lock_ops;

	return 0;
}

void
i2c_del_adapter(struct i2c_adapter *adapter)
{
	/* Linux deletes a unique bus number here */
}

int
__i2c_transfer(struct i2c_adapter *adapter, struct i2c_msg *msgs, int num)
{
#if defined(__OpenBSD__) /* from drm_linux.c */
	int ret, tries;

	tries = adapter->retries;
retry:
	if (adapter->algo)
		ret = adapter->algo->master_xfer(adapter, msgs, num);
	else
		ret = i2c_master_xfer(adapter, msgs, num);
	if (ret == -EAGAIN && tries > 0) {
		tries--;
		goto retry;
	}
#else
	uint64_t start_ticks;
	int ret, tries = 0;

	start_ticks = ticks;
	do {
		ret = adapter->algo->master_xfer(adapter, msgs, num);
		if (ticks > start_ticks + adapter->timeout)
			break;
		if (ret != -EAGAIN)
			break;
		tries++;
	} while (tries < adapter->retries);
#endif

	return ret;
}

/*
 * i2c_transfer()
 * The original Linux implementation does:
 * 1. return -EOPNOTSUPP if adapter->algo->master_xfer is NULL
 * 2. try to transfer msgs by calling adapter->algo->master_xfer()
 * 3. if it took more ticks than adapter->timeout, fail
 * 4. if the transfer failed, retry up to adapter->retries times
 * 5. return the result of the last call of adapter->algo->master_xfer()
 */
int
i2c_transfer(struct i2c_adapter *adapter, struct i2c_msg *msgs, int num)
{
	int ret;

#if defined(__OpenBSD__)
	if (adapter->lock_ops)
		adapter->lock_ops->lock_bus(adapter, 0);

	ret = __i2c_transfer(adapter, msgs, num);

	if (adapter->lock_ops)
		adapter->lock_ops->unlock_bus(adapter, 0);
#else
	if (adapter->algo->master_xfer == NULL)
		return -EOPNOTSUPP;

	adapter->lock_ops->lock_bus(adapter, I2C_LOCK_SEGMENT);
	ret = __i2c_transfer(adapter, msgs, num);
	adapter->lock_ops->unlock_bus(adapter, I2C_LOCK_SEGMENT);
#endif

	return ret;
}

static int
bit_xfer(struct i2c_adapter *i2c_adap, struct i2c_msg msgs[], int num)
{
	/* XXX Linux really does try to transfer some data here */
	return 0;
}

static uint32_t
bit_func(struct i2c_adapter *adap)
{
	return (I2C_FUNC_I2C | I2C_FUNC_NOSTART | I2C_FUNC_SMBUS_EMUL |
		I2C_FUNC_SMBUS_READ_BLOCK_DATA |
		I2C_FUNC_SMBUS_BLOCK_PROC_CALL |
		I2C_FUNC_10BIT_ADDR | I2C_FUNC_PROTOCOL_MANGLING);
}

const struct i2c_algorithm i2c_bit_algo = {
	.master_xfer	= bit_xfer,
	.functionality	= bit_func,
};

int
i2c_bit_add_bus(struct i2c_adapter *adapter)
{
	adapter->algo = &i2c_bit_algo;
#if defined(__OpenBSD__)
	adapter->retries = 3;
#else
	adapter->retries = 2;
#endif

	return 0;
}

struct i2c_client *
i2c_new_device(struct i2c_adapter *adap, struct i2c_board_info const *info)
{
	struct i2c_client *client;

	client = kzalloc(sizeof(*client), GFP_KERNEL);
	if (client == NULL)
		goto done;

	client->adapter = adap;

	strlcpy(client->name, info->type, sizeof(client->name));
	client->addr = info->addr;

done:
	return client;
}

/* Default locking functions */

static void
i2c_default_lock_bus(struct i2c_adapter *adapter, unsigned int flags)
{
	lockmgr(&i2c_lock, LK_EXCLUSIVE);
}

static int
i2c_default_trylock_bus(struct i2c_adapter *adapter, unsigned int flags)
{
	return mutex_trylock(&i2c_lock);
}

static void
i2c_default_unlock_bus(struct i2c_adapter *adapter, unsigned int flags)
{
	lockmgr(&i2c_lock, LK_RELEASE);
}
