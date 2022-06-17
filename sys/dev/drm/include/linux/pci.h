/*	$OpenBSD: pci.h,v 1.12 2022/10/03 10:07:01 jsg Exp $	*/
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
 * Copyright (c) 2014-2020 François Tigeot <ftigeot@wolfpond.org>
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

/* For code from FreeBSD sys/compat/linuxkpi/common/include/linux/pci.h */
/*-
 * Copyright (c) 2010 Isilon Systems, Inc.
 * Copyright (c) 2010 iX Systems, Inc.
 * Copyright (c) 2010 Panasas, Inc.
 * Copyright (c) 2013-2016 Mellanox Technologies, Ltd.
 * All rights reserved.
 * Copyright (c) 2020-2022 The FreeBSD Foundation
 *
 * Portions of this software were developed by Björn Zeeb
 * under sponsorship from the FreeBSD Foundation.
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
 *
 * $FreeBSD$
 */


#ifndef _LINUX_PCI_H_
#define _LINUX_PCI_H_

#include <sys/types.h>
/* sparc64 cpu.h needs time.h and siginfo.h (indirect via param.h) */
#include <sys/param.h>
#include <machine/cpu.h>

#include <sys/pciio.h>
#include <sys/rman.h>
#include <bus/pci/pcivar.h>
#include <bus/pci/pcireg.h>

#include <linux/mod_devicetable.h>

#include <linux/types.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/list.h>
#include <linux/compiler.h>
#include <linux/errno.h>
#include <linux/kobject.h>
#include <linux/atomic.h>
#include <linux/device.h>
#include <linux/io.h>
// #include <uapi/linux/pci.h>

#include <linux/pci_ids.h>
#include <linux/pci_regs.h>

struct pci_dev;
struct pci_bus;
struct device_node;

struct pci_bus {
	struct pci_dev *self;		/* handle to pdev self */
	struct device *dev;		/* handle to dev */

	unsigned char number;		/* bus addr number */

	unsigned char max_bus_speed;
};

#if defined(__OpenBSD__)
struct pci_acpi {
	struct aml_node	*node;
};
#endif

typedef unsigned short pci_dev_flags_t;

#define PCI_DEV_FLAGS_NEEDS_RESUME	(1 << 11)

struct pci_dev {
	struct pci_bus *bus;		/* bus device is nailed to */
	struct device dev;

	uint32_t devfn;
	uint16_t vendor;		/* vendor ID */
	uint16_t device;		/* device ID */
	uint16_t subsystem_vendor;
	uint16_t subsystem_device;

	uint8_t revision;		/* revision ID */

	unsigned int irq;		/* handle with care */
	void *pci_dev_data;

	unsigned int	no_64bit_msi:1;
	pci_dev_flags_t dev_flags;

	/* DragonFly-specific data */
	int		_irq_type;
	struct resource	*_irqr;
	int		_irqrid;
};
#define PCI_ANY_ID	(~0u)

#ifndef PCI_MEM_START
#define PCI_MEM_START	0
#endif

#ifndef PCI_MEM_END
#define PCI_MEM_END	0xffffffff
#endif

#ifndef PCI_MEM64_END
#define PCI_MEM64_END	0xffffffffffffffff
#endif

struct pci_driver {
	const char *name;
	const struct pci_device_id *id_table;
	int (*probe)(struct pci_dev *dev, const struct pci_device_id *id);
	void (*remove)(struct pci_dev *dev);
};

#if defined(__OpenBSD__)
#define PCI_VENDOR_ID_APPLE	PCI_VENDOR_APPLE
#define PCI_VENDOR_ID_ASUSTEK	PCI_VENDOR_ASUSTEK
#define PCI_VENDOR_ID_ATI	PCI_VENDOR_ATI
#define PCI_VENDOR_ID_DELL	PCI_VENDOR_DELL
#define PCI_VENDOR_ID_HP	PCI_VENDOR_HP
#define PCI_VENDOR_ID_IBM	PCI_VENDOR_IBM
#define PCI_VENDOR_ID_INTEL	PCI_VENDOR_INTEL
#define PCI_VENDOR_ID_SONY	PCI_VENDOR_SONY
#define PCI_VENDOR_ID_VIA	PCI_VENDOR_VIATECH
#endif

#if defined(__OpenBSD__)
#define PCI_DEVICE_ID_ATI_RADEON_QY	PCI_PRODUCT_ATI_RADEON_QY
#else /* previous radeon_bios.c */
#define PCI_DEVICE_ID_ATI_RADEON_QY	0x5159
#endif

#define PCI_SUBVENDOR_ID_REDHAT_QUMRANET	0x1af4
#define PCI_SUBDEVICE_ID_QEMU			0x1100

#if defined(__OpenBSD__)
#define PCI_DEVFN(slot, func)	((slot) << 3 | (func))
#define PCI_SLOT(devfn)		((devfn) >> 3)
#define PCI_FUNC(devfn)		((devfn) & 0x7)
#define PCI_BUS_NUM(devfn)	(((devfn) >> 8) & 0xff)
#else
#define PCI_DEVFN(slot, func)	((((slot) & 0x1f) << 3) | ((func) & 0x07))
#define PCI_SLOT(devfn)		(((devfn) >> 3) & 0x1f)
#define PCI_FUNC(devfn)		((devfn) & 0x07)
/* return bus from PCI devid = ((u16)bus_number) << 8) | devfn */
#define PCI_BUS_NUM(x) (((x) >> 8) & 0xff)
#endif

#if defined(__OpenBSD__)
#define pci_dev_put(x)
#else
static inline struct pci_dev *
pci_dev_put(struct pci_dev *dev)
{
	/* Linux decrements a reference count here */
	return dev;
}
#endif

#define PCI_DMA_BIDIRECTIONAL	0

/* extracted from radeon/si.c radeon/cik.c */
#define PCI_EXP_DEVSTA		PCIER_DEVSTS /* 10 */
#define PCI_EXP_DEVSTA_TRPND	0x0020
// uapi/linux/pci_regs.h
// #define PCI_EXP_LNKCAP		0x0c
#define PCI_EXP_LNKCAP_CLKPM	0x00040000
#define PCI_EXP_LNKCTL		0x10
#define PCI_EXP_LNKCTL_HAWD	0x0200
#define PCI_EXP_LNKCTL2		0x30
#define PCI_EXP_LNKCTL2_ENTER_COMP	0x0010
#define PCI_EXP_LNKCTL2_TX_MARGIN	0x0380
#if defined(__OpenBSD__)
#define PCI_EXP_LNKCTL2_TLS		PCI_PCIE_LCSR2_TLS
#define PCI_EXP_LNKCTL2_TLS_2_5GT	PCI_PCIE_LCSR2_TLS_2_5
#define PCI_EXP_LNKCTL2_TLS_5_0GT	PCI_PCIE_LCSR2_TLS_5
#define PCI_EXP_LNKCTL2_TLS_8_0GT	PCI_PCIE_LCSR2_TLS_8
#else /* OpenBSD sys/dev/pci/pcireg.h */
#define PCI_EXP_LNKCTL2_TLS		0x0000000f
#define PCI_EXP_LNKCTL2_TLS_2_5GT	0x00000001
#define PCI_EXP_LNKCTL2_TLS_5_0GT	0x00000002
#define PCI_EXP_LNKCTL2_TLS_8_0GT	0x00000003
#endif

#if defined(__OpenBSD__)
#define PCI_COMMAND		PCI_COMMAND_STATUS_REG
#define PCI_COMMAND_MEMORY	PCI_COMMAND_MEM_ENABLE
#else /* OpenBSD sys/dev/pci/pcireg.h */
#define PCI_COMMAND		0x04
#define PCI_COMMAND_MEMORY	0x00000002	
#endif

#if defined (__DragonFly__)
#include <uapi/linux/pci_regs.h>
#endif

static inline int
pci_read_config_dword(struct pci_dev *pdev, int reg, u32 *val)
{
	*val = (u32)pci_read_config(pdev->dev.bsddev, reg, 4);
	return 0;
}

static inline int
pci_read_config_word(struct pci_dev *pdev, int reg, u16 *val)
{
	*val = (u16)pci_read_config(pdev->dev.bsddev, reg, 2);
	return 0;
}

static inline int
pci_read_config_byte(struct pci_dev *pdev, int reg, u8 *val)
{
	*val = (u8)pci_read_config(pdev->dev.bsddev, reg, 1);
	return 0;
}

static inline int
pci_write_config_dword(struct pci_dev *pdev, int reg, u32 val)
{
	pci_write_config(pdev->dev.bsddev, reg, val, 4);
	return 0;
}

static inline int
pci_write_config_word(struct pci_dev *pdev, int reg, u16 val)
{
	pci_write_config(pdev->dev.bsddev, reg, val, 2);
	return 0;
}

static inline int
pci_write_config_byte(struct pci_dev *pdev, int reg, u8 val)
{
	pci_write_config(pdev->dev.bsddev, reg, val, 1);
	return 0;
}

static inline int
pci_bus_read_config_word(struct pci_bus *bus, unsigned int devfn,
    int reg, u16 *val)
{
	const struct pci_dev *pdev = container_of(&bus, struct pci_dev, bus);

	*val = (u16)pci_read_config(pdev->dev.bsddev, reg, 2);
	return 0;
}

static inline int
pci_bus_read_config_byte(struct pci_bus *bus, unsigned int devfn,
    int reg, u8 *val)
{
	const struct pci_dev *pdev = container_of(&bus, struct pci_dev, bus);

	*val = (u8)pci_read_config(pdev->dev.bsddev, reg, 1);
	return 0;
}

#if defined(__OpenBSD__)
static inline int
pci_bus_write_config_byte(struct pci_bus *bus, unsigned int devfn,
    int reg, u8 val)
{
	pcitag_t tag = pci_make_tag(bus->pc, bus->number,
	    PCI_SLOT(devfn), PCI_FUNC(devfn));
	uint32_t v;

	v = pci_conf_read(bus->pc, tag, (reg & ~0x3));
	v &= ~(0xff << ((reg & 0x3) * 8));
	v |= (val << ((reg & 0x3) * 8));
	pci_conf_write(bus->pc, tag, (reg & ~0x3), v);
	return 0;
}
#else
/* Not sure */
static inline int
pci_bus_write_config_byte(struct pci_bus *bus, unsigned int devfn,
    int reg, u8 val)
{
	const struct pci_dev *pdev = container_of(&bus, struct pci_dev, bus);

	pci_write_config(pdev->dev.bsddev, reg, val, 1);
	return 0;
}
#endif

static inline int
pci_pcie_cap(struct pci_dev *pdev)
{
	int pos;
#if defined(__OpenBSD__)
	if (!pci_get_capability(pdev->pc, pdev->tag, PCI_CAP_PCIEXPRESS,
	    &pos, NULL))
		return -EINVAL;
#else
	pos = pci_get_pciecap_ptr(pdev->dev.bsddev);
	if (!pos)
		return -EINVAL;
#endif
	return pos;
}

#if defined(__OpenBSD__)
bool pcie_aspm_enabled(struct pci_dev *);
#endif

#if defined(__DragonFly__)
#define PCI_PCIE_LCSR		0x10
#define PCI_PCIE_LCSR_ASPM_L0S	0x00000001
#define PCI_PCIE_LCSR_ASPM_L1	0x00000002
#endif

/* From OpenBSD drm_linux.c */
static inline bool
pcie_aspm_enabled(struct pci_dev *pdev)
{
#if defined(__OpenBSD__)
	pci_chipset_tag_t	pc = pdev->pc;
	pcitag_t		tag = pdev->tag;
#endif
	int			pos ;
	uint32_t		lcsr;

#if defined(__OpenBSD__)
	if (!pci_get_capability(pc, tag, PCI_CAP_PCIEXPRESS,
	    &pos, NULL)) 
		return false;

	lcsr = pci_conf_read(pc, tag, pos + PCI_PCIE_LCSR);
#else
	if (!pci_find_extcap(pdev->dev.bsddev, PCIY_EXPRESS, &pos)) {
		return false;
	}
	lcsr = (u32)pci_read_config(pdev->dev.bsddev, pos + PCI_PCIE_LCSR, 4);
#endif
	if ((lcsr & (PCI_PCIE_LCSR_ASPM_L0S | PCI_PCIE_LCSR_ASPM_L1)) != 0)
		return true;
	
	return false;
}

#if defined(__OpenBSD__)
static inline bool
pci_is_pcie(struct pci_dev *pdev)
{
	return (pci_pcie_cap(pdev) > 0);
}
#else
/* defined in sys/bus/pcivar.h */
/* API incompatible */
// int pci_is_pcie(device_t dev)
#endif

static inline bool
pci_is_root_bus(struct pci_bus *pbus)
{
#if defined(__OpenBSD__)
	return (pbus->bridgetag == NULL);
#else
/* From FreeBSD */
	return (pbus-> self == NULL);
#endif
}

static inline struct pci_dev *
pci_upstream_bridge(struct pci_dev *pdev)
{
	if (pci_is_root_bus(pdev->bus))
		return NULL;
	return pdev->bus->self;
}

/* XXX check for ACPI _PR3 */
static inline bool
pci_pr3_present(struct pci_dev *pdev)
{
	return false;
}

int pcie_capability_read_dword(struct pci_dev *dev, int pos, u32 *val);

#if 0
static inline int
pcie_capability_read_dword(struct pci_dev *pdev, int off, u32 *val)
{
	int pos;
#if defined(__OpenBSD__)
	if (!pci_get_capability(pdev->pc, pdev->tag, PCI_CAP_PCIEXPRESS,
	    &pos, NULL)) {
		*val = 0;
		return -EINVAL;
	}
	*val = pci_conf_read(pdev->pc, pdev->tag, pos + off);
#else
/* Not sure, guessed from pcie_get_readrq
 * extracted from drm/radeon/evergreen.c */
	if (!pci_find_extcap(pdev->dev.bsddev, PCIY_EXPRESS, &pos)) {
		*val = 0;
		return -EINVAL;
	}
	*val = (u32)pci_read_config(pdev->dev.bsddev, pos + off, 4);
#endif
	return 0;
}
#endif

static inline int
pcie_capability_read_word(struct pci_dev *pdev, int off, u16 *val)
{
	int pos;
#if defined(__OpenBSD__)
	if (!pci_get_capability(pdev->pc, pdev->tag, PCI_CAP_PCIEXPRESS,
	    &pos, NULL)) {
		*val = 0;
		return -EINVAL;
	}
	pci_read_config_word(pdev, pos + off, val);
#else
/* Not sure, guessed from pcie_get_readrq
 * extracted from drm/radeon/evergreen.c */
	if (!pci_find_extcap(pdev->dev.bsddev, PCIY_EXPRESS, &pos)) {
		*val = 0;
		return -EINVAL;
	}
	*val = (u16)pci_read_config(pdev->dev.bsddev, pos + off, 2);
#endif
	return 0;
}

/* OpenBSD   #define PCI_CAP_PCIEXPRESS 0x10 */
/* DragonFly #define PCIY_EXPRESS       0x10 */

static inline int
pcie_capability_write_word(struct pci_dev *pdev, int off, u16 val)
{
	int pos;
#if defined(__OpenBSD__)
	if (!pci_get_capability(pdev->pc, pdev->tag, PCI_CAP_PCIEXPRESS,
	    &pos, NULL))
		return -EINVAL;
	pci_write_config_word(pdev, pos + off, val);
#else
/* Not sure */
	if (!pci_find_extcap(pdev->dev.bsddev, PCIY_EXPRESS, &pos))
		return -EINVAL;
	pci_write_config_word(pdev, pos + off, val);
#endif
	return 0;
}

/* extracted from drm/radeon/evergreen.c */
static inline int
pcie_get_readrq(struct pci_dev *pdev)
{
#if defined(__OpenBSD__)
	uint16_t val;

	pcie_capability_read_word(pdev, PCI_PCIE_DCSR, &val);

	return 128 << ((val & PCI_PCIE_DCSR_MPS) >> 12);
#else
	uint16_t val;
	int err, pos;

	err = pci_find_extcap(pdev->dev.bsddev, PCIY_EXPRESS, &pos);

	pos += PCIER_DEVCTRL;

	val = pci_read_config(pdev->dev.bsddev, pos, 2);

	return 128 << ((val & PCIEM_DEVCTL_MAX_READRQ_MASK) >> 12);
#endif
}

/* OpenBSD   #define PCI_PCIE_DCSR 0x08 */
/* DragonFly #define PCIER_DEVCTRL 0x08 */
/* OpenBSD   #define PCI_PCIE_DCSR_MPS        0x00007000 */
/* DragonFly #define PCIEM_DEVCTL_MAX_READRQ_MASK 0x7000 */

/* valid rrq sizes: 128, 256, 512, 1024, 2048, 4096 (^2N) */
static inline int
pcie_set_readrq(struct pci_dev *pdev, int rrq)
{
#if defined(__OpenBSD__)
	uint16_t val;
	
	pcie_capability_read_word(pdev, PCI_PCIE_DCSR, &val);
	val &= ~PCI_PCIE_DCSR_MPS;
	val |= (ffs(rrq) - 8) << 12;
	return pcie_capability_write_word(pdev, PCI_PCIE_DCSR, val);
#else
	uint16_t val;
	int err, pos;

	if (rrq < 128 || rrq > 4096 || !is_power_of_2(rrq))
		return -EINVAL;

	err = pci_find_extcap(pdev->dev.bsddev, PCIY_EXPRESS, &pos);
	if (err)
		return (-1);

	pos += PCIER_DEVCTRL;

	val = pci_read_config(pdev->dev.bsddev, pos, 2);
	val &= ~PCIEM_DEVCTL_MAX_READRQ_MASK;
	val |= ((ffs(rrq) - 8) << 12);
	pci_write_config(pdev->dev.bsddev, pos, val, 2);
	return 0;
#endif
}

static inline void
pci_set_master(struct pci_dev *pdev)
{
#if defined(__OpenBSD__)
#elif defined(__DragonFly__)
	pci_enable_busmaster(pdev->dev.bsddev);
#endif
}

static inline void
pci_clear_master(struct pci_dev *pdev)
{
#if defined(__OpenBSD__)
#elif defined(__DragonFly__)
	pci_disable_busmaster(pdev->dev.bsddev);
#endif
}

static inline struct pci_dev *
pci_dev_get(struct pci_dev *dev)
{
	/* Linux increments a reference count here */
	return dev;
}

/* Not sure */
#if defined(__OpenBSD__)
static inline void
pci_save_state(struct pci_dev *pdev)
{
}

static inline void
pci_restore_state(struct pci_dev *pdev)
{
}
#elif defined(__DragonFly__)
static inline void
drm_pci_save_state(struct pci_dev *pdev)
{
	pci_save_state(pdev->dev.bsddev);
}

static inline void
drm_pci_restore_state(struct pci_dev *pdev)
{
	pci_restore_state(pdev->dev.bsddev);
}
#endif

#if defined(__OpenBSD__)
static inline int
pci_enable_msi(struct pci_dev *pdev)
{
	return 0;
}

static inline void
pci_disable_msi(struct pci_dev *pdev)
{
}
#elif defined(__DragonFly__)
static inline int
drm_pci_enable_msi(struct pci_dev *pdev)
{
	return 0;
}

static inline void
drm_pci_disable_msi(struct pci_dev *pdev)
{
}
#endif

static inline int
pci_set_dma_mask(struct pci_dev *dev, u64 mask)
{
	return -EIO;
}

static inline int
pci_set_consistent_dma_mask(struct pci_dev *dev, u64 mask)
{
	return -EIO;
}

typedef enum {
	PCI_D0,
	PCI_D1,
	PCI_D2,
	PCI_D3hot,
	PCI_D3cold
} pci_power_t;

enum pci_bus_speed {
	PCIE_SPEED_2_5GT,
	PCIE_SPEED_5_0GT,
	PCIE_SPEED_8_0GT,
	PCIE_SPEED_16_0GT,
	PCIE_SPEED_32_0GT,
	PCIE_SPEED_64_0GT,
	PCI_SPEED_UNKNOWN
};

enum pcie_link_width {
	PCIE_LNK_X1	= 1,
	PCIE_LNK_X2	= 2,
	PCIE_LNK_X4	= 4,
	PCIE_LNK_X8	= 8,
	PCIE_LNK_X12	= 12,
	PCIE_LNK_X16	= 16,
	PCIE_LNK_X32	= 32,
	PCIE_LNK_WIDTH_UNKNOWN	= 0xff
};

#if 0
typedef int pci_power_t;

#define PCI_D0		0
#define PCI_D1		1
#define PCI_D2		2
#define PCI_D3hot	3
#define PCI_D3cold	4

enum pci_bus_speed {
	PCIE_SPEED_2_5GT		= 0x14,
	PCIE_SPEED_5_0GT		= 0x15,
	PCIE_SPEED_8_0GT		= 0x16,
	PCIE_SPEED_16_0GT		= 0x17,
	PCI_SPEED_UNKNOWN		= 0xff,
};

/* Values from Link Status register, PCIe r3.1, sec 7.8.8 */
enum pcie_link_width {
	PCIE_LNK_WIDTH_RESRV	= 0x00,
	PCIE_LNK_X1		= 0x01,
	PCIE_LNK_X2		= 0x02,
	PCIE_LNK_X4		= 0x04,
	PCIE_LNK_X8		= 0x08,
	PCIE_LNK_X12		= 0x0c,
	PCIE_LNK_X16		= 0x10,
	PCIE_LNK_X32		= 0x20,
	PCIE_LNK_WIDTH_UNKNOWN	= 0xff,
};
#endif

typedef unsigned int pci_ers_result_t;
typedef unsigned int pci_channel_state_t;

#define PCI_ERS_RESULT_DISCONNECT	0
#define PCI_ERS_RESULT_RECOVERED	1

#include <asm/pci.h>

#if defined(__OpenBSD__)

enum pci_bus_speed pcie_get_speed_cap(struct pci_dev *);
enum pcie_link_width pcie_get_width_cap(struct pci_dev *);
int pci_resize_resource(struct pci_dev *, int, int);

#elif defined(__DragonFly__)

#define	PCIER_LINK_CAP		0xc

/* From FreeBSD */
static inline enum pci_bus_speed
pcie_get_speed_cap(struct pci_dev *dev)
{
	device_t root;
	uint32_t lnkcap, lnkcap2;
	int error, pos;

	root = device_get_parent(dev->dev.bsddev);
	if (root == NULL)
		return (PCI_SPEED_UNKNOWN);
	root = device_get_parent(root);
	if (root == NULL)
		return (PCI_SPEED_UNKNOWN);
	root = device_get_parent(root);
	if (root == NULL)
		return (PCI_SPEED_UNKNOWN);

	if (pci_get_vendor(root) == PCI_VENDOR_ID_VIA ||
	    pci_get_vendor(root) == PCI_VENDOR_ID_SERVERWORKS)
		return (PCI_SPEED_UNKNOWN);

	if ((error = pci_find_extcap(root, PCIY_EXPRESS, &pos)) != 0)
		return (PCI_SPEED_UNKNOWN);

	lnkcap2 = pci_read_config(root, pos + PCIER_LINK_CAP2, 4);

	if (lnkcap2) {	/* PCIe r3.0-compliant */
		if (lnkcap2 & PCI_EXP_LNKCAP2_SLS_2_5GB)
			return (PCIE_SPEED_2_5GT);
		if (lnkcap2 & PCI_EXP_LNKCAP2_SLS_5_0GB)
			return (PCIE_SPEED_5_0GT);
		if (lnkcap2 & PCI_EXP_LNKCAP2_SLS_8_0GB)
			return (PCIE_SPEED_8_0GT);
		if (lnkcap2 & PCI_EXP_LNKCAP2_SLS_16_0GB)
			return (PCIE_SPEED_16_0GT);
	} else {	/* pre-r3.0 */
		lnkcap = pci_read_config(root, pos + PCIER_LINK_CAP, 4);
		if (lnkcap & PCI_EXP_LNKCAP_SLS_2_5GB)
			return (PCIE_SPEED_2_5GT);
		if (lnkcap & PCI_EXP_LNKCAP_SLS_5_0GB)
			return (PCIE_SPEED_5_0GT);
		if (lnkcap & PCI_EXP_LNKCAP_SLS_8_0GB)
			return (PCIE_SPEED_8_0GT);
		if (lnkcap & PCI_EXP_LNKCAP_SLS_16_0GB)
			return (PCIE_SPEED_16_0GT);
	}
	return (PCI_SPEED_UNKNOWN);
}

/* From FreeBSD */
static inline enum pcie_link_width
pcie_get_width_cap(struct pci_dev *dev)
{
	uint32_t lnkcap;

	pcie_capability_read_dword(dev, PCI_EXP_LNKCAP, &lnkcap);
	if (lnkcap)
		return ((lnkcap & PCI_EXP_LNKCAP_MLW) >> 4);

	return (PCIE_LNK_WIDTH_UNKNOWN);
}

/* Not implemented */
static inline int
pci_resize_resource(struct pci_dev *pdev, int bar, int nsize)
{
	return -EOPNOTSUPP;
}

#endif

/* Not sure */
static inline void
pcie_bandwidth_available(struct pci_dev *pdev, struct pci_dev **ldev,
    enum pci_bus_speed *speed, enum pcie_link_width *width)
{
	struct pci_dev *bdev = pdev->bus->self;
	if (bdev == NULL)
		return;

	if (speed)
		*speed = pcie_get_speed_cap(bdev);
	if (width)
		*width = pcie_get_width_cap(bdev);
}


static inline int
pci_enable_device(struct pci_dev *pdev)
{
#if defined(__OpenBSD__)
	return 0;
#else
/* From FreeBSD */
	pci_enable_io(pdev->dev.bsddev, SYS_RES_IOPORT);
	pci_enable_io(pdev->dev.bsddev, SYS_RES_MEMORY);
	return (0);
#endif
}

static inline void
pci_disable_device(struct pci_dev *pdev)
{
#if defined(__OpenBSD__)
#else
/* From FreeBSD */
	pci_disable_busmaster(pdev->dev.bsddev);
#endif
}

static inline bool
pci_is_thunderbolt_attached(struct pci_dev *pdev)
{
	return false;
}

static inline void
pci_set_drvdata(struct pci_dev *pdev, void *data)
{
#if defined(__OpenBSD__)
#else
/* From FreeBSD */
	pdev->pci_dev_data = data;
#endif
}

static inline int
pci_domain_nr(struct pci_bus *pbus)
{
#if defined(__OpenBSD__)
	return pbus->domain_nr;
#elif defined(__DragonFly__)
	return pci_get_domain(pbus->self->dev.bsddev);
#endif
}

static inline int
pci_irq_vector(struct pci_dev *pdev, unsigned int num)
{
	return pdev->irq;
}

static inline void
pci_free_irq_vectors(struct pci_dev *pdev)
{
}

static inline int
pci_set_power_state(struct pci_dev *dev, int state)
{
	return 0;
}

#if defined(__OpenBSD__)
static inline void
pci_unregister_driver(void *d)
{
}
#elif defined(__DragonFly__)
static inline void
pci_unregister_driver(struct pci_driver *dev)
{
	/* pci_unregister_driver not implemented */
}
#endif

#define PCI_CLASS_DISPLAY_VGA \
    ((PCI_CLASS_DISPLAY << 8) | PCI_SUBCLASS_DISPLAY_VGA)
#define PCI_CLASS_DISPLAY_OTHER \
    ((PCI_CLASS_DISPLAY << 8) | PCI_SUBCLASS_DISPLAY_MISC)

/* OpenBSD notyet pci_get_class(int, struct pci_dev*) */

static inline struct resource_list_entry*
_pci_get_rle(struct pci_dev *pdev, int bar)
{
	struct pci_devinfo *dinfo;
	device_t dev = pdev->dev.bsddev;
	struct resource_list_entry *rle;

	dinfo = device_get_ivars(dev);

	/* Some child devices don't have registered resources, they
	 * are only present in the parent */
	if (dinfo == NULL)
		dev = device_get_parent(dev);
	dinfo = device_get_ivars(dev);
	if (dinfo == NULL)
		return NULL;

	rle = resource_list_find(&dinfo->resources, SYS_RES_MEMORY, PCIR_BAR(bar));
	if (rle == NULL) {
		rle = resource_list_find(&dinfo->resources,
					 SYS_RES_IOPORT, PCIR_BAR(bar));
	}

	return rle;
}

/*
 * Returns the first address (memory address or I/O port number)
 * associated with one of the PCI I/O regions.The region is selected by
 * the integer bar (the base address register), ranging from 0–5 (inclusive).
 * The return value can be used by ioremap()
 */
static inline phys_addr_t
pci_resource_start(struct pci_dev *pdev, int bar)
{
	struct resource *res;
	int rid;

	rid = PCIR_BAR(bar);
	res = bus_alloc_resource_any(pdev->dev.bsddev, SYS_RES_MEMORY, &rid, RF_SHAREABLE);
	if (res == NULL) {
		kprintf("pci_resource_start(0x%p, 0x%x) failed\n", pdev, PCIR_BAR(bar));
		return -1;
	}

	return rman_get_start(res);
}

static inline phys_addr_t
pci_resource_len(struct pci_dev *pdev, int bar)
{
	struct resource_list_entry *rle;

	rle = _pci_get_rle(pdev, bar);
	if (rle == NULL)
		return -1;

	return  rman_get_size(rle->res);
}

/* OpenBSD comments out use of this */
static inline void __iomem *pci_iomap(struct pci_dev *dev, int bar, unsigned long maxlen)
{
	resource_size_t base, size;

	base = pci_resource_start(dev, bar);
	size = pci_resource_len(dev, bar);

	if (base == 0)
		return NULL;

	if (maxlen && size > maxlen)
		size = maxlen;

	return ioremap(base, size);
}


static inline const char *
pci_name(struct pci_dev *pdev)
{
	return device_get_desc(pdev->dev.bsddev);
}

/* OpenBSD has use of this notyet */
static inline void *
pci_get_drvdata(struct pci_dev *pdev)
{
	return pdev->pci_dev_data;
}


static inline int
pci_register_driver(struct pci_driver *drv)
{
	/* pci_register_driver not implemented */
	return 0;
}




/* DRM_MAX_PCI_RESOURCE */
#define DEVICE_COUNT_RESOURCE	6



static inline bool pcie_cap_has_devctl(const struct pci_dev *dev)
{
		return true;
}

static inline int
pci_find_capability(struct pci_dev *pdev, int capid)
{
	int reg;

	if (pci_find_extcap(pdev->dev.bsddev, capid, &reg))
		return (0);
	return (reg);
}

static inline u16 pcie_flags_reg(struct pci_dev *dev)
{
	int pos;
	u16 reg16;

	pos = pci_find_capability(dev, PCI_CAP_ID_EXP);
	if (!pos)
		return 0;

	pci_read_config_word(dev, pos + PCI_EXP_FLAGS, &reg16);

	return reg16;
}

static inline int pci_pcie_type(struct pci_dev *dev)
{
	return (pcie_flags_reg(dev) & PCI_EXP_FLAGS_TYPE) >> 4;
}


static inline int pcie_cap_version(struct pci_dev *dev)
{
	return pcie_flags_reg(dev) & PCI_EXP_FLAGS_VERS;
}

static inline bool pcie_cap_has_lnkctl(struct pci_dev *dev)
{
	int type = pci_pcie_type(dev);

	return pcie_cap_version(dev) > 1 ||
	       type == PCI_EXP_TYPE_ROOT_PORT ||
	       type == PCI_EXP_TYPE_ENDPOINT ||
	       type == PCI_EXP_TYPE_LEG_END;
}

static inline bool pcie_cap_has_sltctl(struct pci_dev *dev)
{
	int type = pci_pcie_type(dev);

	return pcie_cap_version(dev) > 1 || type == PCI_EXP_TYPE_ROOT_PORT ||
	    (type == PCI_EXP_TYPE_DOWNSTREAM &&
	    pcie_flags_reg(dev) & PCI_EXP_FLAGS_SLOT);
}

static inline bool pcie_cap_has_rtctl(struct pci_dev *dev)
{
	int type = pci_pcie_type(dev);

	return pcie_cap_version(dev) > 1 || type == PCI_EXP_TYPE_ROOT_PORT ||
	    type == PCI_EXP_TYPE_RC_EC;
}

static inline bool
pcie_capability_reg_implemented(struct pci_dev *dev, int pos)
{
	if (!pci_is_pcie(dev->dev.bsddev))
		return false;

	switch (pos) {
	case PCI_EXP_FLAGS_TYPE:
		return true;
	case PCI_EXP_DEVCAP:
	case PCI_EXP_DEVCTL:
	case PCI_EXP_DEVSTA:
		return pcie_cap_has_devctl(dev);
	case PCI_EXP_LNKCAP:
	case PCI_EXP_LNKCTL:
	case PCI_EXP_LNKSTA:
		return pcie_cap_has_lnkctl(dev);
	case PCI_EXP_SLTCAP:
	case PCI_EXP_SLTCTL:
	case PCI_EXP_SLTSTA:
		return pcie_cap_has_sltctl(dev);
	case PCI_EXP_RTCTL:
	case PCI_EXP_RTCAP:
	case PCI_EXP_RTSTA:
		return pcie_cap_has_rtctl(dev);
	case PCI_EXP_DEVCAP2:
	case PCI_EXP_DEVCTL2:
	case PCI_EXP_LNKCAP2:
	case PCI_EXP_LNKCTL2:
	case PCI_EXP_LNKSTA2:
		return pcie_cap_version(dev) > 1;
	default:
		return false;
	}
}

static inline void __iomem __must_check *
pci_map_rom(struct pci_dev *pdev, size_t *size)
{
	return vga_pci_map_bios(device_get_parent(pdev->dev.bsddev), size);
}

static inline void
pci_unmap_rom(struct pci_dev *pdev, void __iomem *rom)
{
	vga_pci_unmap_bios(device_get_parent(pdev->dev.bsddev), rom);
}

static inline int
pci_resource_flags(struct pci_dev *pdev, int bar)
{
	/* Hardcoded to return only the type */
	if ((bar & PCIM_BAR_SPACE) == PCIM_BAR_IO_SPACE) {
		kprintf("pci_resource_flags: pdev=%p bar=%d type=IO\n", pdev, bar);
		return IORESOURCE_IO;
	} else {
		kprintf("pci_resource_flags: pdev=%p bar=%d type=MEM\n", pdev, bar);
		return IORESOURCE_MEM;
	}
}




#include <linux/pci-dma-compat.h>

#endif /* LINUX_PCI_H */
