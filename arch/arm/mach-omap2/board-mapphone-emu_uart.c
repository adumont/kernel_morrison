/*
 * board-mapphone-emu_uart.c
 *
 * Copyright (C) 2009 Motorola, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* Date         Author          Comment
 * ===========  ==============  ==============================================
 * Jun-26-2009  Motorola	Initial revision.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/io.h>

#include <linux/spi/spi.h>
#include <mach/system.h>
#include <linux/irq.h>

#include <mach/dma.h>
#include <mach/clock.h>
#include <mach/board-mapphone-emu_uart.h>
#include <mach/hardware.h>
#include <mach/omap34xx.h>

/*
 * Register definitions for CPCAP related SPI register
 */
#define OMAP2_MCSPI_MAX_FREQ		48000000

#define OMAP2_MCSPI_REVISION		0x00
#define OMAP2_MCSPI_SYSCONFIG		0x10
#define OMAP2_MCSPI_SYSSTATUS		0x14
#define OMAP2_MCSPI_IRQSTATUS		0x18
#define OMAP2_MCSPI_IRQENABLE		0x1c
#define OMAP2_MCSPI_WAKEUPENABLE	0x20
#define OMAP2_MCSPI_SYST		0x24
#define OMAP2_MCSPI_MODULCTRL		0x28

/* per-channel banks, 0x14 bytes each, first is: */
#define OMAP2_MCSPI_CHCONF0		0x2c
#define OMAP2_MCSPI_CHSTAT0		0x30
#define OMAP2_MCSPI_CHCTRL0		0x34
#define OMAP2_MCSPI_TX0			0x38
#define OMAP2_MCSPI_RX0			0x3c

/* per-register bitmasks: */

#define OMAP2_MCSPI_SYSCONFIG_AUTOIDLE	(1 << 0)
#define OMAP2_MCSPI_SYSCONFIG_SOFTRESET	(1 << 1)
#define OMAP2_AFTR_RST_SET_MASTER	(0 << 2)

#define OMAP2_MCSPI_SYSSTATUS_RESETDONE	(1 << 0)
#define OMAP2_MCSPI_SYS_CON_LVL_1 1
#define OMAP2_MCSPI_SYS_CON_LVL_2 2

#define OMAP2_MCSPI_MODULCTRL_SINGLE	(1 << 0)
#define OMAP2_MCSPI_MODULCTRL_MS	(1 << 2)
#define OMAP2_MCSPI_MODULCTRL_STEST	(1 << 3)

#define OMAP2_MCSPI_CHCONF_PHA		(1 << 0)
#define OMAP2_MCSPI_CHCONF_POL		(1 << 1)
#define OMAP2_MCSPI_CHCONF_CLKD_MASK	(0x0f << 2)
#define OMAP2_MCSPI_CHCONF_EPOL		(1 << 6)
#define OMAP2_MCSPI_CHCONF_WL_MASK	(0x1f << 7)
#define OMAP2_MCSPI_CHCONF_TRM_RX_ONLY	(0x01 << 12)
#define OMAP2_MCSPI_CHCONF_TRM_TX_ONLY	(0x02 << 12)
#define OMAP2_MCSPI_CHCONF_TRM_MASK	(0x03 << 12)
#define OMAP2_MCSPI_CHCONF_TRM_TXRX	(~OMAP2_MCSPI_CHCONF_TRM_MASK)
#define OMAP2_MCSPI_CHCONF_DMAW		(1 << 14)
#define OMAP2_MCSPI_CHCONF_DMAR		(1 << 15)
#define OMAP2_MCSPI_CHCONF_DPE0		(1 << 16)
#define OMAP2_MCSPI_CHCONF_DPE1		(1 << 17)
#define OMAP2_MCSPI_CHCONF_IS		(1 << 18)
#define OMAP2_MCSPI_CHCONF_TURBO	(1 << 19)
#define OMAP2_MCSPI_CHCONF_FORCE	(1 << 20)

#define OMAP2_MCSPI_SYSCFG_WKUP		(1 << 2)
#define OMAP2_MCSPI_SYSCFG_IDL		(2 << 3)
#define OMAP2_MCSPI_SYSCFG_CLK		(2 << 8)
#define OMAP2_MCSPI_WAKEUP_EN		(1 << 1)
#define OMAP2_MCSPI_IRQ_WKS		(1 << 16)
#define OMAP2_MCSPI_CHSTAT_RXS		(1 << 0)
#define OMAP2_MCSPI_CHSTAT_TXS		(1 << 1)
#define OMAP2_MCSPI_CHSTAT_EOT		(1 << 2)

#define OMAP2_MCSPI_CHCTRL_EN		(1 << 0)
#define OMAP2_MCSPI_MODE_IS_MASTER	0
#define OMAP2_MCSPI_MODE_IS_SLAVE	1
#define OMAP_MCSPI_WAKEUP_ENABLE	1

#define OMAP_MCSPI_BASE                 IO_ADDRESS(0x48098000)

#define WORD_LEN            32
#define CLOCK_DIV           12	/* 2^(12)=4096  48000000/4096<19200 */

#define LEVEL1              1
#define LEVEL2              2
#define WRITE_CPCAP         1
#define READ_CPCAP          0

#define CM_ICLKEN1_CORE  IO_ADDRESS(0x48004A10)
#define CM_FCLKEN1_CORE  IO_ADDRESS(0x48004A00)
#define OMAP2_MCSPI_EN_MCSPI1   (1 << 18)

#define  RESET_FAIL      1
#define RAW_MOD_REG_BIT(val, mask, set) do { \
    if (set) \
		val |= mask; \
    else \
		 val &= ~mask; \
} while (0)

struct cpcap_dev {
	u16 address;
	u16 value;
	u32 result;
	int access_flag;
};

static char tx[4];
static bool emu_uart_is_active = FALSE;

/*   Although SPI driver is provided through linux system as implemented above,
 *   it can not cover some special situation.
 *
 *   During Debug phase, OMAP may need to acess CPCAP by SPI
 *   to configure or check related register when boot up is not finished.
 *   However, at this time, spi driver integrated in linux system may not
 *   be initialized properly.
 *
 *   So we provode the following SPI driver with common API for access capcap
 *   by SPI directly, i.e. we will skip the linux system driver,
 *   but access SPI hardware directly to configure read/write specially for
 *   cpcap access.
 *
 *   So developer should be very careful to use these APIs:
 *
 *        read_cpcap_register_raw()
 *        write_cpcap_register_raw()
 *
 *   Pay attention: Only use them when boot up phase.
 *   Rasons are as follows:
 *   1. Although we provide protection on these two APIs for concurrency and
 *      race conditions, it may impact the performance of system
 *      because it will mask all interrupts during access.
 *   2. Calling these APIs will reset all SPI registers, and may make previous
 *      data lost during run time.
 *
 *      So, if developer wants to access CPCAP after boot up is finished,
 *      we suggest they should use poweric interface.
 *
 */

static inline void raw_writel_reg(u32 value, u32 reg)
{
#if defined(LOCAL_DEVELOPER_DEBUG)
	unsigned int absolute_reg;

	absolute_reg = OMAP_MCSPI_BASE + reg;
	printk(KERN_ERR " raw write reg =0x%x value=0x%x \n", absolute_reg,
	       value);
#endif

	__raw_writel(value, OMAP_MCSPI_BASE + reg);
}

static inline u32 raw_readl_reg(u32 reg)
{
	u32 result;
	unsigned int absolute_reg;
	absolute_reg = (u32)OMAP_MCSPI_BASE + reg;
#if defined(LOCAL_DEVELOPER_DEBUG)
	printk(KERN_ERR " raw read reg =0x%x  \n", absolute_reg);
#endif
	result = __raw_readl(absolute_reg);
#if defined(LOCAL_DEVELOPER_DEBUG)
	printk(KERN_ERR " raw read reg =0x%x result =0x%x  \n",
	       absolute_reg, result);
#endif
	return result;
}

static void raw_omap_mcspi_wakeup_enable(int level)
{
	u32 result;

	/* configure SYSCONFIG register...  */
	if (level == LEVEL1) {
		result = raw_readl_reg(OMAP2_MCSPI_SYSCONFIG);
		result =
		    result | OMAP2_MCSPI_SYSCFG_WKUP |
		    OMAP2_MCSPI_SYSCFG_IDL | OMAP2_MCSPI_SYSCFG_CLK |
		    OMAP2_MCSPI_SYSCONFIG_AUTOIDLE;
		raw_writel_reg(result, OMAP2_MCSPI_SYSCONFIG);
	}

	if (level == LEVEL2) {
		result = raw_readl_reg(OMAP2_MCSPI_SYSCONFIG);
		result =
		    result | OMAP2_MCSPI_SYSCFG_WKUP |
		    OMAP2_MCSPI_SYSCFG_IDL |
		    OMAP2_MCSPI_SYSCONFIG_AUTOIDLE;
		RAW_MOD_REG_BIT(result, OMAP2_MCSPI_SYSCFG_CLK, 0);
		raw_writel_reg(result, OMAP2_MCSPI_SYSCONFIG);
	}

	/* configure wakeupenable register...  */
	raw_writel_reg(OMAP2_MCSPI_WAKEUP_EN, OMAP2_MCSPI_WAKEUPENABLE);

	/* configure enable interrupt register... */
	result = raw_readl_reg(OMAP2_MCSPI_IRQENABLE);
	result = result | OMAP2_MCSPI_IRQ_WKS;
	raw_writel_reg(result, OMAP2_MCSPI_IRQENABLE);
}

static void raw_omap2_mcspi_set_master_mode(void)
{
	u32 result;

	/* configure MCSPI_MODULCTRL register... */
	result = raw_readl_reg(OMAP2_MCSPI_MODULCTRL);

	RAW_MOD_REG_BIT(result, OMAP2_MCSPI_MODULCTRL_STEST, 0);
	RAW_MOD_REG_BIT(result, OMAP2_MCSPI_MODULCTRL_MS,
			OMAP2_MCSPI_MODE_IS_MASTER);
	RAW_MOD_REG_BIT(result, OMAP2_MCSPI_MODULCTRL_SINGLE, 1);

	raw_writel_reg(result, OMAP2_MCSPI_MODULCTRL);
}

static void raw_omap2_mcspi_channel_config(void)
{
	u32 result;

	/* select channel 0... otherwise 0x14*channel_num */
	result = raw_readl_reg(OMAP2_MCSPI_CHCONF0);

	/* configure master mode... */
	result &= ~OMAP2_MCSPI_CHCONF_IS;
	result &= ~OMAP2_MCSPI_CHCONF_DPE1;
	result |= OMAP2_MCSPI_CHCONF_DPE0;

	/* configure wordlength  */
	result &= ~OMAP2_MCSPI_CHCONF_WL_MASK;
	result |= (WORD_LEN - 1) << 7;

	/*  configure active high  */
	result &= ~OMAP2_MCSPI_CHCONF_EPOL;

	/* set clock divisor  */
	result &= ~OMAP2_MCSPI_CHCONF_CLKD_MASK;
	result |= CLOCK_DIV << 2;

	/* configure mode  polarity=0 phase=0  */
	result &= ~OMAP2_MCSPI_CHCONF_POL;
	result &= ~OMAP2_MCSPI_CHCONF_PHA;

	raw_writel_reg(result, OMAP2_MCSPI_CHCONF0);

}

static void raw_mcspi_setup(void)
{
	raw_omap_mcspi_wakeup_enable(LEVEL1);
	raw_omap2_mcspi_set_master_mode();
	raw_omap2_mcspi_channel_config();
	raw_omap_mcspi_wakeup_enable(LEVEL2);
}

static int raw_mcspi_reset(void)
{
	unsigned long timeout;
	u32 tmp;

	raw_omap_mcspi_wakeup_enable(LEVEL1);

	raw_writel_reg(OMAP2_MCSPI_SYSCONFIG_SOFTRESET,
		      OMAP2_MCSPI_SYSCONFIG);

	timeout = jiffies + msecs_to_jiffies(1000);

	do {
		tmp = raw_readl_reg(OMAP2_MCSPI_SYSSTATUS);
		if (time_after(jiffies, timeout)) {
			printk(KERN_ERR "SPI Error: Reset is time out!\n");
			return -RESET_FAIL;
		}
	} while (!(tmp & OMAP2_MCSPI_SYSSTATUS_RESETDONE));

	/*configure all modules in  reset master mode */
	raw_writel_reg(OMAP2_AFTR_RST_SET_MASTER, OMAP2_MCSPI_MODULCTRL);

	/* call wakeup function to set sysconfig as per pm activity */
	raw_omap_mcspi_wakeup_enable(LEVEL1);
	raw_omap_mcspi_wakeup_enable(LEVEL2);

	return 0;
}

static void raw_omap2_mcspi_force_cs(int enable_tag)
{
	u32 result;
	result = raw_readl_reg(OMAP2_MCSPI_CHCONF0);
	/*
	 * Manual spim_csx assertion to keep spim_csx for channel x active
	 * RW 0x0 between SPI words (single channel master mode only).
	 */
	RAW_MOD_REG_BIT(result, OMAP2_MCSPI_CHCONF_FORCE, enable_tag);
	raw_writel_reg(result, OMAP2_MCSPI_CHCONF0);
}

static void raw_omap2_mcspi_set_enable(int enable)
{
	u32 result;

	result = enable ? OMAP2_MCSPI_CHCTRL_EN : 0;
	raw_writel_reg(result, OMAP2_MCSPI_CHCTRL0);
}


static int raw_mcspi_wait_for_reg_bit(unsigned long reg, unsigned long bit)
{
	unsigned long timeout;

	timeout = jiffies + msecs_to_jiffies(1000);

	while (!(raw_readl_reg(reg) & bit)) {
		if (time_after(jiffies, timeout))
			return -1;
	}

	return 0;
}

static void parser_cpcap(struct cpcap_dev *dev)
{
	if (dev->access_flag == WRITE_CPCAP) {
		tx[3] = ((dev->address >> 6) & 0x000000FF) | 0x80;
		tx[2] = (dev->address << 2) & 0x000000FF;
		tx[1] = (dev->value >> 8) & 0x000000FF;
		tx[0] = dev->value & 0x000000FF;
	} else {
		tx[3] = ((dev->address >> 6) & 0x000000FF);
		tx[2] = (dev->address << 2) & 0x000000FF;
		tx[1] = 1;
		tx[0] = 1;
	}
}

static void raw_omap2_mcspi_txrx_pio(struct cpcap_dev *dev)
{
	u32 result;
	u32 tx_32bit;

	/* config tranmission mode --- tx rx together */
	result = raw_readl_reg(OMAP2_MCSPI_CHCONF0);
	result &= ~OMAP2_MCSPI_CHCONF_TRM_MASK;
	raw_writel_reg(result, OMAP2_MCSPI_CHCONF0);

	/* enable the mcspi port! */
	raw_omap2_mcspi_set_enable(1);

	parser_cpcap(dev);

	memcpy((void *)&tx_32bit, (void *)tx, 4);

	if (raw_mcspi_wait_for_reg_bit(OMAP2_MCSPI_CHSTAT0,
				       OMAP2_MCSPI_CHSTAT_TXS) < 0) {
		printk(KERN_ERR "SPI Error: TXS timed out\n");
		goto out;
	}
	raw_writel_reg(tx_32bit, OMAP2_MCSPI_TX0);

	if (raw_mcspi_wait_for_reg_bit(OMAP2_MCSPI_CHSTAT0,
				       OMAP2_MCSPI_CHSTAT_RXS) < 0) {
		printk(KERN_ERR "SPI Error: RXS timed out\n");
		goto out;
	}

	result = raw_readl_reg(OMAP2_MCSPI_RX0);

	dev->result = result;

out:
	/* disable the mcspi port! */
	raw_omap2_mcspi_set_enable(0);
}

static void raw_mcspi_run(struct cpcap_dev *dev)
{
	raw_omap_mcspi_wakeup_enable(LEVEL1);
	raw_omap2_mcspi_set_master_mode();
	raw_omap2_mcspi_channel_config();
	raw_omap2_mcspi_force_cs(1);
	raw_omap2_mcspi_txrx_pio(dev);
	raw_omap2_mcspi_force_cs(0);
	raw_omap_mcspi_wakeup_enable(LEVEL2);
}

static void raw_omap_mcspi_enable_IFclock(void)
{
	u32 result;

	result = __raw_readl(CM_FCLKEN1_CORE);
	RAW_MOD_REG_BIT(result, OMAP2_MCSPI_EN_MCSPI1, 1);
	__raw_writel(result, CM_FCLKEN1_CORE);

	result = __raw_readl(CM_ICLKEN1_CORE);
	RAW_MOD_REG_BIT(result, OMAP2_MCSPI_EN_MCSPI1, 1);
	__raw_writel(result, CM_ICLKEN1_CORE);

}

/*
 * write_cpcap_register_raw is for cpcap spi write directly
 * @return 0 on success; less than zero on failure.
 */
static int write_cpcap_register_raw(u16 addr, u16 val)
{
	int result;
	unsigned long intr_flags;
	struct cpcap_dev cpcap_write;

#ifdef CONFIG_EMU_UART_DEBUG
	if (is_emu_uart_active() && (addr == 897 || addr == 411))
		return 0;
#endif

	local_irq_save(intr_flags);
	raw_omap_mcspi_enable_IFclock();

	result = raw_mcspi_reset();
	if (result < 0) {
		local_irq_restore(intr_flags);
		printk(KERN_ERR "reset failed !\n");
		return result;
	}

	raw_mcspi_setup();

	cpcap_write.address = addr;
	cpcap_write.value = val;
	cpcap_write.access_flag = WRITE_CPCAP;
	raw_mcspi_run(&cpcap_write);

	local_irq_restore(intr_flags);

	return result;
}

/*
 *  read_cpcap_register_raw is for cpcap spi read directly,
 *  read result is in val
 *  @return 0 on success; less than zero on failure.
 */
static int read_cpcap_register_raw(u16 addr, u16 *val)
{
	int result;
	unsigned long intr_flag;
	struct cpcap_dev cpcap_read;

	local_irq_save(intr_flag);
	raw_omap_mcspi_enable_IFclock();

	result = raw_mcspi_reset();
	if (result < 0) {
		local_irq_restore(intr_flag);
		printk(KERN_ERR "reset failed !\n");
		return result;
	}

	raw_mcspi_setup();

	cpcap_read.address = addr;
	cpcap_read.access_flag = READ_CPCAP;
	raw_mcspi_run(&cpcap_read);
	*val = cpcap_read.result;

	local_irq_restore(intr_flag);

	return result;
}

/*
 * Check if the writting is allowed. If MiniUSB port has already been
 * configured as UART3, we should ignore some SCM register writting.
 */
int is_emu_uart_iomux_reg(unsigned short offset)
{
	if ((emu_uart_is_active) && \
	    ((offset >= 0x1A2 && offset < 0x1BA) || (offset == 0x19E)))
		return 1;
	else
		return 0;
}

bool is_emu_uart_active(void)
{
	return emu_uart_is_active;
}

static void write_omap_mux_register(u16 offset, u8 mode, u8 input_en)
{
	u16 tmp_val, reg_val;
	u32 reg = OMAP343X_CTRL_BASE + offset;

	reg_val = mode | (input_en << 8);
	tmp_val = omap_readw(reg) & ~(0x0007 | (1 << 8));
	reg_val = reg_val | tmp_val;
	omap_writew(reg_val, reg);
}

void activate_emu_uart(void)
{
	int i;

	/*
	 * Step 1:
	 * Configure OMAP SCM to set all ULPI pin of USB OTG to SAFE MODE
	 */
	for (i = 0; i < 0x18; i += 2)
		write_omap_mux_register(0x1A2 + i, 7, 0);

	/*
	 * Step 2:
	 * Configure CPCAP to route UART3 to USB port; Switch VBUSIN to supply
	 * UART/USB transeiver and set VBUS standby mode 3
	 */
	write_cpcap_register_raw(897, 0x0101);
	write_cpcap_register_raw(411, 0x014C);

	/* Step 3:
	 * Configure OMAP SCM to set ULPI port as UART3 function
	 */
	/*
	 * Set UART3 RX pin in safe mode
	 */
	write_omap_mux_register(0x19E, 7, 0);
	/*
	 * Route UART3 TX to ULPIDATA0, RX to ULPIDATA1
	 */
	write_omap_mux_register(0x1AA, 2, 0);
	write_omap_mux_register(0x1AC, 2, 1);

	emu_uart_is_active = TRUE;
	printk
	    (KERN_ALERT "WARNING: MiniUSB port works in UART3 mode,"
	     "the USB functionality UNAVAILABLE!\n");

}
