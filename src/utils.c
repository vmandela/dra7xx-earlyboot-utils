/*
 * (C) Copyright 2017
 * Texas Instruments Incorporated, <www.ti.com>
 *
 * Venkateswara Rao Mandela <venkat.mandela@ti.com>
 *
 * SPDX-License-Identifier:	BSD-3-Clause
 */
#include "utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>

char *rproc_names[NUM_RPROCS] = { "ipu1", "ipu2", "dsp1", "dsp2" };

void * read_file(char *filename) {

  FILE *fp;
  void *mptr = NULL;
  size_t file_size=1024;
  size_t nread;


  fp=fopen(filename,"rb");
  if(fp==NULL) {
    printf("Could not open file %s\n\n",filename);
    goto err_ret;
    /*
      } else {
      printf("Loaded file %s\n\n",filename);
    */
  }
  fseek(fp,0,SEEK_END);
  file_size = ftell(fp);
  fseek(fp,0,SEEK_SET);

  /*
  printf("%s found; size is %d bytes\n",filename,file_size);
  */
  mptr = (void *)malloc(file_size);
  assert(mptr!=NULL);
  memset(mptr,0x00,file_size);

  nread = fread(mptr,sizeof(uint8_t),file_size,fp);

  if(nread != file_size){
    printf("file %s : partially read %lu/%lu\n",
           filename,
           nread, file_size);
  }

  fclose(fp);
 err_ret:
  return mptr;
}

uint32_t get_phandle(uint8_t *ptr) {
  uint32_t t;

  t = *((uint32_t *) ptr);
  return ntohl(t);
}

uint32_t is_core_name_valid(char *cname) {

  int32_t i = 0;

  for (i = 0; i < NUM_RPROCS; i++) {
    if(strcmp(cname, rproc_names[i]) == 0)
      return 1;
  }
  return 0;
}
