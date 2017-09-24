/*
 * (C) Copyright 2017
 * Texas Instruments Incorporated, <www.ti.com>
 *
 * Venkateswara Rao Mandela <venkat.mandela@ti.com>
 *
 * SPDX-License-Identifier:	BSD-3-Clause
 */
#ifndef __TI_FDT_UTILS_H
#define __TI_FDT_UTILS_H

#include <libfdt.h>

/**
 * find the offset to a remoteproc node based on its name.
 * The name is compared against "ti,hwmods" property of the node.
 * valid inputs are "ipu1", "ipu2", "dsp1", "dsp2"
 *
 * returns offset to the node if found
 * returns -FDT_ERR_NOTFOUND if not found
 */
int find_rproc_offset_by_name(const void *fdt, char *rproc_name);

/**
 * Find the base address and length of a CMA node.
 */
void find_cma_props(const struct fdt_property *prop,
                    uint32_t *cma_base,
                    uint32_t *cma_len);

/**
 * Given an remoteproc node offset find its CMA region base address and length.
 */
int find_rproc_cma_props(const void *fdt,
                          int offset,
                          uint32_t *cma_base,
                          uint32_t *cma_len);

int find_rproc_cma_props_by_name(const void *fdt,
                                 char *rproc_name,
                                 uint32_t *cma_base,
                                 uint32_t *cma_len);
#endif
