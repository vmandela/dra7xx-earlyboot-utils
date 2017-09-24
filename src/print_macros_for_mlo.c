/*
 * (C) Copyright 2017
 * Texas Instruments Incorporated, <www.ti.com>
 *
 * Venkateswara Rao Mandela <venkat.mandela@ti.com>
 *
 * SPDX-License-Identifier:	BSD-3-Clause
 */
#include <stdio.h>
#include <stdlib.h>
#include <libfdt.h>
#include "utils.h"
#include <ctype.h>
#define _BSD_SOURCE
#include <endian.h>
#include "ti_fdt_utils.h"

int main(int argc, char *argv[]) {

  char *dt_name;
  void *fdt;
  int32_t i;
  int32_t chosen_offset;
  uint32_t cma_base[NUM_RPROCS];
  uint32_t cma_len[NUM_RPROCS];
  char rproc_hwmod_name[NUM_RPROCS][32];
  int32_t rsv_mem_offset;
  int32_t lenp=0;
  int32_t rnode = 0;
  const struct fdt_property *prop;
  uint32_t pgtbl_base=0,pgtbl_len=0;

  if (argc != 2) {
    printf("usage is\n"
           "%s <dtb path>\n", argv[0]);
    exit(1);
  }
  dt_name = argv[1];
  fdt = read_file(dt_name);
  if(fdt == NULL) {
    printf("Could not open file %s\n\n",dt_name);
    exit(1);
  }

  printf("fdt length is %d\n",fdt_totalsize(fdt));

  for(i = 0;i < NUM_RPROCS; i++){

    const char *h;
    uint32_t j = 0;

    cma_base[i] = 0;
    cma_len[i] = 0;
    memset(&rproc_hwmod_name[i][0], 0x00, 32);

    chosen_offset = find_rproc_offset_by_name(fdt, rproc_names[i]);
    if(chosen_offset<0){
      printf("%s not found\n",rproc_names[i]);
      continue;
    }

    h = (const char *)fdt_getprop(fdt,chosen_offset,
                    "ti,hwmods",NULL);

    for (j = 0; j < strlen(h); j++)
      rproc_hwmod_name[i][j] = toupper(h[j]);

    find_rproc_cma_props(fdt, chosen_offset,
                         &cma_base[i], &cma_len[i]);

  }

  rsv_mem_offset = fdt_path_offset(fdt,"/reserved-memory");
  if(rsv_mem_offset<0){
    printf("Reservations not found\n");
    free(fdt);
    return 1;
  }

  rnode = fdt_subnode_offset(fdt, rsv_mem_offset,
                             "late_pgtbl");
  if (rnode > 0) {
    prop = fdt_get_property(fdt, rnode,
                            "reg",&lenp);
    find_cma_props(prop, &pgtbl_base, &pgtbl_len);
  } else {
    printf("late attach page table reservation not found\n");
  }

  for (i = 0; i < NUM_RPROCS; i++) {
    if(cma_base[i] == 0)
      continue;
    printf("%s_%s %20s 0x%08x\n",
           "#define DRA7_RPROC_CMA_BASE",
           rproc_hwmod_name[i],
           " ",
           cma_base[i]);
  }

  printf("\n");
  for (i = 0; i < NUM_RPROCS; i++) {
    if(cma_len[i] == 0)
      continue;
    printf("%s_%s %20s 0x%08x\n",
           "#define DRA7_RPROC_CMA_SIZE",
           rproc_hwmod_name[i],
           " ",
           cma_len[i]);
  }

  if(pgtbl_base) {
    printf("\n");
    printf("%s %24s 0x%08x\n",
           "#define DRA7_PGTBL_BASE_ADDR",
           " ",
           pgtbl_base);
  }

  free(fdt);

  return 0;
}
