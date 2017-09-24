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
#include "ti_fdt_utils.h"
#include "rsc_utils.h"

int main(int argc, char *argv[]) {

  char *dt_name;
  char *c_name;
  char *bin_path;
  void *fdt;
  void *bin;
  uint32_t cma_base, cma_len;
  int32_t ret;

  if (argc != 4) {
    printf("usage is\n"
           "%s <dtb path> <core name> <binary path>\n"
           "core name is one of ipu1, ipu2, dsp1, dsp2\n",argv[0]);
    exit(1);
  }

  dt_name = argv[1];
  fdt = read_file(dt_name);
  if(fdt == NULL) {
    printf("Could not open file %s\n\n",dt_name);
    goto error0;
  }

  c_name = argv[2];
  if(!is_core_name_valid(c_name))
    goto error0;

  bin_path = argv[3];
  bin = read_file(bin_path);
  if (bin == NULL) {
    printf("Could not open binary %s\n\n", bin_path);
    goto error0;
  }

  ret = find_rproc_cma_props_by_name(fdt, c_name,
                                     &cma_base, &cma_len);
  if(ret <0)
    goto error1;

  /* */
  ret = check_vring_address(bin, cma_base, cma_len);
  if(ret)
    printf("VRING's mapped correctly\n");
  else {
    printf("VRING's not mapepd correctly\n");
    print_rsc_table(bin);
  }

 error1:
  free(bin);
 error0:
  free(fdt);

  return 0;
}
