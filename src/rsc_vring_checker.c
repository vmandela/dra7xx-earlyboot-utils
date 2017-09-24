/*
 * (C) Copyright 2017
 * Texas Instruments Incorporated, <www.ti.com>
 *
 * Venkateswara Rao Mandela <venkat.mandela@ti.com>
 *
 * SPDX-License-Identifier:	BSD-3-Clause
 */
#include <stdio.h>
#include <string.h>
#include "rsc_priv.h"
#include "utils.h"

struct fw_rsc_vdev_vring vring[2];
static int vring_mapped = 0;

void check_vring_init(void) {
  memset(&vring[0], 0x00, sizeof(vring[0])*2);
}

int check_vring(void) {
  return vring_mapped;
}

void check_vring_destroy(void) {
}

static int check_vdev(struct fw_rsc_vdev *rsc, int offset, int avail)
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

	debug("%10s rsc: id %d, dfeatures %x, cfg len %d, %d vrings\n",
	       "vdev",
	       rsc->id, rsc->dfeatures, rsc->config_len, rsc->num_of_vrings);

	/* we currently support only two vrings per rvdev */
	if (rsc->num_of_vrings > 2) {
		printf("too many vrings: %d\n", rsc->num_of_vrings);
		return -EINVAL;
	}

	for (i = 0; i < rsc->num_of_vrings; i++) {

          memcpy(&vring[i], &rsc->vring[i],
                 sizeof(struct fw_rsc_vdev_vring));

          debug("%18s rsc: num %d pa 0x%x da 0x%x \n",
                 "vring", rsc->vring[i].num,
                 0, rsc->vring[i].da);
	}

	return 0;
}

static int check_devmem(struct fw_rsc_devmem *rsc, int offset, int avail)
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

	debug("%10s rsc: pa 0x%08x, da 0x%08x, len 0x%08x\n",
	       "devmem",
	      rsc->pa, rsc->da, rsc->len);

        if (!vring_mapped) {

          int32_t i = 0;
          int32_t overlap = 0;
          for (i = 0; i < 2; i++,overlap<<=1) {
            debug("vring[i].da %08x rsc->da %08x rsc->da + rsc->len %08x\n",
                   vring[i].da,
                   rsc->da,
                   rsc->da + rsc->len);

            if ((vring[i].da >= rsc->da) &&
                (vring[i].da <= rsc->da + rsc->len)) {
              overlap++;
            }
          }
          if (overlap == 6)
            vring_mapped = 1;
        }
	return 0;
}

handle_resource_t vring_check_handlers[RSC_LAST] = {
  [RSC_CARVEOUT] = NULL,
  [RSC_DEVMEM] = (handle_resource_t) check_devmem,
  [RSC_TRACE] = NULL,
  [RSC_VDEV] = (handle_resource_t) check_vdev,
  [RSC_INTMEM] = NULL,
};
