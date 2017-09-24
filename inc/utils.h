/*
 * (C) Copyright 2017
 * Texas Instruments Incorporated, <www.ti.com>
 *
 * Venkateswara Rao Mandela <venkat.mandela@ti.com>
 *
 * SPDX-License-Identifier:	BSD-3-Clause
 */
#ifndef __UTILS_H
#define __UTILS_H
#include <stdint.h>

extern void *read_file(char *filename);
extern uint32_t get_phandle(uint8_t *ptr);
extern uint32_t is_core_name_valid(char *cname);

#define NUM_RPROCS (4)
extern char *rproc_names[NUM_RPROCS];

#ifdef DEBUG
#define debug printf
#else
#define debug(...)
#endif

#endif
