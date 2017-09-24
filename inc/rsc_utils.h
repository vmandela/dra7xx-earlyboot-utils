/*
 * (C) Copyright 2017
 * Texas Instruments Incorporated, <www.ti.com>
 *
 * Venkateswara Rao Mandela <venkat.mandela@ti.com>
 *
 * SPDX-License-Identifier:	BSD-3-Clause
 */
#ifndef RSC_TABLE_H_
#define RSC_TABLE_H_
#include <stdint.h>

struct mmu_mapping {
  uint32_t pa;
  uint32_t va;
  uint32_t len;
};

typedef struct {
  uint32_t cnt;
  struct mmu_mapping *mappings;
} map_array;

int print_rsc_table(void *addr);


map_array *calc_mmu_mapping(void *bin_ptr,
                            uint32_t cma_base,
                            uint32_t cma_len);

int check_vring_address(void *addr, uint32_t cma_base,
			uint32_t cma_len);


#endif
