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
#include "rsc_utils.h"
#include "utils.h"

int main(int argc, char *argv[]) {

  char *fw_name = argv[1];
  void *mptr;

  if (argc!=2) {
    printf("usage is\n"
           "%s <firmware path>\n", argv[0]);
    exit(1);
  }

  mptr = read_file(argv[1]);
  if(mptr == NULL) {
    printf("Could not open file %s\n\n",fw_name);
    exit(1);
  }

  print_rsc_table(mptr);
  free(mptr);

  return 0;
}
