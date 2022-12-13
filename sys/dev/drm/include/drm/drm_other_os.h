/*
 * Copyright (c) 2015 Imre Vadász <imre@vdsz.com>
 * Copyright (c) 2015 Rimvydas Jasinskas
 * Copyright (c) 2018 François Tigeot <ftigeot@wolfpond.org>
 *
 * DRM Dragonfly-specific helper functions
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

#ifndef _DRM_OTHER_OS_H_
#define _DRM_OTHER_OS_H_

#if defined(__DragonFly__)

#include <sys/types.h>
#include <sys/bus.h>
#include <sys/device.h>
#include <sys/file.h>

#include <sys/conf.h>
// #include <sys/sysctl.h>

#include <vm/vm_object.h>
#include <vm/vm_extern.h>
#include <vm/vm_pager.h>

struct pci_dev;
struct drm_device;

#if 0
struct sysctl_ctx_list;
struct sysctl_oid;

struct drm_sysctl_info {
	struct sysctl_ctx_list ctx;
	char   name[2];
};
#endif

/* Length for the array of resource pointers for drm_get_resource_*. */
#define DRM_MAX_PCI_RESOURCE	6

struct drm_softc {
	void *drm_driver_data;
};

#define drm_get_device_from_kdev(_kdev) (_kdev->si_drv1)

int vm_phys_fictitious_reg_range(vm_paddr_t start, vm_paddr_t end,
    vm_memattr_t memattr);
void vm_phys_fictitious_unreg_range(vm_paddr_t start, vm_paddr_t end);
vm_page_t vm_phys_fictitious_to_vm_page(vm_paddr_t pa);

d_kqfilter_t drm_kqfilter;
d_mmap_t drm_mmap;
d_mmap_single_t drm_mmap_single;

int drm_device_detach(device_t kdev);

void drm_cdevpriv_dtor(void *cd);

#if 0
int drm_add_busid_modesetting(struct drm_device *dev,
    struct sysctl_ctx_list *ctx, struct sysctl_oid *top);
#endif

/* XXX glue logic, should be done in drm_pci_init(), pending drm update */
void drm_init_pdev(device_t dev, struct pci_dev **pdev);
void drm_fini_pdev(struct pci_dev **pdev);
void drm_print_pdev(struct pci_dev *pdev);

#if 0
int drm_sysctl_init(struct drm_device *dev);
int drm_sysctl_cleanup(struct drm_device *dev);
#endif

int ttm_bo_mmap_single(struct file *fp, struct drm_device *dev, vm_ooffset_t *offset,
    vm_size_t size, struct vm_object **obj_res, int nprot);

int drm_gem_mmap_single(struct drm_device *dev, vm_ooffset_t *offset,
    vm_size_t size, struct vm_object **obj_res, int nprot);

int drm_pci_device_is_agp(struct drm_device *dev);

#endif /* DragonFly */

#endif /* _DRM_OTHER_OS_H_ */
