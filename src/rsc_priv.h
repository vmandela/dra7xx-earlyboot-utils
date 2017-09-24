/*
 * (C) Copyright 2017
 * Texas Instruments Incorporated, <www.ti.com>
 *
 * Venkateswara Rao Mandela <venkat.mandela@ti.com>
 *
 * SPDX-License-Identifier:	BSD-3-Clause
 */
#ifndef __RSC_PRIV_H
#define __RSC_PRIV_H
#include <errno.h>
#include "remoteproc.h"

typedef int (*handle_resource_t) (void *, int offset, int avail);
extern handle_resource_t printing_handlers[RSC_LAST];
struct resource_table *find_resource_table(void *addr,
					   int *tablesz);
int valid_elf_image(unsigned long addr);

extern handle_resource_t vring_check_handlers[RSC_LAST];

int check_vring(void);
void check_vring_init(void);
void check_vring_destroy(void);

#endif
