/* Public domain. */

#ifndef _ASM_BYTEORDER_H
#define _ASM_BYTEORDER_H

#include <sys/endian.h>
#include <linux/types.h>

#if defined(__OpenBSD__)
#define le16_to_cpu(x) letoh16(x)
#define le32_to_cpu(x) letoh32(x)
#define le64_to_cpu(x) letoh64(x)
#else
#define le16_to_cpu(x)	le16toh(x)
#define le32_to_cpu(x)	le32toh(x)
#define le64_to_cpu(x)	le64toh(x)
#endif

#if defined(__OpenBSD__)
#define be16_to_cpu(x) betoh16(x)
#define be32_to_cpu(x) betoh32(x)
#define be64_to_cpu(x) betoh64(x)
#else
#define be16_to_cpu(x)	be16toh(x)
#define be32_to_cpu(x)	be32toh(x)
#define be64_to_cpu(x)	be64toh(x)
#endif

#if defined(__OpenBSD__)
#define le16_to_cpup(x)	lemtoh16(x)
#define le32_to_cpup(x)	lemtoh32(x)
#define le64_to_cpup(x)	lemtoh64(x)
#else
#define le16_to_cpup(x)	le16toh(*x)
#define le32_to_cpup(x)	le32toh(*x)
#define le64_to_cpup(x)	le64toh(*x)
#endif

#if defined(__OpenBSD__)
#define be16_to_cpup(x)	bemtoh16(x)
#define be32_to_cpup(x)	bemtoh32(x)
#define be64_to_cpup(x)	bemtoh64(x)
#else
#define be16_to_cpup(x)	be16toh(*x)
#define be32_to_cpup(x)	be32toh(*x)
#define be64_to_cpup(x)	be64toh(*x)
#endif

#if defined(__OpenBSD__) 
#define get_unaligned_le32(x)	lemtoh32(x)
#else /* On DragonFly defined in asm/unaligned.h */
#endif

#define cpu_to_le16(x) htole16(x)
#define cpu_to_le32(x) htole32(x)
#define cpu_to_le64(x) htole64(x)

#define cpu_to_be16(x) htobe16(x)
#define cpu_to_be32(x) htobe32(x)
#define cpu_to_be64(x) htobe64(x)

#if defined(__OpenBSD__) 
#define swab16(x) swap16(x)
#define swab32(x) swap32(x)
#else /* not sure */
#define swab16(x) bswap16(x)
#define swab32(x) bswap32(x)
#endif

static inline void
le16_add_cpu(uint16_t *p, uint16_t n)
{
#if defined(__OpenBSD__)
	htolem16(p, lemtoh16(p) + n);
#else
	*p = htole16(le16toh(*p) + n);
#endif
}

#endif
