/*
 * (C) Copyright 2017
 * Texas Instruments Incorporated, <www.ti.com>
 *
 * Venkateswara Rao Mandela <venkat.mandela@ti.com>
 *
 */
 /* Copyright (c) 2001 William L. Pitts
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms are freely
  * permitted provided that the above copyright notice and this
  * paragraph and the following disclaimer are duplicated in all
  * such forms.
  *
  * This software is provided "AS IS" and without any express or
  * implied warranties, including, without limitation, the implied
  * warranties of merchantability and fitness for a particular
  * purpose.
  */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elf.h>
#include <errno.h>
#include "remoteproc.h"
#include "rsc_utils.h"
#include "rsc_priv.h"
#include "utils.h"

static struct resource_table *rsc_table;

/* e_ident */
#define IS_ELF(ehdr) ((ehdr).e_ident[EI_MAG0] == ELFMAG0 && \
		      (ehdr).e_ident[EI_MAG1] == ELFMAG1 && \
		      (ehdr).e_ident[EI_MAG2] == ELFMAG2 && \
		      (ehdr).e_ident[EI_MAG3] == ELFMAG3)
/* ======================================================================
 * Determine if a valid ELF image exists at the given memory location.
 * First looks at the ELF header magic field, the makes sure that it is
 * executable and makes sure that it is for a PowerPC.
 * ====================================================================== */
int valid_elf_image(unsigned long addr)
{
	Elf32_Ehdr *ehdr;		/* Elf header structure pointer */

	/* -------------------------------------------------- */

	ehdr = (Elf32_Ehdr *) addr;

	if (!IS_ELF(*ehdr)) {
		printf("## No elf image at address 0x%08lx\n", addr);
		return 0;
	}

	if (ehdr->e_type != ET_EXEC) {
		printf("## Not a 32-bit elf image at address 0x%08lx\n", addr);
		return 0;
	}

#if 0
	if (ehdr->e_machine != EM_PPC) {
		printf("## Not a PowerPC elf image at address 0x%08lx\n", addr);
		return 0;
	}
#endif

	return 1;
}

void print_rsc_table_header(void) {
	printf("Resource table attributes\n");
	printf("%20s : %4d\n", "ver", rsc_table->ver);
	printf("%20s : %4d\n", "num", rsc_table->num);
	printf("%20s : %4d\n", "reserved[0]", rsc_table->reserved[0]);
	printf("%20s : %4d\n", "reserved[1]", rsc_table->reserved[1]);
	printf("%20s : %4d\n", "offset[0]", rsc_table->offset[0]);
	printf("\n");
}

/* handle firmware resource entries before booting the remote processor */
static int handle_resources(int len, handle_resource_t handlers[RSC_LAST],
			    int print_index)
{
	handle_resource_t handler;
	int ret = 0;
	unsigned int i;

	for (i = 0; i < rsc_table->num; i++) {
		int offset = rsc_table->offset[i];
		struct fw_rsc_hdr *hdr = (void *)rsc_table + offset;
		int avail = len - offset - sizeof(*hdr);
		void *rsc = (void *)hdr + sizeof(*hdr);

		/* make sure table isn't truncated */
		if (avail < 0) {
			printf("rsc table is truncated\n");
			return -EINVAL;
		}

		debug("rsc: index %d: type %d\n", i, hdr->type);

		if (hdr->type >= RSC_LAST) {
			printf("unsupported resource %d\n", hdr->type);
			continue;
		}

		if(print_index)
			printf(" [%2d] : ", i);
		handler = handlers[hdr->type];
		if (!handler)
			continue;

		ret = handler(rsc, offset + sizeof(*hdr), avail);
		if (ret)
			break;
	}

	return ret;
}

static Elf32_Shdr *find_table(void *addr)
{
	Elf32_Ehdr *ehdr;	/* Elf header structure pointer     */
	Elf32_Shdr *shdr;	/* Section header structure pointer */
	Elf32_Shdr sectionheader;
	unsigned int i;
	u8 *elf_data;
	char *name_table;
	struct resource_table *ptable;

	ehdr = (Elf32_Ehdr *) addr;
	elf_data = (u8 *) ehdr;
	shdr = (Elf32_Shdr *) (elf_data + ehdr->e_shoff);
	memcpy(&sectionheader, &shdr[ehdr->e_shstrndx], sizeof(sectionheader));
	name_table = (char *)(elf_data + sectionheader.sh_offset);

	for (i = 0; i < ehdr->e_shnum; i++, shdr++) {
		memcpy(&sectionheader, shdr, sizeof(sectionheader));
		u32 size = sectionheader.sh_size;
		u32 offset = sectionheader.sh_offset;

		if (strcmp(name_table + sectionheader.sh_name,
			   ".resource_table"))
			continue;

		ptable = (struct resource_table *)(elf_data + offset);

		/* make sure table has at least the header */
		if (sizeof(struct resource_table) > size) {
			printf("header-less resource table\n");
			return NULL;
		}

		debug("resource table version %d\n", ptable->ver);
		/* we don't support any version beyond the first */
		if (ptable->ver != 1) {
			printf("unsupported fw ver: %d\n", ptable->ver);
			return NULL;
		}

		/* make sure reserved bytes are zeroes */
		if (ptable->reserved[0] || ptable->reserved[1]) {
			printf("non zero reserved bytes\n");
			return NULL;
		}

		/* make sure the offsets array isn't truncated */
		if (ptable->num * sizeof(ptable->offset[0]) +
		    sizeof(struct resource_table) > size) {
			printf("resource table incomplete\n");
			return NULL;
		}

		return shdr;
	}

	return NULL;
}

struct resource_table *find_resource_table(void *addr,
					   int *tablesz)
{
	Elf32_Shdr *shdr;
	Elf32_Shdr sectionheader;
	struct resource_table *ptable;
	u8 *elf_data = (u8 *) addr;

	shdr = find_table(addr);
	if (!shdr) {
		printf
		    ("find_resource_table: failed to get resource section header\n");
		return NULL;
	}

	memcpy(&sectionheader, shdr, sizeof(sectionheader));
	ptable = (struct resource_table *)(elf_data + sectionheader.sh_offset);
	if (tablesz)
		*tablesz = sectionheader.sh_size;

	return ptable;
}

int print_rsc_table(void *addr)
{
	struct resource_table *ptable = NULL;
	int tablesz;
	int ret;

	ptable = find_resource_table(addr, &tablesz);
	if (!ptable) {
		printf("%s : failed to find resource table\n", __func__);
		return 0;
	}
	else {
		debug("%s : found resource table\n", __func__);

		rsc_table = malloc(tablesz);
		if (!rsc_table) {
			printf("resource table alloc failed!\n");
			return 0;
		}

		/* Copy the resource table into a local buffer
		 * before handling the resource table.
		 */
		memcpy(rsc_table, ptable, tablesz);

		print_rsc_table_header();
		ret = handle_resources(tablesz, printing_handlers, 1);
		if (ret) {
			printf("handle_resources failed: %d\n", ret);
			return 0;
		}

		/* Instead of trying to mimic the kernel flow of copying the
		 * processed resource table into its post ELF load location in
		 * DDR , I am copying it into its original location.
		 */
		memcpy(ptable, rsc_table, tablesz);

		free(rsc_table);
		rsc_table = NULL;
	}
	return 0;
}

int check_vring_address(void *addr, uint32_t cma_base,
			uint32_t cma_len)
{
	struct resource_table *ptable = NULL;
	int tablesz;
	int ret;

	check_vring_init();

	ptable = find_resource_table(addr, &tablesz);
	if (!ptable) {
		printf("%s : failed to find resource table\n", __func__);
		return 0;
	}
	else {
		debug("%s : found resource table\n", __func__);

		rsc_table = malloc(tablesz);
		if (!rsc_table) {
			printf("resource table alloc failed!\n");
			return 0;
		}

		/* Copy the resource table into a local buffer
		 * before handling the resource table.
		 */
		memcpy(rsc_table, ptable, tablesz);

		ret = handle_resources(tablesz, vring_check_handlers, 0);
		if (ret) {
			printf("handle_resources failed: %d\n", ret);
			return 0;
		}

		check_vring();

		/* Instead of trying to mimic the kernel flow of copying the
		 * processed resource table into its post ELF load location in
		 * DDR , I am copying it into its original location.
		 */
		memcpy(ptable, rsc_table, tablesz);

		free(rsc_table);
		rsc_table = NULL;
	}
	return check_vring();
}
