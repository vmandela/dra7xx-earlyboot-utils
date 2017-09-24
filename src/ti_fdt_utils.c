/*
 * (C) Copyright 2017
 * Texas Instruments Incorporated, <www.ti.com>
 *
 * Venkateswara Rao Mandela <venkat.mandela@ti.com>
 *
 * SPDX-License-Identifier:	BSD-3-Clause
 */
#include <assert.h>
#include <arpa/inet.h>
#include "ti_fdt_utils.h"
#include "stdio.h"
#include "utils.h"

void find_cma_props(const struct fdt_property *prop,
                   uint32_t *cma_base,
                   uint32_t *cma_len) {
  int32_t j = 0;

  /* From kernel 3.14 addresses are specified as 64 bit instead
     of 32 bit. However since the SOC only supports 32 bit
     addressing, the first 32 bits of each address should be
     zero. Check for it first before proceeding.
  */
  uint32_t reg_arr_le[4];

  for(j=0; j<4; j++) {
    uint32_t reg_be = 0;
    reg_be = *((uint32_t *) (&prop->data[j*4]));
    reg_arr_le[j] = ntohl(reg_be);
    /* debug("reg_arr[%d] is 0x%08x\n", j, reg_arr_le[j]); */
  }
  assert(reg_arr_le[0] == 0);
  assert(reg_arr_le[2] == 0);
  *cma_len = reg_arr_le[3];
  *cma_base = reg_arr_le[1];
  /*  printf("cma_base 0x%08x: cma_len 0x%08x\n", *cma_base, *cma_len); */
  return;
}

int find_rproc_cma_props(const void *fdt,
                          int offset,
                          uint32_t *cma_base,
                          uint32_t *cma_len) {
    unsigned char *ptr;
    uint32_t phandle;
    uint32_t cma_off;
    const struct fdt_property *prop;
    int32_t lenp=0;

    ptr = (unsigned char *) fdt_getprop(fdt, offset,
                      "memory-region",NULL);

    if (ptr == NULL) {
      printf("CMA region node not found\n");
      return -1;
    }

    phandle = get_phandle(ptr);
    cma_off = fdt_node_offset_by_phandle(fdt, phandle);
    prop = fdt_get_property(fdt, cma_off,
                            "reg",&lenp);
    if(!prop)
      return -1;

    find_cma_props(prop, cma_base, cma_len);

    return 0;
}

int find_rproc_offset_by_name(const void *fdt, char *rproc_name) {

  int offset;
  char *comp_str[2] = { "ti,dra7-dsp", "ti,dra7-ipu" };
  int i = 0;
  const char *h;

  for (i = 0; i < 2; i++) {
    offset = -1;
    do {
      offset = fdt_node_offset_by_compatible(fdt, offset, comp_str[i]);
      if (offset >= 0) {
        h = (const char *)fdt_getprop(fdt, offset,
                                      "ti,hwmods",NULL);
        if (strcmp(h, rproc_name) == 0)
          return offset;
      }
    } while (offset != -FDT_ERR_NOTFOUND);
  }

  return (-FDT_ERR_NOTFOUND);
}

int find_rproc_cma_props_by_name(const void *fdt,
                                 char *rproc_name,
                                 uint32_t *cma_base,
                                 uint32_t *cma_len) {
  int offset;

  *cma_base = 0;
  *cma_len = 0;

  offset = find_rproc_offset_by_name(fdt, rproc_name);
  if (offset < 0)
    return -1;

  return find_rproc_cma_props(fdt, offset, cma_base, cma_len);
}
