/*
 *  Copyright (c) 2013 Qualcomm Atheros, Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/bootmem.h>

#include <asm/bootinfo.h>
#include <asm/addrspace.h>

#include <atheros.h>

int __ath_flash_size;

void __init prom_init(void)
{
	int memsz = 0x2000000;
#if 0
	int argc = fw_arg0, i;
	char **arg = (char **)fw_arg1;
#endif

	/* pass the Ram size from bootloader, so we do not need parse option [mem=xxx] of the kernel command line */
    printk("Ram size passed from bootloader =%ldM\n", (fw_arg2/(1024*1024)));
    memsz = (int)fw_arg2;

	printk("flash_size passed from bootloader = %ld\n", fw_arg3);
	__ath_flash_size = fw_arg3;

#if 0
	/*
	 * if user passes kernel args, ignore the default one
	 */
	if (argc > 1) {
		arcs_cmdline[0] = '\0';

		for (i = 1; i < argc; i++)
			printk("arg %d: %s\n", i, arg[i]);

		/*
		 * arg[0] is "g", the rest is boot parameters
		 */
		for (i = 1; i < argc; i++) {
			if (strlen(arcs_cmdline) + strlen(arg[i] + 1)
			    >= sizeof(arcs_cmdline))
				break;
			strcat(arcs_cmdline, arg[i]);
			strcat(arcs_cmdline, " ");
		}
	}
#endif

	// mips_machgroup = MACH_GROUP_ATHEROS;

	mips_machtype = CONFIG_ATH_MACH_TYPE;

	/*
	 * By default, use all available memory. You can override this
	 * to use, say, 8MB by specifying "mem=8M" as an argument on the
	 * linux bootup command line.
	 */
	add_memory_region(0, memsz, BOOT_MEM_RAM);
}

void __init prom_free_prom_memory(void)
{
}
