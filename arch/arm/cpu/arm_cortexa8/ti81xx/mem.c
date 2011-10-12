/*
 * (C) Copyright 2011 �����Žݵ������޹�˾
 *		luwei <sytu_xww@yahoo.com.cn>
 *
 * (C) Copyright 2010 Texas Instruments, <www.ti.com>
 *
 * Author :
 *     Mansoor Ahamed <mansoor.ahamed@ti.com>
 *
 * Initial Code from:
 *     Manikandan Pillai <mani.pillai@ti.com>
 *     Richard Woodruff <r-woodruff2@ti.com>
 *     Syed Mohammed Khasim <khasim@ti.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

// 0  2011��10��12��  ��������Ǻ����������NAND��ѡ�����߲���Ϊ����Ϊ16λ���ݣ���
//					  ����Ϊ8λ�����ߣ������ߵ�ʱ��ѡ��һ����

#include <common.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/mem.h>
#include <asm/arch/sys_proto.h>
#include <command.h>

/*
 * Only One NAND allowed on board at a time.
 * The GPMC CS Base for the same
 */
unsigned int boot_flash_base;
unsigned int boot_flash_off;
unsigned int boot_flash_sec;
unsigned int boot_flash_type;
volatile unsigned int boot_flash_env_addr;

struct gpmc *gpmc_cfg;

#if defined(CONFIG_CMD_NAND)

#if defined(CONFIG_NAND_MICRON)
static const u32 gpmc_m_nand[GPMC_MAX_REG] = {
	M_NAND_GPMC_CONFIG1,
	M_NAND_GPMC_CONFIG2,
	M_NAND_GPMC_CONFIG3,
	M_NAND_GPMC_CONFIG4,
	M_NAND_GPMC_CONFIG5,
	M_NAND_GPMC_CONFIG6, 0
};
#endif

#if defined(CONFIG_NAND_SAMSUNG)
static const u32 gpmc_m_nand[GPMC_MAX_REG] = {
	SMNAND_GPMC_CONFIG1,
	SMNAND_GPMC_CONFIG2,
	SMNAND_GPMC_CONFIG3,
	SMNAND_GPMC_CONFIG4,
	SMNAND_GPMC_CONFIG5,
	SMNAND_GPMC_CONFIG6, 0
};
#endif
#define GPMC_CS 0

#endif

/* 
 * ����: enable_gpmc_cs_config
 *
 * ����: 
 *		const u32 *gpmc_config  GPMC��������
 *		struct gpmc_cs *cs		CPU�Ĵ���
 *		u32 base				����ַ
 *		u32 size				��С
 *
 * ����ֵ: ��
 *
 * ����:
 * 		����gpmc_config�Ĳ������ã�д��GPMC��Ӧ��cs�Ĵ�����������NAND����ַ�ʹ�С��
 */
void enable_gpmc_cs_config(const u32 *gpmc_config, struct gpmc_cs *cs, u32 base,
			u32 size)
{
	writel(0, &cs->config7);
	sdelay(1000);
	/* Delay for settling */
	writel(gpmc_config[0], &cs->config1);
	writel(gpmc_config[1], &cs->config2);
	writel(gpmc_config[2], &cs->config3);
	writel(gpmc_config[3], &cs->config4);
	writel(gpmc_config[4], &cs->config5);
	writel(gpmc_config[5], &cs->config6);
	/* Enable the config */
	//����NAND����ַ�ʹ�С����ʹ��
	writel((((size & 0xF) << 8) | ((base >> 24) & 0x3F) |
		(1 << 6)), &cs->config7);
	sdelay(2000);
}

/* 
 * ����: gpmc_init
 *
 * ����: 
 *		��
 *
 * ����ֵ: ��
 *
 * ����:
 * 		����GPMC
 *		Init GPMC for x16, MuxMode (SDRAM in x32).
 * 		This code can only be executed from SRAM or SDRAM.
 * 		��ʼ��GPMC���ߣ�x16����
 *
 * TODO �޸�Ϊx8 for K9F2G08U0B 
 */
void gpmc_init(void)
{
	/* putting a blanket check on GPMC based on ZeBu for now */
	/* GPMC_BASE = 0x50000000 gpmc�ļĴ�����ַ */
	gpmc_cfg = (struct gpmc *)GPMC_BASE;

#ifdef CONFIG_NOR_BOOT
	/* env setup */
	boot_flash_base = CONFIG_SYS_FLASH_BASE;
	boot_flash_off = CONFIG_ENV_OFFSET;
	boot_flash_sec = NOR_SECT_SIZE;
	boot_flash_env_addr = boot_flash_base + boot_flash_off;
#else
#if defined(CONFIG_CMD_NAND) || defined(CONFIG_CMD_ONENAND)
	const u32 *gpmc_config = NULL;
	u32 base = 0;
	u32 size = 0;
#if defined(CONFIG_ENV_IS_IN_NAND) || defined(CONFIG_ENV_IS_IN_ONENAND)
	u32 f_off = CONFIG_SYS_MONITOR_LEN;
	u32 f_sec = 0;
#endif
#endif
	//-------------------------------------------------------------------//
	//
	//					ȫ������ 
	//
	//-------------------------------------------------------------------//
	/* SIDLEMODE = 0x1 No-idle. An idle request is never acknowledged */
	writel(0x00000008, &gpmc_cfg->sysconfig);
	/* WAIT0EDGEDETECTIONSTATUS = 1 ��λ */
	writel(0x00000100, &gpmc_cfg->irqstatus);
	/* WAIT1EDGEDETECTIONENABLE = 1 ʹ�ܱ��ش����ж�wait1 */
	writel(0x00000200, &gpmc_cfg->irqenable);
	/* 
	 * LIMITEDADDRESS = 1 ���Ƶ�ַʹ�� 
	 * WRITEPROTECT = 1   WP�ܽ�Ϊ��
	 */
	writel(0x00000012, &gpmc_cfg->config);
	/*
	 * Disable the GPMC0 config set by ROM code
	 */
	writel(0, &gpmc_cfg->cs[0].config7);
	sdelay(1000);

	//-------------------------------------------------------------------//
	//
	//					��дʱ������ 
	//
	//-------------------------------------------------------------------//
#if defined(CONFIG_CMD_NAND)	/* CS 0 */
	gpmc_config = gpmc_m_nand;

	base = PISMO1_NAND_BASE;
	size = PISMO1_NAND_SIZE;
	enable_gpmc_cs_config(gpmc_config, &gpmc_cfg->cs[0], base, size);
#if defined(CONFIG_ENV_IS_IN_NAND)
	f_off = MNAND_ENV_OFFSET;
	f_sec = (128 << 10);	/* 128 KiB */
	/* env setup */
	boot_flash_base = base;
	boot_flash_off = f_off;
	boot_flash_sec = f_sec;
	boot_flash_env_addr = f_off;
#endif
#endif

#ifdef CONFIG_ENV_IS_IN_SPI_FLASH
	/* env setup */
	boot_flash_base = 0x0;
	boot_flash_off = CONFIG_ENV_OFFSET;
	boot_flash_sec = CONFIG_ENV_SECT_SIZE;
	boot_flash_env_addr = CONFIG_ENV_OFFSET;
#endif

#endif
}


