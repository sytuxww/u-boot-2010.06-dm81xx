/*
 * (C) Copyright 2009
 * Texas Instruments, <www.ti.com>
 *
 * Author :
 *      Manikandan Pillai <mani.pillai@ti.com>
 *
 * Derived from Beagle Board and 3430 SDP code by
 *      Richard Woodruff <r-woodruff2@ti.com>
 *      Syed Mohammed Khasim <khasim@ti.com>
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

#include <common.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/cpu.h>
#include <asm/arch/clock.h>

extern omap3_sysinfo sysinfo;

/******************************************
 * get_cpu_rev(void) - extract rev info
 ******************************************/
u32 get_cpu_rev(void)
{
	u32 id;
	u32 rev;

	id = readl(DEVICE_ID);
	rev = (id >> 28) & 0xF;

#ifdef CONFIG_TI814X
	/* PG2.1 devices should read 0x3 as chip rev
	 * Some PG2.1 devices have 0xc as chip rev
	 */
	if (0x3 == rev || 0xc == rev)
		return PG2_1;
	else
		return PG1_0;
#endif
	return rev;
}

/******************************************
 * get_cpu_type(void) - extract cpu info
 ******************************************/
u32 get_cpu_type(void)
{
	u32 id = 0;
	u32 partnum;

	id = readl(DEVICE_ID);
	partnum = (id >> 12) & 0xffff;

	return partnum;
}

/*************************************************************************
 * get_board_rev() - setup to pass kernel board revision information
 * returns:(bit[0-3] sub version, higher bit[7-4] is higher version)
 *************************************************************************/
u32 get_board_rev(void)
{
	return 0x0;
}

/*************************************************************
 *  get_device_type(): tell if GP/HS/EMU/TST
 *************************************************************/
u32 get_device_type(void)
{
	int mode;
	mode = __raw_readl(CONTROL_STATUS) & (DEVICE_MASK);
	return(mode >>= 8);
}

/************************************************
 * get_sysboot_value(void) - return SYS_BOOT[4:0]
 ************************************************/
u32 get_sysboot_value(void)
{
	int mode;
	mode = __raw_readl(CONTROL_STATUS) & (SYSBOOT_MASK);
	return mode;
}

/******************************************
 * pg_val_ti816x() - runtime PG ver detect
 ******************************************/
u32 pg_val_ti816x(u32 pg1_val, u32 pg2_val)
{
	/* TI816X PG1.0 devices should read 0x0 as chip rev
	 * TI816X PG1.1 devices should read 0x1 as chip rev
	 */
	if (0x1 == get_cpu_rev())
		return pg2_val;
	else
		return pg1_val;
}

/***************************************************
 * u32 pg_val_ti814x() - return the PG specifi value
 ***************************************************/
u32 pg_val_ti814x(u32 pg1_val, u32 pg2_val)
{
	/* PG2.1 devices should read 0x3 as chip rev */
	if (PG2_1 == get_cpu_rev())
		return pg2_val;
	else
		return pg1_val;
}

#ifdef CONFIG_DISPLAY_CPUINFO
/**
 * Print CPU information
 */
int print_cpuinfo (void)
{
	char *cpu_s, *sec_s;
	int arm_freq, ddr_freq , rev;

	rev = get_cpu_rev();
	switch (get_cpu_type()) {
	case TI8168:
		cpu_s = "8168";
		break;
	case TI8148:
		cpu_s = "8148";
		break;
	default:
		cpu_s = "Unknown cpu type";
		break;
	}

	switch (get_device_type()) {
	case TST_DEVICE:
		sec_s = "TST";
		break;
	case EMU_DEVICE:
		sec_s = "EMU";
		break;
	case HS_DEVICE:
		sec_s = "HS";
		break;
	case GP_DEVICE:
		sec_s = "GP";
		break;
	default:
		sec_s = "?";
	}

#ifdef CONFIG_TI816X
	printf("TI%s-%s rev 1.%d\n",
			cpu_s, sec_s, rev);
#else
	if (rev < PG_END) {
		char cpu_rev_str[3][4] = {"1.0", "2.1"}, *cpu_rev;

		cpu_rev = cpu_rev_str[rev];
		printf("TI%s-%s rev %s\n",
			cpu_s, sec_s, cpu_rev);
	} else {
		printf("TI%s-%s rev ?????[%1x]\n",
			cpu_s, sec_s, rev);
	}
#endif
	printf("\n");

	/* ARM and DDR frequencies */

#ifdef CONFIG_TI816X
	/* f0 = ((N * K) / (FREQ * P * M)) * fr */

	arm_freq = (((MAIN_N * FAPLL_K * OSC_FREQ)/
			( MAIN_P * MAIN_MDIV2 )))/SYSCLK_2_DIV;

	/*
	 * If the fractional part (MAIN_FRACFREQ2i) in non zero then the formula
	 * is : fo = ((N * K) / ( (INTFREQ + FRACFREQ) * p * M)) * fr
	 *
	 * For 13.824, INTFREQ = 0xD and FRACFREQ = 0xD2F1A9
	 * We are supposed to divide FRACFREQ by 0x1000000
	 *
	 * Due to overflow of values on multiplying by 0x1000000
	 * we discard the least 8 bits
	 */
	arm_freq = (arm_freq * (0x1000000 >> 8));
	arm_freq=arm_freq/((MAIN_INTFREQ2 * 0x1000000 + MAIN_FRACFREQ2)>>8);

	ddr_freq = ((DDR_N * OSC_FREQ)/DDR_MDIV1);
#else
	/* clk_out  = ((OSC_0/ ( N+1 )) * M) / M2   */
	arm_freq = ((OSC_0_FREQ / (MODENA_N + 1) * MODENA_M) / MODENA_M2);
	ddr_freq = ((OSC_0_FREQ / (DDR_N + 1) * DDR_M) / DDR_M2);
#endif
	printf("ARM clk: %dMHz\n", arm_freq);
	printf("DDR clk: %dMHz\n", ddr_freq);
	printf("\n");

	return 0;
}
#endif	/* CONFIG_DISPLAY_CPUINFO */
