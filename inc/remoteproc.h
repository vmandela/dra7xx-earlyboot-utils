/*
 * Remote Processor Framework
 *
 * Copyright(c) 2011 Texas Instruments, Inc.
 * Copyright(c) 2011 Google, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 * * Neither the name Texas Instruments nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef REMOTEPROC_H
#define REMOTEPROC_H

#include <linux/types.h>
#include <stdint.h>

typedef uint32_t u32;
typedef uint8_t  u8;
typedef uint32_t dma_addr_t;

/**
 * struct resource_table - firmware resource table header
 * @ver: version number
 * @num: number of resource entries
 * @reserved: reserved (must be zero)
 * @offset: array of offsets pointing at the various resource entries
 *
 * A resource table is essentially a list of system resources required
 * by the remote processor. It may also include configuration entries.
 * If needed, the remote processor firmware should contain this table
 * as a dedicated ".resource_table" ELF section.
 *
 * Some resources entries are mere announcements, where the host is informed
 * of specific remoteproc configuration. Other entries require the host to
 * do something (e.g. allocate a system resource). Sometimes a negotiation
 * is expected, where the firmware requests a resource, and once allocated,
 * the host should provide back its details (e.g. address of an allocated
 * memory region).
 *
 * The header of the resource table, as expressed by this structure,
 * contains a version number (should we need to change this format in the
 * future), the number of available resource entries, and their offsets
 * in the table.
 *
 * Immediately following this header are the resource entries themselves,
 * each of which begins with a resource entry header (as described below).
 */
struct resource_table {
	u32 ver;
	u32 num;
	u32 reserved[2];
	u32 offset[0];
} __attribute__ ((__packed__));

/**
 * struct fw_rsc_hdr - firmware resource entry header
 * @type: resource type
 * @data: resource data
 *
 * Every resource entry begins with a 'struct fw_rsc_hdr' header providing
 * its @type. The content of the entry itself will immediately follow
 * this header, and it should be parsed according to the resource type.
 */
struct fw_rsc_hdr {
	u32 type;
	u8 data[0];
} __attribute__ ((__packed__));

/**
 * enum fw_resource_type - types of resource entries
 *
 * @RSC_CARVEOUT:   request for allocation of a physically contiguous
 *		    memory region.
 * @RSC_DEVMEM:     request to iommu_map a memory-based peripheral.
 * @RSC_TRACE:	    announces the availability of a trace buffer into which
 *		    the remote processor will be writing logs.
 * @RSC_VDEV:       declare support for a virtio device, and serve as its
 *		    virtio header.
 * @RSC_INTMEM:     request to map into kernel an internal memory region.
 * @RSC_CUSTOM:     a custom resource type that needs to be handled outside
 *		    remoteproc core.
 * @RSC_LAST:       just keep this one at the end
 *
 * For more details regarding a specific resource type, please see its
 * dedicated structure below.
 *
 * Please note that these values are used as indices to the rproc_handle_rsc
 * lookup table, so please keep them sane. Moreover, @RSC_LAST is used to
 * check the validity of an index before the lookup table is accessed, so
 * please update it as needed.
 */
enum fw_resource_type {
	RSC_CARVEOUT = 0,
	RSC_DEVMEM = 1,
	RSC_TRACE = 2,
	RSC_VDEV = 3,
	RSC_INTMEM = 4,
	RSC_CUSTOM = 5,
	RSC_LAST = 6,
};

#define FW_RSC_ADDR_ANY (0xFFFFFFFFFFFFFFFF)

/**
 * struct fw_rsc_carveout - physically contiguous memory request
 * @da: device address
 * @pa: physical address
 * @len: length (in bytes)
 * @flags: iommu protection flags
 * @reserved: reserved (must be zero)
 * @name: human-readable name of the requested memory region
 *
 * This resource entry requests the host to allocate a physically contiguous
 * memory region.
 *
 * These request entries should precede other firmware resource entries,
 * as other entries might request placing other data objects inside
 * these memory regions (e.g. data/code segments, trace resource entries, ...).
 *
 * Allocating memory this way helps utilizing the reserved physical memory
 * (e.g. CMA) more efficiently, and also minimizes the number of TLB entries
 * needed to map it (in case @rproc is using an IOMMU). Reducing the TLB
 * pressure is important; it may have a substantial impact on performance.
 *
 * If the firmware is compiled with static addresses, then @da should specify
 * the expected device address of this memory region. If @da is set to
 * FW_RSC_ADDR_ANY, then the host will dynamically allocate it, and then
 * overwrite @da with the dynamically allocated address.
 *
 * We will always use @da to negotiate the device addresses, even if it
 * isn't using an iommu. In that case, though, it will obviously contain
 * physical addresses.
 *
 * Some remote processors needs to know the allocated physical address
 * even if they do use an iommu. This is needed, e.g., if they control
 * hardware accelerators which access the physical memory directly (this
 * is the case with OMAP4 for instance). In that case, the host will
 * overwrite @pa with the dynamically allocated physical address.
 * Generally we don't want to expose physical addresses if we don't have to
 * (remote processors are generally _not_ trusted), so we might want to
 * change this to happen _only_ when explicitly required by the hardware.
 *
 * @flags is used to provide IOMMU protection flags, and @name should
 * (optionally) contain a human readable name of this carveout region
 * (mainly for debugging purposes).
 */
struct fw_rsc_carveout {
	u32 da;
	u32 pa;
	u32 len;
	u32 flags;
	u32 reserved;
	u8 name[32];
} __attribute__ ((__packed__));

/**
 * struct fw_rsc_devmem - iommu mapping request
 * @da: device address
 * @pa: physical address
 * @len: length (in bytes)
 * @flags: iommu protection flags
 * @reserved: reserved (must be zero)
 * @name: human-readable name of the requested region to be mapped
 *
 * This resource entry requests the host to iommu map a physically contiguous
 * memory region. This is needed in case the remote processor requires
 * access to certain memory-based peripherals; _never_ use it to access
 * regular memory.
 *
 * This is obviously only needed if the remote processor is accessing memory
 * via an iommu.
 *
 * @da should specify the required device address, @pa should specify
 * the physical address we want to map, @len should specify the size of
 * the mapping and @flags is the IOMMU protection flags. As always, @name may
 * (optionally) contain a human readable name of this mapping (mainly for
 * debugging purposes).
 *
 * Note: at this point we just "trust" those devmem entries to contain valid
 * physical addresses, but this isn't safe and will be changed: eventually we
 * want remoteproc implementations to provide us ranges of physical addresses
 * the firmware is allowed to request, and not allow firmwares to request
 * access to physical addresses that are outside those ranges.
 */
struct fw_rsc_devmem {
	u32 da;
	u32 pa;
	u32 len;
	u32 flags;
	u32 reserved;
	u8 name[32];
} __attribute__ ((__packed__));

/**
 * struct fw_rsc_trace - trace buffer declaration
 * @da: device address
 * @len: length (in bytes)
 * @reserved: reserved (must be zero)
 * @name: human-readable name of the trace buffer
 *
 * This resource entry provides the host information about a trace buffer
 * into which the remote processor will write log messages.
 *
 * @da specifies the device address of the buffer, @len specifies
 * its size, and @name may contain a human readable name of the trace buffer.
 *
 * After booting the remote processor, the trace buffers are exposed to the
 * user via debugfs entries (called trace0, trace1, etc..).
 */
struct fw_rsc_trace {
	u32 da;
	u32 len;
	u32 reserved;
	u8 name[32];
} __attribute__ ((__packed__));

/**
 * struct fw_rsc_vdev_vring - vring descriptor entry
 * @da: device address
 * @align: the alignment between the consumer and producer parts of the vring
 * @num: num of buffers supported by this vring (must be power of two)
 * @notifyid is a unique rproc-wide notify index for this vring. This notify
 * index is used when kicking a remote processor, to let it know that this
 * vring is triggered.
 * @reserved: reserved (must be zero)
 *
 * This descriptor is not a resource entry by itself; it is part of the
 * vdev resource type (see below).
 *
 * Note that @da should either contain the device address where
 * the remote processor is expecting the vring, or indicate that
 * dynamically allocation of the vring's device address is supported.
 */
struct fw_rsc_vdev_vring {
	u32 da;
	u32 align;
	u32 num;
	u32 notifyid;
	u32 reserved;
} __attribute__ ((__packed__));

/**
 * struct fw_rsc_vdev - virtio device header
 * @id: virtio device id (as in virtio_ids.h)
 * @notifyid is a unique rproc-wide notify index for this vdev. This notify
 * index is used when kicking a remote processor, to let it know that the
 * status/features of this vdev have changes.
 * @dfeatures specifies the virtio device features supported by the firmware
 * @gfeatures is a place holder used by the host to write back the
 * negotiated features that are supported by both sides.
 * @config_len is the size of the virtio config space of this vdev. The config
 * space lies in the resource table immediate after this vdev header.
 * @status is a place holder where the host will indicate its virtio progress.
 * @num_of_vrings indicates how many vrings are described in this vdev header
 * @reserved: reserved (must be zero)
 * @vring is an array of @num_of_vrings entries of 'struct fw_rsc_vdev_vring'.
 *
 * This resource is a virtio device header: it provides information about
 * the vdev, and is then used by the host and its peer remote processors
 * to negotiate and share certain virtio properties.
 *
 * By providing this resource entry, the firmware essentially asks remoteproc
 * to statically allocate a vdev upon registration of the rproc (dynamic vdev
 * allocation is not yet supported).
 *
 * Note: unlike virtualization systems, the term 'host' here means
 * the Linux side which is running remoteproc to control the remote
 * processors. We use the name 'gfeatures' to comply with virtio's terms,
 * though there isn't really any virtualized guest OS here: it's the host
 * which is responsible for negotiating the final features.
 * Yeah, it's a bit confusing.
 *
 * Note: immediately following this structure is the virtio config space for
 * this vdev (which is specific to the vdev; for more info, read the virtio
 * spec). the size of the config space is specified by @config_len.
 */
struct fw_rsc_vdev {
	u32 id;
	u32 notifyid;
	u32 dfeatures;
	u32 gfeatures;
	u32 config_len;
	u8 status;
	u8 num_of_vrings;
	u8 reserved[2];
	struct fw_rsc_vdev_vring vring[0];
} __attribute__ ((__packed__));

/**
 * struct fw_rsc_intmem - internal memory publishing request
 * @version: version for this resource type (must be one)
 * @da: device address
 * @pa: physical address
 * @len: length (in bytes)
 * @reserved: reserved (must be zero)
 * @name: human-readable name of the region being published
 *
 * This resource entry allows a remote processor to publish an internal
 * memory region to the host. This resource type allows a remote processor
 * to publish the whole or just a portion of certain internal memories,
 * while it owns and manages any unpublished portion (eg: a shared L1
 * memory that can be split configured as RAM and/or cache). This is
 * primarily provided to allow a host to load code/data into internal
 * memories, the memory for which is neither allocated nor required to
 * be mapped into an iommu.
 *
 * @da should specify the required address as accessible by the device
 * without going through an iommu, @pa should specify the physical address
 * for the region as seen on the bus, @len should specify the size of the
 * memory region. As always, @name may (optionally) contain a human readable
 * name of this mapping (mainly for debugging purposes). The @version field
 * is added for future scalability, and should be 1 for now.
 *
 * Note: at this point we just "trust" these intmem entries to contain valid
 * physical bus addresses. these are not currently intended to be managed
 * as host-controlled heaps, as it is much better to do that from the remote
 * processor side.
 */
struct fw_rsc_intmem {
	u32 version;
	u32 da;
	u32 pa;
	u32 len;
	u32 reserved;
	u8 name[32];
} __attribute__ ((__packed__));

/**
 * struct fw_rsc_custom - custom resource definition
 * @sub_type: implementation specific type
 * @size: size of the custom resource
 * @data: label for the start of the resource
 */
struct fw_rsc_custom {
	u32 sub_type;
	u32 size;
	u8 data[0];
} __attribute__ ((__packed__));

/**
 * struct rproc_mem_entry - memory entry descriptor
 * @va:	virtual address
 * @dma: dma address
 * @len: length, in bytes
 * @da: device address
 * @priv: associated data
 * @node: list node
 */
struct rproc_mem_entry {
	dma_addr_t dma;
	int len;
	u32 da;
};

#endif /* REMOTEPROC_H */
