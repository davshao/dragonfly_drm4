/*
 * Copyright (c) 2010 The DragonFly Project.  All rights reserved.
 *
 * This code is derived from software contributed to The DragonFly Project
 * by Ákos Kovács <akoskovacs@gmx.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of The DragonFly Project nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific, prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "libfsid.h"

#include <vfs/ext2fs/ext2fs.h>
#include <vfs/ext2fs/fs.h>

/*
 * struct ext2fs is actually ext4 compatible,
 * whereas it used to be using the real ext2 superblock struct.
 */
static char buffer[sizeof(struct ext2fs)*2];

fsid_t
ext2_probe(const char *dev)
{
	static struct ext2fs *fs;

	if (fsid_dev_read(dev, SBOFF, sizeof(buffer), buffer) != 0)
		return FSID_UNKNOWN;

	fs = (struct ext2fs *)&buffer;

	if (fs->e2fs_magic == E2FS_MAGIC)
		return FSID_EXT2;

	return FSID_UNKNOWN;
}

char *
ext2_volname(const char *dev)
{
	static struct ext2fs *fs;

	if (fsid_dev_read(dev, SBOFF, sizeof(buffer), buffer) != 0)
		return NULL;

	fs = (struct ext2fs *)&buffer;

	if (fs->e2fs_magic != E2FS_MAGIC || fs->e2fs_vname[0] == '\0')
		return NULL;

	fs->e2fs_vname[sizeof(fs->e2fs_vname) - 1] = '\0';

	return fs->e2fs_vname;
}
