/*
 * (C) Copyright 2004-2008
 * Texas Instruments, <www.ti.com>
 * Richard Woodruff <r-woodruff2@ti.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR /PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
  */
#ifndef _SYS_PROTO_H_
#define _SYS_PROTO_H_

typedef struct {
	u32 board_type_v1;
	u32 board_type_v2;
	u32 mtype;
	char *board_string;
	char *nand_string;
} omap3_sysinfo;

/* CPU Revision for TI814X PG2.1 is 0x3 and PG1.0 is0 */
enum cpu_rev {
	PG1_0 = 0,
	PG2_1,
	PG_END
};

void prcm_init(u32);
void per_clocks_enable(void);
void gpmc_init(void);
void watchdog_init(void);
void set_muxconf_regs(void);
u32 get_cpu_rev(void);
u32 get_mem_type(void);
u32 get_sysboot_value(void);
int print_cpuinfo (void);
u32 is_gpmc_muxed(void);
u32 get_gpmc0_type(void);
u32 get_gpmc0_width(void);
u32 get_board_type(void);
void display_board_info(u32);
u32 get_sdr_cs_size(u32);
u32 get_sdr_cs_offset(u32);
u32 get_device_type(void);
u32 get_boot_type(void);
void sr32(void*, u32 , u32 , u32);
u32 wait_on_value(u32, u32, void*, u32);
void sdelay(unsigned long);
void omap_nand_switch_ecc(int);
void power_init_r(void);
void invalidate_dcache(u32);
u32 pg_val_ti816x(u32, u32);
u32 pg_val_ti814x(u32, u32);
#endif

