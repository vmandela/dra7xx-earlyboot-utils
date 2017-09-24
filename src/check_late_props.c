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

const char *rp_late_node_names[3]= {"timers","iommus","watchdog-timers"};
const char *rp_late_prop_names[3]= {"ti,late-attach",
                                    "ti,no-idle-on-init",
                                    "ti,no-reset-on-init"};

int fdt_node_test_late_attach_props(void *fdt, int offset, const char *rp_name){

  int err=0;
  int i=0;
  const void *h;
  char *s[2] = { "not"," " };

  h = fdt_getprop(fdt,offset,
                  "ti,hwmods",NULL);
  for(i=0;i<3;i++){
    const void *ptr;
    char *stat_str = s[1];

    ptr = fdt_getprop(fdt,offset,
                      rp_late_prop_names[i],NULL);
    err = (err << 1);
    if(ptr == NULL) {
      err++;
      stat_str = s[0];
    }
    printf("%5s: %10s: %20s %3s found\n",
           rp_name, (char *) h,
           rp_late_prop_names[i],
           stat_str);
  }
  return err;
}

int fdt_test_rproc_node(void *fdt, int rp_offset) {
  int err;
  int j = 0;
  int lenp;
  const void *h;

  printf("\n");
  h = fdt_getprop(fdt,rp_offset,
                  "ti,hwmods",NULL);
  err = fdt_node_test_late_attach_props(fdt, rp_offset, h);


  /* Test tick timers, mmu's and watchdog timers */
  for(j = 0; j < 3; j++){
    unsigned char *ptr;
    int k = 0;
    uint32_t phandle[4];
    uint32_t n_offset;

    const char *pname = rp_late_node_names[j];

    ptr = (unsigned char *)fdt_getprop(fdt,rp_offset,
                                       pname,&lenp);
    if(!ptr) {
      printf("unable to find %s on %s: "
             "err code %d\n",pname,
             (char *) h, lenp);
      continue;
    }

    for (k=0;k<(lenp/4);k++) {
      int e1 = 0;

      phandle[k]=get_phandle(&ptr[4*k]);
      n_offset = fdt_node_offset_by_phandle(fdt,phandle[k]);
      e1 = fdt_node_test_late_attach_props(fdt, n_offset, h);
      err = (err << 3) + e1;
    }
  }

  return err;

}

int main(int argc, char *argv[]) {

  char *dt_name;
  void *fdt;
  int32_t i;
  char rproc_name[32];
  int32_t chosen_offset;

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
    sprintf(rproc_name,"rproc%d",i);
    chosen_offset = fdt_path_offset(fdt,rproc_name);
    if(chosen_offset<0){
      printf("%s not found\n",rproc_name);
      return 1;
    } else {
      fdt_test_rproc_node(fdt, chosen_offset);
    }
  }
  free(fdt);

  return 0;
}
