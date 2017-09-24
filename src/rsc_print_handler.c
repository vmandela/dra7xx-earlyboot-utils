/*
 * (C) Copyright 2017
 * Texas Instruments Incorporated, <www.ti.com>
 *
 * Venkateswara Rao Mandela <venkat.mandela@ti.com>
 *
 * SPDX-License-Identifier:	BSD-3-Clause
 */
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "remoteproc.h"
#include "rsc_priv.h"

static int print_trace(struct fw_rsc_trace *rsc, int offset, int avail)
{
	if (sizeof(*rsc) > avail) {
		printf("trace rsc is truncated\n");
		return -EINVAL;
	}

	/* make sure reserved bytes are zeroes */
	if (rsc->reserved) {
		printf("trace rsc has non zero reserved bytes\n");
		return -EINVAL;
	}

	printf("%10s rsc: da 0x%x, len 0x%x\n",
	       "trace",
	       rsc->da, rsc->len);

	return 0;
}

static int print_devmem(struct fw_rsc_devmem *rsc, int offset, int avail)
{
	if (sizeof(*rsc) > avail) {
		printf("devmem rsc is truncated\n");
		return -EINVAL;
	}

	/* make sure reserved bytes are zeroes */
	if (rsc->reserved) {
		printf("devmem rsc has non zero reserved bytes\n");
		return -EINVAL;
	}

	printf("%10s rsc: pa 0x%08x, da 0x%08x, len 0x%08x\n",
	       "devmem",
	      rsc->pa, rsc->da, rsc->len);

	return 0;
}

static int print_carveout(struct fw_rsc_carveout *rsc, int offset, int avail)
{
	if (sizeof(*rsc) > avail) {
		printf("carveout rsc is truncated\n");
		return -EINVAL;
	}

	/* make sure reserved bytes are zeroes */
	if (rsc->reserved) {
		printf("carveout rsc has non zero reserved bytes\n");
		return -EINVAL;
	}

	printf("%10s rsc: pa 0x%08x, da 0x%08x, len 0x%08x, flags 0x%08x\n",
	       "carveout",
	       rsc->pa, rsc->da, rsc->len, rsc->flags);

	return 0;
}

static int print_vdev(struct fw_rsc_vdev *rsc, int offset, int avail)
{
	int i;

	/* make sure resource isn't truncated */
	if (sizeof(*rsc) + rsc->num_of_vrings * sizeof(struct fw_rsc_vdev_vring)
	    + rsc->config_len > avail) {
		printf("vdev rsc is truncated\n");
		return -EINVAL;
	}

	/* make sure reserved bytes are zeroes */
	if (rsc->reserved[0] || rsc->reserved[1]) {
		printf("vdev rsc has non zero reserved bytes\n");
		return -EINVAL;
	}

	printf("%10s rsc: id %d, dfeatures %x, cfg len %d, %d vrings\n",
	       "vdev",
	       rsc->id, rsc->dfeatures, rsc->config_len, rsc->num_of_vrings);

	/* we currently support only two vrings per rvdev */
	if (rsc->num_of_vrings > 2) {
		printf("too many vrings: %d\n", rsc->num_of_vrings);
		return -EINVAL;
	}

	for (i = 0; i < rsc->num_of_vrings; i++) {

		printf("%18s rsc: num %d pa 0x%x da 0x%x \n",
		       "vring", rsc->vring[i].num,
		       0, rsc->vring[i].da);
	}

	return 0;
}

static int print_intmem(struct fw_rsc_intmem *rsc, int offset, int avail)
{

	printf( "intmem rsc: da 0x%x, pa 0x%x, len 0x%x\n",
		rsc->da, rsc->pa, rsc->len);
	return 0;
}
/*
 * A lookup table for resource handlers. The indices are defined in
 * enum fw_resource_type.
 */
handle_resource_t printing_handlers[RSC_LAST] = {
	[RSC_CARVEOUT] = (handle_resource_t) print_carveout,
	[RSC_DEVMEM] = (handle_resource_t) print_devmem,
	[RSC_TRACE] = (handle_resource_t) print_trace,
	[RSC_VDEV] = (handle_resource_t) print_vdev,
	[RSC_INTMEM] = (handle_resource_t) print_intmem,
};
