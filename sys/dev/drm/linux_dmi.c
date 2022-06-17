/*
 * Copyright (c) 2019 François Tigeot <ftigeot@wolfpond.org>
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

#include <sys/kernel.h>

#include <linux/types.h>
#include <linux/bug.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/ctype.h>
#include <linux/dmi.h>
#include <asm/unaligned.h>

char smbios_sys_vendor[64];
char smbios_board_vendor[64];
char smbios_product_name[64];
char smbios_board_prod[64];
char smbios_bios_date[64];

static int
dmi_init_info(void *dummy __unused)
{
	const char *s = NULL;
	s = kgetenv("smbios.system.maker");
	if (s != NULL) {
		strlcpy(smbios_sys_vendor, s, sizeof(smbios_sys_vendor));
	}
	s = kgetenv("smbios.planar.maker");
	if (s != NULL) {
		strlcpy(smbios_board_vendor, s, sizeof(smbios_board_vendor));
	}
	s = kgetenv("smbios.system.product");
	if (s != NULL) {
		strlcpy(smbios_product_name, s, sizeof(smbios_product_name));
	}
	s = kgetenv("smbios.planar.product");
	if (s != NULL) {
		strlcpy(smbios_board_prod, s, sizeof(smbios_board_prod));
	}
	s = kgetenv("smbios.bios.reldate");
	if (s != NULL) {
		strlcpy(smbios_bios_date, s, sizeof(smbios_bios_date));
	}
	return 0;
}


bool
dmi_match(enum dmi_field slot, const char *str)
{
	const char *s = NULL;

	switch (slot) {
	case DMI_NONE:
		break;
	case DMI_SYS_VENDOR:
		s = kgetenv("smbios.system.maker");
		if (s != NULL && !strcmp(s, str))
			return true;
		break;
	case DMI_PRODUCT_NAME:
		s = kgetenv("smbios.system.product");
		if (s != NULL && !strcmp(s, str))
			return true;
		break;
	case DMI_PRODUCT_VERSION:
		s = kgetenv("smbios.system.version");
		if (s != NULL && !strcmp(s, str))
			return true;
		break;
	case DMI_BOARD_VENDOR:
		s = kgetenv("smbios.planar.maker");
		if (s != NULL && !strcmp(s, str))
			return true;
		break;
	case DMI_BOARD_NAME:
		s = kgetenv("smbios.planar.product");
		if (s != NULL && !strcmp(s, str))
			return true;
		break;
	default:
		return false;
	}

	return false;
}

/*
 * Check if dmi_system_id structure matches system DMI data
 */
static bool
dmi_found(const struct dmi_system_id *dsi)
{
#if defined(__OpenBSD__)
	int i, slot;

	for (i = 0; i < nitems(dsi->matches); i++) {
		slot = dsi->matches[i].slot;
		if (slot == DMI_NONE)
			break;
		if (!dmi_match(slot, dsi->matches[i].substr))
			return false;
	}

	return true;
#else
	int i, slot;
	bool found = false;
	char *sys_vendor, *board_vendor, *product_name, *board_name;

	sys_vendor = kgetenv("smbios.system.maker");
	board_vendor = kgetenv("smbios.planar.maker");
	product_name = kgetenv("smbios.system.product");
	board_name = kgetenv("smbios.planar.product");

	for (i = 0; i < NELEM(dsi->matches); i++) {
		slot = dsi->matches[i].slot;
		switch (slot) {
		case DMI_NONE:
			break;
		case DMI_SYS_VENDOR:
			if (sys_vendor != NULL &&
			    !strcmp(sys_vendor, dsi->matches[i].substr))
				break;
			else
				goto done;
		case DMI_BOARD_VENDOR:
			if (board_vendor != NULL &&
			    !strcmp(board_vendor, dsi->matches[i].substr))
				break;
			else
				goto done;
		case DMI_PRODUCT_NAME:
			if (product_name != NULL &&
			    !strcmp(product_name, dsi->matches[i].substr))
				break;
			else
				goto done;
		case DMI_BOARD_NAME:
			if (board_name != NULL &&
			    !strcmp(board_name, dsi->matches[i].substr))
				break;
			else
				goto done;
		default:
			goto done;
		}
	}
	found = true;

done:
	if (sys_vendor != NULL)
		kfreeenv(sys_vendor);
	if (board_vendor != NULL)
		kfreeenv(board_vendor);
	if (product_name != NULL)
		kfreeenv(product_name);
	if (board_name != NULL)
		kfreeenv(board_name);

	return found;
#endif
}

const struct dmi_system_id *
dmi_first_match(const struct dmi_system_id *sysid)
{
	const struct dmi_system_id *dsi;

	for (dsi = sysid; dsi->matches[0].slot != 0 ; dsi++) {
		if (dmi_found(dsi))
			return dsi;
	}

	return NULL;
}

const char *
dmi_get_system_info(int slot)
{
	WARN_ON(slot != DMI_BIOS_DATE);
	if (slot == DMI_BIOS_DATE)
		return smbios_bios_date;
	return NULL;
}

int
dmi_check_system(const struct dmi_system_id *sysid)
{
	const struct dmi_system_id *dsi;
	int num = 0;

	for (dsi = sysid; dsi->matches[0].slot != 0 ; dsi++) {
		if (dmi_found(dsi)) {
			num++;
			if (dsi->callback && dsi->callback(dsi))
				break;
		}
	}
	return (num);
}

SYSINIT(linux_dmi_init_info, SI_SUB_DRIVERS, SI_ORDER_MIDDLE, dmi_init_info, NULL);
