/* linux/arch/arm/mach-msm/board-mahimahi.c
 *
 * Copyright (C) 2009 Google, Inc.
 * Copyright (C) 2009 HTC Corporation.
 * Author: Dima Zavin <dima@android.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/usb/android_composite.h>
#if 0
#include <linux/usb/f_accessory.h>
#endif

#include <linux/android_pmem.h>
#include <linux/synaptics_i2c_rmi.h>
#include <linux/a1026.h>
#include <linux/capella_cm3602.h>
#include <linux/akm8973.h>
#include <linux/regulator/machine.h>
#include <linux/ds2784_battery.h>
#include <../../../drivers/staging/android/timed_gpio.h>
#include <../../../drivers/w1/w1.h>

#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/setup.h>

#include <mach/board.h>
#include <mach/hardware.h>
#if 0
#include <mach/msm_hsusb.h>
#endif
#include <mach/msm_iomap.h>
#include <mach/msm_serial_debugger.h>
#include <mach/system.h>
#include <mach/msm_serial_hs.h>
#include <mach/bcm_bt_lpm.h>
#include <mach/msm_smd.h>
#include <mach/msm_flashlight.h>
#ifdef CONFIG_PERFLOCK
#include <mach/perflock.h>
#endif
#include <mach/socinfo.h>
#include "board-mahimahi.h"
#include "devices.h"
#include "proc_comm.h"
#include "board-mahimahi-tpa2018d1.h"
#include "board-mahimahi-smb329.h"

#include <linux/msm_kgsl.h>
#include <linux/regulator/machine.h>
#include "footswitch.h"
#include <mach/msm_memtypes.h>
#include "acpuclock.h"
#include <linux/spi/spi.h>
#include <linux/bma150.h>
#include <linux/usb/composite.h>
#include <mach/dma.h>
#include <mach/htc_usb.h>
#ifdef CONFIG_USB_MSM_OTG_72K
#include <mach/msm_hsusb.h>
#else
#include <linux/usb/msm_hsusb.h>
#endif
#include <mach/msm_hsusb_hw.h>
#include "pm.h"
#include "irq.h"
#include <mach/msm_spi.h>
#include "pm-boot.h"


static uint debug_uart;

module_param_named(debug_uart, debug_uart, uint, 0);

extern void notify_usb_connected(int);
extern void msm_init_pmic_vibrator(void);
extern void __init mahimahi_audio_init(void);

extern int microp_headset_has_mic(void);

#if 0
static int mahimahi_phy_init_seq[] = {
	0x0C, 0x31,
	0x31, 0x32,
	0x1D, 0x0D,
	0x1D, 0x10,
	-1 };

static void mahimahi_usb_phy_reset(void)
{
	u32 id;
	int ret;

	id = PCOM_CLKRGM_APPS_RESET_USB_PHY;
	ret = msm_proc_comm(PCOM_CLK_REGIME_SEC_RESET_ASSERT, &id, NULL);
	if (ret) {
		pr_err("%s: Cannot assert (%d)\n", __func__, ret);
		return;
	}

	msleep(1);

	id = PCOM_CLKRGM_APPS_RESET_USB_PHY;
	ret = msm_proc_comm(PCOM_CLK_REGIME_SEC_RESET_DEASSERT, &id, NULL);
	if (ret) {
		pr_err("%s: Cannot assert (%d)\n", __func__, ret);
		return;
	}
}

static void mahimahi_usb_hw_reset(bool enable)
{
	u32 id;
	int ret;
	u32 func;

	id = PCOM_CLKRGM_APPS_RESET_USBH;
	if (enable)
		func = PCOM_CLK_REGIME_SEC_RESET_ASSERT;
	else
		func = PCOM_CLK_REGIME_SEC_RESET_DEASSERT;
	ret = msm_proc_comm(func, &id, NULL);
	if (ret)
		pr_err("%s: Cannot set reset to %d (%d)\n", __func__, enable,
		       ret);
}


static struct msm_hsusb_platform_data msm_hsusb_pdata = {
	.phy_init_seq		= mahimahi_phy_init_seq,
	.phy_reset		= mahimahi_usb_phy_reset,
	.hw_reset		= mahimahi_usb_hw_reset,
	.usb_connected		= notify_usb_connected,
};

static char *usb_functions_ums[] = {
	"usb_mass_storage",
};

static char *usb_functions_ums_adb[] = {
	"usb_mass_storage",
	"adb",
};

static char *usb_functions_rndis[] = {
	"rndis",
};

static char *usb_functions_rndis_adb[] = {
	"rndis",
	"adb",
};

#ifdef CONFIG_USB_ANDROID_ACCESSORY
static char *usb_functions_accessory[] = { "accessory" };
static char *usb_functions_accessory_adb[] = { "accessory", "adb" };
#endif

#ifdef CONFIG_USB_ANDROID_DIAG
static char *usb_functions_adb_diag[] = {
	"usb_mass_storage",
	"adb",
	"diag",
};
#endif

static char *usb_functions_all[] = {
#ifdef CONFIG_USB_ANDROID_RNDIS
	"rndis",
#endif
#ifdef CONFIG_USB_ANDROID_ACCESSORY
	"accessory",
#endif
	"usb_mass_storage",
	"adb",
#ifdef CONFIG_USB_ANDROID_ACM
	"acm",
#endif
#ifdef CONFIG_USB_ANDROID_DIAG
	"diag",
#endif
};

static struct android_usb_product usb_products[] = {
	{
		.product_id	= 0x4e11,
		.num_functions	= ARRAY_SIZE(usb_functions_ums),
		.functions	= usb_functions_ums,
	},
	{
		.product_id	= 0x4e12,
		.num_functions	= ARRAY_SIZE(usb_functions_ums_adb),
		.functions	= usb_functions_ums_adb,
	},
	{
		.product_id	= 0x4e13,
		.num_functions	= ARRAY_SIZE(usb_functions_rndis),
		.functions	= usb_functions_rndis,
	},
	{
		.product_id	= 0x4e14,
		.num_functions	= ARRAY_SIZE(usb_functions_rndis_adb),
		.functions	= usb_functions_rndis_adb,
	},
#ifdef CONFIG_USB_ANDROID_ACCESSORY
	{
		.vendor_id	= USB_ACCESSORY_VENDOR_ID,
		.product_id	= USB_ACCESSORY_PRODUCT_ID,
		.num_functions	= ARRAY_SIZE(usb_functions_accessory),
		.functions	= usb_functions_accessory,
	},
	{
		.vendor_id	= USB_ACCESSORY_VENDOR_ID,
		.product_id	= USB_ACCESSORY_ADB_PRODUCT_ID,
		.num_functions	= ARRAY_SIZE(usb_functions_accessory_adb),
		.functions	= usb_functions_accessory_adb,
	},
#endif
#ifdef CONFIG_USB_ANDROID_DIAG
	{
		.product_id	= 0x4e17,
		.num_functions	= ARRAY_SIZE(usb_functions_adb_diag),
		.functions	= usb_functions_adb_diag,
	},
#endif
};

static struct usb_mass_storage_platform_data mass_storage_pdata = {
	.nluns		= 1,
	.vendor		= "Google, Inc.",
	.product	= "Nexus One",
	.release	= 0x0100,
};

static struct platform_device usb_mass_storage_device = {
	.name	= "usb_mass_storage",
	.id	= -1,
	.dev	= {
		.platform_data = &mass_storage_pdata,
	},
};

#ifdef CONFIG_USB_ANDROID_RNDIS
static struct usb_ether_platform_data rndis_pdata = {
	/* ethaddr is filled by board_serialno_setup */
	.vendorID	= 0x18d1,
	.vendorDescr	= "Google, Inc.",
};

static struct platform_device rndis_device = {
	.name	= "rndis",
	.id	= -1,
	.dev	= {
		.platform_data = &rndis_pdata,
	},
};
#endif

static struct android_usb_platform_data android_usb_pdata = {
	.vendor_id	= 0x18d1,
	.product_id	= 0x4e11,
	.version	= 0x0100,
	.product_name		= "Nexus One",
	.manufacturer_name	= "Google, Inc.",
	.num_products = ARRAY_SIZE(usb_products),
	.products = usb_products,
	.num_functions = ARRAY_SIZE(usb_functions_all),
	.functions = usb_functions_all,
};

static struct platform_device android_usb_device = {
	.name	= "android_usb",
	.id		= -1,
	.dev		= {
		.platform_data = &android_usb_pdata,
	},
};
#endif

/* start usb */
#define MAHIMAHI_GPIO_USBPHY_3V3_ENABLE		104

static unsigned ulpi_on_gpio_table[] = {
	GPIO_CFG(0x68, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_4MA),
	GPIO_CFG(0x6f, 1, GPIO_INPUT, GPIO_NO_PULL, GPIO_2MA),
	GPIO_CFG(0x70, 1, GPIO_INPUT, GPIO_NO_PULL, GPIO_2MA),
	GPIO_CFG(0x71, 1, GPIO_INPUT, GPIO_NO_PULL, GPIO_2MA),
	GPIO_CFG(0x72, 1, GPIO_INPUT, GPIO_NO_PULL, GPIO_2MA),
	GPIO_CFG(0x73, 1, GPIO_INPUT, GPIO_NO_PULL, GPIO_2MA),
	GPIO_CFG(0x74, 1, GPIO_INPUT, GPIO_NO_PULL, GPIO_2MA),
	GPIO_CFG(0x75, 1, GPIO_INPUT, GPIO_NO_PULL, GPIO_2MA),
	GPIO_CFG(0x76, 1, GPIO_INPUT, GPIO_NO_PULL, GPIO_2MA),
	GPIO_CFG(0x77, 1, GPIO_INPUT, GPIO_NO_PULL, GPIO_2MA),
	GPIO_CFG(0x78, 1, GPIO_INPUT, GPIO_NO_PULL, GPIO_2MA),
	GPIO_CFG(0x79, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA),
};

static unsigned ulpi_off_gpio_table[] = {
	GPIO_CFG(0x68, 1, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_4MA),
	GPIO_CFG(0x6f, 1, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA),
	GPIO_CFG(0x70, 1, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA),
	GPIO_CFG(0x71, 1, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA),
	GPIO_CFG(0x72, 1, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA),
	GPIO_CFG(0x73, 1, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA),
	GPIO_CFG(0x74, 1, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA),
	GPIO_CFG(0x75, 1, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA),
	GPIO_CFG(0x76, 1, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA),
	GPIO_CFG(0x77, 1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA),
	GPIO_CFG(0x78, 1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA),
	GPIO_CFG(0x79, 1, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_2MA),
};

static void usb_gpio_init(void)
{
	if (gpio_request(0x68, "mahimahi_3v3_enable"))
		pr_err("failed to request gpio mahimahi_3v3_enable\n");
	if (gpio_request(0x6f, "ulpi_data_0"))
		pr_err("failed to request gpio ulpi_data_0\n");
	if (gpio_request(0x70, "ulpi_data_1"))
		pr_err("failed to request gpio ulpi_data_1\n");
	if (gpio_request(0x71, "ulpi_data_2"))
		pr_err("failed to request gpio ulpi_data_2\n");
	if (gpio_request(0x72, "ulpi_data_3"))
		pr_err("failed to request gpio ulpi_data_3\n");
	if (gpio_request(0x73, "ulpi_data_4"))
		pr_err("failed to request gpio ulpi_data_4\n");
	if (gpio_request(0x74, "ulpi_data_5"))
		pr_err("failed to request gpio ulpi_data_5\n");
	if (gpio_request(0x75, "ulpi_data_6"))
		pr_err("failed to request gpio ulpi_data_6\n");
	if (gpio_request(0x76, "ulpi_data_7"))
		pr_err("failed to request gpio ulpi_data_7\n");
	if (gpio_request(0x77, "ulpi_dir"))
		pr_err("failed to request gpio ulpi_dir\n");
	if (gpio_request(0x78, "ulpi_next"))
		pr_err("failed to request gpio ulpi_next\n");
	if (gpio_request(0x79, "ulpi_stop"))
		pr_err("failed to request gpio ulpi_stop\n");
}

static int usb_config_gpio(int config)
{
	int pin, rc;

	if (config) {
		for (pin = 0; pin < ARRAY_SIZE(ulpi_on_gpio_table); pin++) {			
			rc = gpio_tlmm_config(ulpi_on_gpio_table[pin],
					      GPIO_CFG_ENABLE);
			if (rc) {
				printk(KERN_ERR
				       "%s: gpio_tlmm_config(%#x)=%d\n",
				       __func__, ulpi_off_gpio_table[pin], rc);
				return -EIO;
			}
		}
	} else {
		for (pin = 0; pin < ARRAY_SIZE(ulpi_off_gpio_table); pin++) {
			rc = gpio_tlmm_config(ulpi_off_gpio_table[pin],
					      GPIO_CFG_ENABLE);
			if (rc) {
				printk(KERN_ERR
				       "%s: gpio_tlmm_config(%#x)=%d\n",
				       __func__, ulpi_on_gpio_table[pin], rc);
				return -EIO;
			}
		}
	}

	return 0;
}

static void usb_phy_shutdown(void)
{
	printk("%s: %s\n", __FILE__, __func__);
	gpio_set_value(MAHIMAHI_GPIO_USBPHY_3V3_ENABLE, 1); 
	mdelay(3);
	gpio_set_value(MAHIMAHI_GPIO_USBPHY_3V3_ENABLE, 0);
	mdelay(3);
}

int usb_phy_reset(void  __iomem *regs)
{
	printk("%s: %s\n", __FILE__, __func__);
	usb_phy_shutdown();
	gpio_set_value(MAHIMAHI_GPIO_USBPHY_3V3_ENABLE, 0); 
	mdelay(3);
	gpio_set_value(MAHIMAHI_GPIO_USBPHY_3V3_ENABLE, 1);
	mdelay(3);
	usb_config_gpio(1);

	return 0;
}

#define USB_LINK_RESET_TIMEOUT      (msecs_to_jiffies(10))
#define CLKRGM_APPS_RESET_USBH      37
#define CLKRGM_APPS_RESET_USB_PHY   34

#define ULPI_VERIFY_MAX_LOOP_COUNT  3
static void *usb_base;
#ifndef MSM_USB_BASE
#define MSM_USB_BASE              ((unsigned)usb_base)
#endif
static unsigned mahimahi_ulpi_read(void __iomem *usb_base, unsigned reg)
{
	unsigned timeout = 100000;

	/* initiate read operation */
	writel(ULPI_RUN | ULPI_READ | ULPI_ADDR(reg),
	       USB_ULPI_VIEWPORT);

	/* wait for completion */
	while ((readl(USB_ULPI_VIEWPORT) & ULPI_RUN) && (--timeout))
		cpu_relax();

	if (timeout == 0) {
		printk(KERN_ERR "ulpi_read: timeout %08x\n",
			readl(USB_ULPI_VIEWPORT));
		return 0xffffffff;
	}
	return ULPI_DATA_READ(readl(USB_ULPI_VIEWPORT));
}

static int mahimahi_ulpi_write(void __iomem *usb_base, unsigned val, unsigned reg)
{
	unsigned timeout = 10000;

	/* initiate write operation */
	writel(ULPI_RUN | ULPI_WRITE |
	       ULPI_ADDR(reg) | ULPI_DATA(val),
	       USB_ULPI_VIEWPORT);

	/* wait for completion */
	while ((readl(USB_ULPI_VIEWPORT) & ULPI_RUN) && (--timeout))
		cpu_relax();

	if (timeout == 0) {
		printk(KERN_ERR "ulpi_write: timeout\n");
		return -1;
	}

	return 0;
}

void msm_hsusb_apps_reset_link(int reset)
{
	int ret;
	unsigned usb_id = CLKRGM_APPS_RESET_USBH;

	if (reset)
		ret = msm_proc_comm(PCOM_CLK_REGIME_SEC_RESET_ASSERT,
				&usb_id, NULL);
	else
		ret = msm_proc_comm(PCOM_CLK_REGIME_SEC_RESET_DEASSERT,
				&usb_id, NULL);
	if (ret)
		printk(KERN_INFO "%s: Cannot set reset to %d (%d)\n",
			__func__, reset, ret);
}
EXPORT_SYMBOL(msm_hsusb_apps_reset_link);

void msm_hsusb_apps_reset_phy(void)
{
	int ret;
	unsigned usb_phy_id = CLKRGM_APPS_RESET_USB_PHY;

	ret = msm_proc_comm(PCOM_CLK_REGIME_SEC_RESET_ASSERT,
			&usb_phy_id, NULL);
	if (ret) {
		printk(KERN_INFO "%s: Cannot assert (%d)\n", __func__, ret);
		return;
	}
	msleep(1);
	ret = msm_proc_comm(PCOM_CLK_REGIME_SEC_RESET_DEASSERT,
			&usb_phy_id, NULL);
	if (ret) {
		printk(KERN_INFO "%s: Cannot assert (%d)\n", __func__, ret);
		return;
	}
}
EXPORT_SYMBOL(msm_hsusb_apps_reset_phy);

static int msm_hsusb_phy_verify_access(void __iomem *usb_base)
{
	int temp;

	for (temp = 0; temp < ULPI_VERIFY_MAX_LOOP_COUNT; temp++) {
		if (mahimahi_ulpi_read(usb_base, ULPI_DEBUG) != (unsigned)-1)
			break;
		msm_hsusb_apps_reset_phy();
	}

	if (temp == ULPI_VERIFY_MAX_LOOP_COUNT) {
		pr_err("%s: ulpi read failed for %d times\n",
				__func__, ULPI_VERIFY_MAX_LOOP_COUNT);
		return -1;
	}

	return 0;
}

static unsigned msm_hsusb_ulpi_read_with_reset(void __iomem *usb_base, unsigned reg)
{
	int temp;
	unsigned res;

	for (temp = 0; temp < ULPI_VERIFY_MAX_LOOP_COUNT; temp++) {
		res = mahimahi_ulpi_read(usb_base, reg);
		if (res != -1)
			return res;
		msm_hsusb_apps_reset_phy();
	}

	pr_err("%s: ulpi read failed for %d times\n",
			__func__, ULPI_VERIFY_MAX_LOOP_COUNT);

	return -1;
}

static int msm_hsusb_ulpi_write_with_reset(void __iomem *usb_base,
		unsigned val, unsigned reg)
{
	int temp;
	int res;

	for (temp = 0; temp < ULPI_VERIFY_MAX_LOOP_COUNT; temp++) {
		res = mahimahi_ulpi_write(usb_base, val, reg);
		if (!res)
			return 0;
		msm_hsusb_apps_reset_phy();
	}

	pr_err("%s: ulpi write failed for %d times\n",
			__func__, ULPI_VERIFY_MAX_LOOP_COUNT);
	return -1;
}

static int msm_hsusb_phy_caliberate(void __iomem *usb_base)
{
	int ret;
	unsigned res;

	ret = msm_hsusb_phy_verify_access(usb_base);
	if (ret)
		return -ETIMEDOUT;

	res = msm_hsusb_ulpi_read_with_reset(usb_base, ULPI_FUNC_CTRL_CLR);
	if (res == -1)
		return -ETIMEDOUT;

	res = msm_hsusb_ulpi_write_with_reset(usb_base,
			res | ULPI_SUSPENDM,
			ULPI_FUNC_CTRL_CLR);
	if (res)
		return -ETIMEDOUT;

	msm_hsusb_apps_reset_phy();

	return msm_hsusb_phy_verify_access(usb_base);
}

void msm_hsusb_8x50_phy_reset(void)
{
	u32 temp;
	unsigned long timeout;
	int ret, usb_phy_error;
	printk(KERN_INFO "msm_hsusb_phy_reset\n");
	usb_base = ioremap(MSM_HSUSB_PHYS, 4096);

	msm_hsusb_apps_reset_link(1);
	msm_hsusb_apps_reset_phy();
	msm_hsusb_apps_reset_link(0);

	/* select ULPI phy */
	temp = (readl(USB_PORTSC) & ~PORTSC_PTS);
	writel(temp | PORTSC_PTS_ULPI, USB_PORTSC);

	if ((ret = msm_hsusb_phy_caliberate(usb_base))) {
		usb_phy_error = 1;
		pr_err("msm_hsusb_phy_caliberate returned with %i\n", ret);
		return;
	}

	/* soft reset phy */
	writel(USBCMD_RESET, USB_USBCMD);
	timeout = jiffies + USB_LINK_RESET_TIMEOUT;
	while (readl(USB_USBCMD) & USBCMD_RESET) {
		if (time_after(jiffies, timeout)) {
			pr_err("usb link reset timeout\n");
			break;
		}
		msleep(1);
	}
	usb_phy_error = 0;

	return;
}

static int mahimahi_phy_init_seq[] = {0x0C, 0x31, 0x30, 0x32, 0x1D, 0x0D, 0x1D, 0x10, -1};

static struct msm_otg_platform_data msm_otg_pdata = {
	.phy_init_seq		= mahimahi_phy_init_seq,
	.mode			= USB_PERIPHERAL,
	.otg_control		= OTG_PHY_CONTROL,
};

static uint32_t usb_phy_3v3_table[] =
{
	PCOM_GPIO_CFG(MAHIMAHI_GPIO_USBPHY_3V3_ENABLE, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_4MA)
};

static struct usb_mass_storage_platform_data mass_storage_pdata = {
	.nluns		= 1,
	.vendor		= "Google, Inc.",
	.product	= "Nexus One",
	.release	= 0x0100,
};

static struct platform_device usb_mass_storage_device = {
	.name	= "usb_mass_storage",
	.id	= -1,
	.dev	= {
		.platform_data = &mass_storage_pdata,
	},
};

#ifdef CONFIG_USB_ANDROID_RNDIS
static struct usb_ether_platform_data rndis_pdata = {
	.vendorID	= 0x0bb4,
	.vendorDescr	= "HTC",
};

static struct platform_device rndis_device = {
	.name	= "rndis",
	.id	= -1,
	.dev	= {
		.platform_data = &rndis_pdata,
	},
};
#endif

static struct android_usb_platform_data android_usb_pdata = {
	.functions = usb_functions_all,
	.vendor_id	= 0x18d1,
	.product_id	= 0x4e11,
	.version	= 0x0100,
	.product_name		= "Nexus One",
	.manufacturer_name	= "Google, Inc.",
	.num_products = ARRAY_SIZE(usb_products),
	.products = usb_products,
	.num_functions = ARRAY_SIZE(usb_functions_all),
	.functions = usb_functions_all,
};

static struct platform_device android_usb_device = {
	.name	= "android_usb",
	.id		= -1,
	.dev		= {
		.platform_data = &android_usb_pdata,
	},
};

void mahimahi_add_usb_devices(void)
{
	printk(KERN_INFO "%s rev: %d\n", __func__, system_rev);
	android_usb_pdata.products[0].product_id =
			android_usb_pdata.product_id;


	/* add cdrom support in normal mode */
	if (board_mfg_mode() == 0) {
		android_usb_pdata.nluns = 3;
		android_usb_pdata.cdrom_lun = 0x4;
	}

	msm_device_otg.dev.platform_data = &msm_otg_pdata;
	//msm_device_gadget_peripheral.dev.platform_data = &msm_gadget_pdata;
	msm_device_gadget_peripheral.dev.parent = &msm_device_otg.dev;
	usb_gpio_init();
	platform_device_register(&msm_device_gadget_peripheral);
	platform_device_register(&android_usb_device);
}

unsigned mahimahi_get_vbus_state(void)
{
	if(readl(MSM_SHARED_RAM_BASE+0xef20c))
		return 1;
	else
		return 0;
}
/* end usb */

static struct platform_device mahimahi_rfkill = {
	.name = "mahimahi_rfkill",
	.id = -1,
};

/* start kgsl */
static struct resource kgsl_3d0_resources[] = {
	{
		.name  = KGSL_3D0_REG_MEMORY,
		.start = 0xA0000000,
		.end = 0xA001ffff,
		.flags = IORESOURCE_MEM,
	},
	{
		.name = KGSL_3D0_IRQ,
		.start = INT_GRAPHICS,
		.end = INT_GRAPHICS,
		.flags = IORESOURCE_IRQ,
	},
};

static struct kgsl_device_platform_data kgsl_3d0_pdata = {
	.pwr_data = {
		.pwrlevel = {
			{
				.gpu_freq = 0,
				.bus_freq = 128000000,
			},
		},
		.init_level = 0,
		.num_levels = 1,
		.set_grp_async = NULL,
		.idle_timeout = HZ/5,
	},
	.clk = {
		.name = {
			.clk = "core_clk",
		},
	},
	.imem_clk_name = {
		.clk = "iface_clk",
	},
};

struct platform_device msm_kgsl_3d0 = {
	.name = "kgsl-3d0",
	.id = 0,
	.num_resources = ARRAY_SIZE(kgsl_3d0_resources),
	.resource = kgsl_3d0_resources,
	.dev = {
		.platform_data = &kgsl_3d0_pdata,
	},
};
/* end kgsl */

/* start footswitch regulator */
struct platform_device *msm_footswitch_devices[] = {
	FS_PCOM(FS_GFX3D,  "fs_gfx3d"),
};

unsigned msm_num_footswitch_devices = ARRAY_SIZE(msm_footswitch_devices);
/* end footswitch regulator */

/* start pmem setup */
#define MSM_PMEM_SF_SIZE	0x1700000
#define MSM_AUDIO_SIZE		0x80000
#define PMEM_KERNEL_EBI1_SIZE 0x00028000

static struct android_pmem_platform_data android_pmem_kernel_ebi1_pdata = {
	.name = PMEM_KERNEL_EBI1_DATA_NAME,
	/* if no allocator_type, defaults to PMEM_ALLOCATORTYPE_BITMAP,
	 * the only valid choice at this time. The board structure is
	 * set to all zeros by the C runtime initialization and that is now
	 * the enum value of PMEM_ALLOCATORTYPE_BITMAP, now forced to 0 in
	 * include/linux/android_pmem.h.
	 */
	.cached = 0,
};

#ifdef CONFIG_KERNEL_PMEM_SMI_REGION

static struct android_pmem_platform_data android_pmem_kernel_smi_pdata = {
	.name = PMEM_KERNEL_SMI_DATA_NAME,
	/* if no allocator_type, defaults to PMEM_ALLOCATORTYPE_BITMAP,
	 * the only valid choice at this time. The board structure is
	 * set to all zeros by the C runtime initialization and that is now
	 * the enum value of PMEM_ALLOCATORTYPE_BITMAP, now forced to 0 in
	 * include/linux/android_pmem.h.
	 */
	.cached = 0,
};

#endif

static struct android_pmem_platform_data android_pmem_pdata = {
	.name = "pmem",
	.allocator_type = PMEM_ALLOCATORTYPE_ALLORNOTHING,
	.cached = 1,
	.memory_type = MEMTYPE_EBI1,
};

static struct android_pmem_platform_data android_pmem_adsp_pdata = {
	.name = "pmem_adsp",
	.allocator_type = PMEM_ALLOCATORTYPE_BITMAP,
	.cached = 1,
	.memory_type = MEMTYPE_EBI1,
};

static struct android_pmem_platform_data android_pmem_venc_pdata = {
	.name = "pmem_venc",
	.allocator_type = PMEM_ALLOCATORTYPE_BITMAP,
	.cached = 1,
	.memory_type = MEMTYPE_EBI1,
};

static struct android_pmem_platform_data android_pmem_audio_pdata = {
	.name = "pmem_audio",
	.allocator_type = PMEM_ALLOCATORTYPE_BITMAP,
	.cached = 0,
	.memory_type = MEMTYPE_EBI1,
};

static struct platform_device android_pmem_device = {
	.name = "android_pmem",
	.id = 0,
	.dev = { .platform_data = &android_pmem_pdata },
};

static struct platform_device android_pmem_adsp_device = {
	.name = "android_pmem",
	.id = 1,
	.dev = { .platform_data = &android_pmem_adsp_pdata },
};

static struct platform_device android_pmem_audio_device = {
	.name = "android_pmem",
	.id = 2,
	.dev = { .platform_data = &android_pmem_audio_pdata },
};

#ifdef CONFIG_KERNEL_PMEM_SMI_REGION
static struct platform_device android_pmem_kernel_smi_device = {
	.name = "android_pmem",
	.id = 4,
	.dev = { .platform_data = &android_pmem_kernel_smi_pdata },
};
#endif

static struct platform_device android_pmem_venc_device = {
	.name = "android_pmem",
	.id = 5,
	.dev = { .platform_data = &android_pmem_venc_pdata },
};

static unsigned pmem_kernel_ebi1_size = PMEM_KERNEL_EBI1_SIZE;
static int __init pmem_kernel_ebi1_size_setup(char *p)
{
	pmem_kernel_ebi1_size = memparse(p, NULL);
	return 0;
}
early_param("pmem_kernel_ebi1_size", pmem_kernel_ebi1_size_setup);

static unsigned pmem_sf_size = MSM_PMEM_SF_SIZE;
static int __init pmem_sf_size_setup(char *p)
{
	pmem_sf_size = memparse(p, NULL);
	return 0;
}
early_param("pmem_sf_size", pmem_sf_size_setup);

static unsigned pmem_mdp_size = MSM_PMEM_MDP_SIZE;
static int __init pmem_mdp_size_setup(char *p)
{
	pmem_mdp_size = memparse(p, NULL);
	return 0;
}
early_param("pmem_mdp_size", pmem_mdp_size_setup);

static unsigned pmem_venc_size = MSM_PMEM_VENC_SIZE;
static int __init pmem_venc_size_setup(char *p)
{
	pmem_venc_size = memparse(p, NULL);
	return 0;
}
early_param("pmem_venc_size", pmem_venc_size_setup);

static unsigned pmem_adsp_size = MSM_PMEM_ADSP_SIZE;
static int __init pmem_adsp_size_setup(char *p)
{
	pmem_adsp_size = memparse(p, NULL);
	return 0;
}
early_param("pmem_adsp_size", pmem_adsp_size_setup);

static unsigned pmem_audio_size = MSM_AUDIO_SIZE;
static int __init pmem_audio_size_setup(char *p)
{
	pmem_audio_size = memparse(p, NULL);
	return 0;
}
early_param("audio_size", pmem_audio_size_setup);

static struct memtype_reserve mahimahi_reserve_table[] __initdata = {
	[MEMTYPE_SMI] = {
	},
	[MEMTYPE_EBI0] = {
		.flags	=	MEMTYPE_FLAGS_1M_ALIGN,
	},
	[MEMTYPE_EBI1] = {
		.flags	=	MEMTYPE_FLAGS_1M_ALIGN,
	},
};

static void __init size_pmem_device(struct android_pmem_platform_data *pdata, unsigned long start, unsigned long size)
{
  pdata->size = size;
  pr_info("%s: pmem %s requests %lu bytes dynamically.\n",
      __func__, pdata->name, size);
}

static void __init size_pmem_devices(void)
{
#ifdef CONFIG_ANDROID_PMEM
  size_pmem_device(&android_pmem_adsp_pdata, 0, pmem_adsp_size);
  size_pmem_device(&android_pmem_pdata, 0, pmem_mdp_size);
  size_pmem_device(&android_pmem_venc_pdata, 0, pmem_venc_size);
  size_pmem_device(&android_pmem_audio_pdata, 0, pmem_audio_size);
  mahimahi_reserve_table[MEMTYPE_EBI1].size += PMEM_KERNEL_EBI1_SIZE;
#endif
}

static void __init reserve_memory_for(struct android_pmem_platform_data *p)
{
  pr_info("%s: reserve %lu bytes from memory %d for %s.\n", __func__, p->size, p->memory_type, p->name);
  mahimahi_reserve_table[p->memory_type].size += p->size;
}

static void __init reserve_pmem_memory(void)
{
#ifdef CONFIG_ANDROID_PMEM
	reserve_memory_for(&android_pmem_adsp_pdata);
	reserve_memory_for(&android_pmem_pdata);
	reserve_memory_for(&android_pmem_venc_pdata);
	reserve_memory_for(&android_pmem_audio_pdata);
#endif
}

static void __init mahimahi_calculate_reserve_sizes(void)
{
	size_pmem_devices();
	reserve_pmem_memory();
}

static int mahimahi_paddr_to_memtype(unsigned int paddr)
{
	return MEMTYPE_EBI1;
}

static struct reserve_info mahimahi_reserve_info __initdata = {
	.memtype_reserve_table = mahimahi_reserve_table,
	.calculate_reserve_sizes = mahimahi_calculate_reserve_sizes,
	.paddr_to_memtype = mahimahi_paddr_to_memtype,
};

static void __init mahimahi_reserve(void)
{
	reserve_info = &mahimahi_reserve_info;
	msm_reserve();
}
/* End pmem setup */

static struct resource ram_console_resources[] = {
	{
		.start	= MSM_RAM_CONSOLE_BASE,
		.end	= MSM_RAM_CONSOLE_BASE + MSM_RAM_CONSOLE_SIZE - 1,
		.flags	= IORESOURCE_MEM,
	},
};

static struct platform_device ram_console_device = {
	.name		= "ram_console",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(ram_console_resources),
	.resource	= ram_console_resources,
};

static int mahimahi_ts_power(int on)
{
	pr_info("%s: power %d\n", __func__, on);

	if (on) {
		/* level shifter should be off */
		gpio_set_value(MAHIMAHI_GPIO_TP_EN, 1);
		msleep(120);
		/* enable touch panel level shift */
		gpio_set_value(MAHIMAHI_GPIO_TP_LS_EN, 1);
		msleep(3);
	} else {
		gpio_set_value(MAHIMAHI_GPIO_TP_LS_EN, 0);
		gpio_set_value(MAHIMAHI_GPIO_TP_EN, 0);
		udelay(50);
	}

	return 0;
}

static struct synaptics_i2c_rmi_platform_data mahimahi_synaptics_ts_data[] = {
	{
		.version = 0x105,
		.power = mahimahi_ts_power,
		.flags = SYNAPTICS_FLIP_Y,
		.inactive_left = -15 * 0x10000 / 480,
		.inactive_right = -15 * 0x10000 / 480,
		.inactive_top = -15 * 0x10000 / 800,
		.inactive_bottom = -50 * 0x10000 / 800,
		.sensitivity_adjust = 9,
	},
	{
		.flags = SYNAPTICS_FLIP_Y,
		.inactive_left = -15 * 0x10000 / 480,
		.inactive_right = -15 * 0x10000 / 480,
		.inactive_top = -15 * 0x10000 / 800,
		.inactive_bottom = -40 * 0x10000 / 800,
		.sensitivity_adjust = 12,
	},
};

static struct a1026_platform_data a1026_data = {
	.gpio_a1026_micsel = MAHIMAHI_AUD_MICPATH_SEL,
	.gpio_a1026_wakeup = MAHIMAHI_AUD_A1026_WAKEUP,
	.gpio_a1026_reset = MAHIMAHI_AUD_A1026_RESET,
	.gpio_a1026_clk = MAHIMAHI_AUD_A1026_CLK,
	/*.gpio_a1026_int = MAHIMAHI_AUD_A1026_INT,*/
};

static struct akm8973_platform_data compass_platform_data = {
	.layouts = MAHIMAHI_LAYOUTS,
	.project_name = MAHIMAHI_PROJECT_NAME,
	.reset = MAHIMAHI_GPIO_COMPASS_RST_N,
	.intr = MAHIMAHI_GPIO_COMPASS_INT_N,
};

static struct regulator_consumer_supply tps65023_dcdc1_supplies[] = {
	{
		.supply = "acpu_vcore",
	},
};

static struct regulator_init_data tps65023_data[5] = {
	{
		.constraints = {
			.name = "dcdc1", /* VREG_MSMC2_1V29 */
			.min_uV = 975000,
			.max_uV = 1275000,
			.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE,
		},
		.consumer_supplies = tps65023_dcdc1_supplies,
		.num_consumer_supplies = ARRAY_SIZE(tps65023_dcdc1_supplies),
	},
	/* dummy values for unused regulators to not crash driver: */
	{
		.constraints = {
			.name = "dcdc2", /* VREG_MSMC1_1V26 */
			.min_uV = 1260000,
			.max_uV = 1260000,
		},
	},
	{
		.constraints = {
			.name = "dcdc3", /* unused */
			.min_uV = 800000,
			.max_uV = 3300000,
		},
	},
	{
		.constraints = {
			.name = "ldo1", /* unused */
			.min_uV = 1000000,
			.max_uV = 3150000,
		},
	},
	{
		.constraints = {
			.name = "ldo2", /* V_USBPHY_3V3 */
			.min_uV = 3300000,
			.max_uV = 3300000,
		},
	},
};


static void ds2482_set_slp_n(unsigned n)
{
	gpio_direction_output(MAHIMAHI_GPIO_DS2482_SLP_N, n);
}

static struct tpa2018d1_platform_data tpa2018_data = {
	.gpio_tpa2018_spk_en = MAHIMAHI_CDMA_GPIO_AUD_SPK_AMP_EN,
};

static struct i2c_board_info base_i2c_devices[] = {
	{
		I2C_BOARD_INFO("ds2482", 0x30 >> 1),
		.platform_data = ds2482_set_slp_n,
	},
	{
		I2C_BOARD_INFO(SYNAPTICS_I2C_RMI_NAME, 0x40),
		.platform_data = mahimahi_synaptics_ts_data,
		.irq = MSM_GPIO_TO_INT(MAHIMAHI_GPIO_TP_INT_N)
	},
	{
		I2C_BOARD_INFO("mahimahi-microp", 0x66),
		.irq = MSM_GPIO_TO_INT(MAHIMAHI_GPIO_UP_INT_N)
	},
	{
		I2C_BOARD_INFO("s5k3e2fx", 0x20 >> 1),
	},
	{
		I2C_BOARD_INFO("tps65023", 0x48),
		.platform_data = tps65023_data,
	},
};

static struct i2c_board_info rev0_i2c_devices[] = {
	{
		I2C_BOARD_INFO(AKM8973_I2C_NAME, 0x1C),
		.platform_data = &compass_platform_data,
		.irq = MSM_GPIO_TO_INT(MAHIMAHI_REV0_GPIO_COMPASS_INT_N),
	},
};

static struct i2c_board_info rev1_i2c_devices[] = {
	{
		I2C_BOARD_INFO("audience_a1026", 0x3E),
		.platform_data = &a1026_data,
		/*.irq = MSM_GPIO_TO_INT(MAHIMAHI_AUD_A1026_INT)*/
	},
	{
		I2C_BOARD_INFO(AKM8973_I2C_NAME, 0x1C),
		.platform_data = &compass_platform_data,
		.irq = MSM_GPIO_TO_INT(MAHIMAHI_GPIO_COMPASS_INT_N),
	},
};

static struct i2c_board_info rev_CX_i2c_devices[] = {
	{
		I2C_BOARD_INFO("tpa2018d1", 0x58),
		.platform_data = &tpa2018_data,
	},
	{
		I2C_BOARD_INFO("smb329", 0x6E >> 1),
	},
};

static uint32_t camera_off_gpio_table[] = {
	/* CAMERA */
	PCOM_GPIO_CFG(0, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_4MA), /* DAT0 */
	PCOM_GPIO_CFG(1, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_4MA), /* DAT1 */
	PCOM_GPIO_CFG(2, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_4MA), /* DAT2 */
	PCOM_GPIO_CFG(3, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_4MA), /* DAT3 */
	PCOM_GPIO_CFG(4, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_4MA), /* DAT4 */
	PCOM_GPIO_CFG(5, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_4MA), /* DAT5 */
	PCOM_GPIO_CFG(6, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_4MA), /* DAT6 */
	PCOM_GPIO_CFG(7, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_4MA), /* DAT7 */
	PCOM_GPIO_CFG(8, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_4MA), /* DAT8 */
	PCOM_GPIO_CFG(9, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_4MA), /* DAT9 */
	PCOM_GPIO_CFG(10, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_4MA), /* DAT10 */
	PCOM_GPIO_CFG(11, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_4MA), /* DAT11 */
	PCOM_GPIO_CFG(12, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_4MA), /* PCLK */
	PCOM_GPIO_CFG(13, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_4MA), /* HSYNC */
	PCOM_GPIO_CFG(14, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_4MA), /* VSYNC */
	PCOM_GPIO_CFG(15, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_4MA), /* MCLK */
};

static uint32_t camera_on_gpio_table[] = {
	/* CAMERA */
	PCOM_GPIO_CFG(0, 1, GPIO_INPUT, GPIO_PULL_UP, GPIO_2MA), /* DAT0 */
	PCOM_GPIO_CFG(1, 1, GPIO_INPUT, GPIO_PULL_UP, GPIO_2MA), /* DAT1 */
	PCOM_GPIO_CFG(2, 1, GPIO_INPUT, GPIO_PULL_UP, GPIO_2MA), /* DAT2 */
	PCOM_GPIO_CFG(3, 1, GPIO_INPUT, GPIO_PULL_UP, GPIO_2MA), /* DAT3 */
	PCOM_GPIO_CFG(4, 1, GPIO_INPUT, GPIO_PULL_UP, GPIO_2MA), /* DAT4 */
	PCOM_GPIO_CFG(5, 1, GPIO_INPUT, GPIO_PULL_UP, GPIO_2MA), /* DAT5 */
	PCOM_GPIO_CFG(6, 1, GPIO_INPUT, GPIO_PULL_UP, GPIO_2MA), /* DAT6 */
	PCOM_GPIO_CFG(7, 1, GPIO_INPUT, GPIO_PULL_UP, GPIO_2MA), /* DAT7 */
	PCOM_GPIO_CFG(8, 1, GPIO_INPUT, GPIO_PULL_UP, GPIO_2MA), /* DAT8 */
	PCOM_GPIO_CFG(9, 1, GPIO_INPUT, GPIO_PULL_UP, GPIO_2MA), /* DAT9 */
	PCOM_GPIO_CFG(10, 1, GPIO_INPUT, GPIO_PULL_UP, GPIO_2MA), /* DAT10 */
	PCOM_GPIO_CFG(11, 1, GPIO_INPUT, GPIO_PULL_UP, GPIO_2MA), /* DAT11 */
	PCOM_GPIO_CFG(12, 1, GPIO_INPUT, GPIO_PULL_UP, GPIO_16MA), /* PCLK */
	PCOM_GPIO_CFG(13, 1, GPIO_INPUT, GPIO_PULL_UP, GPIO_2MA), /* HSYNC */
	PCOM_GPIO_CFG(14, 1, GPIO_INPUT, GPIO_PULL_UP, GPIO_2MA), /* VSYNC */
	PCOM_GPIO_CFG(15, 1, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_8MA), /* MCLK */
};

void config_camera_on_gpios(void)
{
	config_gpio_table(camera_on_gpio_table,
		ARRAY_SIZE(camera_on_gpio_table));
}

void config_camera_off_gpios(void)
{
	config_gpio_table(camera_off_gpio_table,
		ARRAY_SIZE(camera_off_gpio_table));
}

static struct resource msm_camera_resources[] = {
	{
		.start	= MSM_VFE_PHYS,
		.end	= MSM_VFE_PHYS + MSM_VFE_SIZE - 1,
		.flags	= IORESOURCE_MEM,
	},
	{
		.start	= INT_VFE,
		 INT_VFE,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct msm_camera_device_platform_data msm_camera_device_data = {
	.camera_gpio_on  = config_camera_on_gpios,
	.camera_gpio_off = config_camera_off_gpios,
	.ioext.mdcphy = MSM_MDC_PHYS,
	.ioext.mdcsz  = MSM_MDC_SIZE,
	.ioext.appphy = MSM_CLK_CTL_PHYS,
	.ioext.appsz  = MSM_CLK_CTL_SIZE,
};

static struct camera_flash_cfg msm_camera_sensor_flash_cfg = {
        .camera_flash           = flashlight_control,
        .num_flash_levels       = FLASHLIGHT_NUM,
        .low_temp_limit         = 5,
        .low_cap_limit          = 15,

};

static struct msm_camera_sensor_info msm_camera_sensor_s5k3e2fx_data = {
	.sensor_name = "s5k3e2fx",
	.sensor_reset = 144, /* CAM1_RST */
	.sensor_pwd = 143,  /* CAM1_PWDN, enabled in a9 */
	/*.vcm_pwd = 31, */  /* CAM1_VCM_EN, enabled in a9 */
	.pdata = &msm_camera_device_data,
	.resource = msm_camera_resources,
	.num_resources = ARRAY_SIZE(msm_camera_resources),
	.flash_cfg      = &msm_camera_sensor_flash_cfg,
};

static struct platform_device msm_camera_sensor_s5k3e2fx = {
	.name      = "msm_camera_s5k3e2fx",
	.dev      = {
		.platform_data = &msm_camera_sensor_s5k3e2fx_data,
	},
};

static int capella_cm3602_power(int on)
{
	/* TODO eolsen Add Voltage reg control */
	if (on) {
		gpio_direction_output(MAHIMAHI_GPIO_PROXIMITY_EN, 0);
	} else {
		gpio_direction_output(MAHIMAHI_GPIO_PROXIMITY_EN, 1);
	}

	return 0;
}


static struct capella_cm3602_platform_data capella_cm3602_pdata = {
	.power = capella_cm3602_power,
	.p_out = MAHIMAHI_GPIO_PROXIMITY_INT_N
};

static struct platform_device capella_cm3602 = {
	.name = CAPELLA_CM3602,
	.id = -1,
	.dev = {
		.platform_data = &capella_cm3602_pdata
	}
};

static uint32_t flashlight_gpio_table[] = {
	PCOM_GPIO_CFG(MAHIMAHI_GPIO_FLASHLIGHT_TORCH, 0, GPIO_OUTPUT,
						GPIO_NO_PULL, GPIO_2MA),
	PCOM_GPIO_CFG(MAHIMAHI_GPIO_FLASHLIGHT_FLASH, 0, GPIO_OUTPUT,
						GPIO_NO_PULL, GPIO_2MA),
};

static uint32_t flashlight_gpio_table_rev_CX[] = {
	PCOM_GPIO_CFG(MAHIMAHI_CDMA_GPIO_FLASHLIGHT_TORCH, 0, GPIO_OUTPUT,
						GPIO_NO_PULL, GPIO_2MA),
	PCOM_GPIO_CFG(MAHIMAHI_GPIO_FLASHLIGHT_FLASH, 0, GPIO_OUTPUT,
						GPIO_NO_PULL, GPIO_2MA),
};


static int config_mahimahi_flashlight_gpios(void)
{
	if (is_cdma_version(system_rev)) {
		config_gpio_table(flashlight_gpio_table_rev_CX,
			ARRAY_SIZE(flashlight_gpio_table_rev_CX));
	} else {
		config_gpio_table(flashlight_gpio_table,
			ARRAY_SIZE(flashlight_gpio_table));
	}
	return 0;
}

static struct flashlight_platform_data mahimahi_flashlight_data = {
	.gpio_init  = config_mahimahi_flashlight_gpios,
	.torch = MAHIMAHI_GPIO_FLASHLIGHT_TORCH,
	.flash = MAHIMAHI_GPIO_FLASHLIGHT_FLASH,
	.flash_duration_ms = 600
};

static struct platform_device mahimahi_flashlight_device = {
	.name = "flashlight",
	.dev = {
		.platform_data  = &mahimahi_flashlight_data,
	},
};

static struct timed_gpio timed_gpios[] = {
	{
		.name = "vibrator",
		.gpio = MAHIMAHI_GPIO_VIBRATOR_ON,
		.max_timeout = 15000,
	},
};

static struct timed_gpio_platform_data timed_gpio_data = {
	.num_gpios	= ARRAY_SIZE(timed_gpios),
	.gpios		= timed_gpios,
};

static struct platform_device mahimahi_timed_gpios = {
	.name		= "timed-gpio",
	.id		= -1,
	.dev		= {
		.platform_data = &timed_gpio_data,
	},
};

static struct msm_serial_hs_platform_data msm_uart_dm1_pdata = {
	.rx_wakeup_irq = -1,
	.inject_rx_on_wakeup = 0,
	.exit_lpm_cb = bcm_bt_lpm_exit_lpm_locked,
};

static struct bcm_bt_lpm_platform_data bcm_bt_lpm_pdata = {
	.gpio_wake = MAHIMAHI_GPIO_BT_WAKE,
	.gpio_host_wake = MAHIMAHI_GPIO_BT_HOST_WAKE,
	.request_clock_off_locked = msm_hs_request_clock_off,
	.request_clock_on_locked = msm_hs_request_clock_on_locked,
};

struct platform_device bcm_bt_lpm_device = {
	.name = "bcm_bt_lpm",
	.id = 0,
	.dev = {
		.platform_data = &bcm_bt_lpm_pdata,
	},
};

static int ds2784_charge(int on, int fast)
{
	if (is_cdma_version(system_rev)) {
		if (!on)
			smb329_set_charger_ctrl(SMB329_DISABLE_CHG);
		else
			smb329_set_charger_ctrl(fast ? SMB329_ENABLE_FAST_CHG : SMB329_ENABLE_SLOW_CHG);
	}
	else
		gpio_direction_output(MAHIMAHI_GPIO_BATTERY_CHARGER_CURRENT, !!fast);
	gpio_direction_output(MAHIMAHI_GPIO_BATTERY_CHARGER_EN, !on);
	return 0;
}

static int w1_ds2784_add_slave(struct w1_slave *sl)
{
	struct dd {
		struct platform_device pdev;
		struct ds2784_platform_data pdata;
	} *p;

	int rc;

	p = kzalloc(sizeof(struct dd), GFP_KERNEL);
	if (!p) {
		pr_err("%s: out of memory\n", __func__);
		return -ENOMEM;
	}

	rc = gpio_request(MAHIMAHI_GPIO_BATTERY_CHARGER_EN, "charger_en");
	if (rc < 0) {
		pr_err("%s: gpio_request(%d) failed: %d\n", __func__,
			MAHIMAHI_GPIO_BATTERY_CHARGER_EN, rc);
		kfree(p);
		return rc;
	}

	if (!is_cdma_version(system_rev)) {
		rc = gpio_request(MAHIMAHI_GPIO_BATTERY_CHARGER_CURRENT, "charger_current");
		if (rc < 0) {
			pr_err("%s: gpio_request(%d) failed: %d\n", __func__,
				MAHIMAHI_GPIO_BATTERY_CHARGER_CURRENT, rc);
			gpio_free(MAHIMAHI_GPIO_BATTERY_CHARGER_EN);
			kfree(p);
			return rc;
		}
	}

	p->pdev.name = "ds2784-battery";
	p->pdev.id = -1;
	p->pdev.dev.platform_data = &p->pdata;
	p->pdata.charge = ds2784_charge;
	p->pdata.w1_slave = sl;

	platform_device_register(&p->pdev);

	return 0;
}

static struct w1_family_ops w1_ds2784_fops = {
	.add_slave = w1_ds2784_add_slave,
};

static struct w1_family w1_ds2784_family = {
	.fid = W1_FAMILY_DS2784,
	.fops = &w1_ds2784_fops,
};

static int __init ds2784_battery_init(void)
{
	return w1_register_family(&w1_ds2784_family);
}

static struct resource qsd_spi_resources[] = {
	{
		.name   = "spi_irq_in",
		.start  = INT_SPI_INPUT,
		.end    = INT_SPI_INPUT,
		.flags  = IORESOURCE_IRQ,
	},
	{
		.name   = "spi_irq_out",
		.start  = INT_SPI_OUTPUT,
		.end    = INT_SPI_OUTPUT,
		.flags  = IORESOURCE_IRQ,
	},
	{
		.name   = "spi_irq_err",
		.start  = INT_SPI_ERROR,
		.end    = INT_SPI_ERROR,
		.flags  = IORESOURCE_IRQ,
	},
	{
		.name   = "spi_base",
		.start  = 0xA1200000,
		.end    = 0xA1200000 + SZ_4K - 1,
		.flags  = IORESOURCE_MEM,
	},
	{
		.name   = "spi_clk",
		.start  = 17,
		.end    = 1,
		.flags  = IORESOURCE_IRQ,
	},
	{
		.name   = "spi_mosi",
		.start  = 18,
		.end    = 1,
		.flags  = IORESOURCE_IRQ,
	},
	{
		.name   = "spi_miso",
		.start  = 19,
		.end    = 1,
		.flags  = IORESOURCE_IRQ,
	},
	{
		.name   = "spi_cs0",
		.start  = 20,
		.end    = 1,
		.flags  = IORESOURCE_IRQ,
	},
	{
		.name   = "spi_pwr",
		.start  = 21,
		.end    = 0,
		.flags  = IORESOURCE_IRQ,
	},
	{
		.name   = "spi_irq_cs0",
		.start  = 22,
		.end    = 0,
		.flags  = IORESOURCE_IRQ,
	},
};

static struct spi_platform_data mahimahi_spi_pdata = {
	.clk_rate	= 4800000,
};

struct platform_device qsd_device_spi = {
	.name           = "spi_qsd",
	.id             = 0,
	.num_resources  = ARRAY_SIZE(qsd_spi_resources),
	.resource       = qsd_spi_resources,
	.dev		= {
		.platform_data = &mahimahi_spi_pdata
	},
};

struct platform_device mahimahi_rtc = {
	.name = "msm_rtc",
	.id = -1,
};

static struct platform_device *devices[] __initdata = {
#if !defined(CONFIG_MSM_SERIAL_DEBUGGER)
	&msm_device_uart1,
#endif
	&qsd_device_spi,
	&bcm_bt_lpm_device,
	&msm_device_uart_dm1,
	&ram_console_device,
	&mahimahi_rfkill,
	&mahimahi_rtc,
	&msm_device_dmov,
	&msm_device_smd,
	&msm_device_nand,
	&msm_device_otg,
#if 0
	&msm_device_hsusb,
	&usb_mass_storage_device,
#endif
#ifdef CONFIG_USB_ANDROID_RNDIS
	&rndis_device,
#endif
#if 0
	&android_usb_device,
#endif
	&android_pmem_device,
	&android_pmem_adsp_device,
#ifdef CONFIG_720P_CAMERA
	&android_pmem_venc_device,
#endif
	&msm_kgsl_3d0,
	&msm_device_i2c,
	&capella_cm3602,
	&msm_camera_sensor_s5k3e2fx,
	&mahimahi_flashlight_device,
};

static uint32_t bt_gpio_table[] = {
	PCOM_GPIO_CFG(MAHIMAHI_GPIO_BT_UART1_RTS, 2, GPIO_OUTPUT,
		      GPIO_PULL_UP, GPIO_8MA),
	PCOM_GPIO_CFG(MAHIMAHI_GPIO_BT_UART1_CTS, 2, GPIO_INPUT,
		      GPIO_PULL_UP, GPIO_8MA),
	PCOM_GPIO_CFG(MAHIMAHI_GPIO_BT_UART1_RX, 2, GPIO_INPUT,
		      GPIO_PULL_UP, GPIO_8MA),
	PCOM_GPIO_CFG(MAHIMAHI_GPIO_BT_UART1_TX, 2, GPIO_OUTPUT,
		      GPIO_PULL_UP, GPIO_8MA),
	PCOM_GPIO_CFG(MAHIMAHI_GPIO_BT_RESET_N, 0, GPIO_OUTPUT,
		      GPIO_PULL_DOWN, GPIO_4MA),
	PCOM_GPIO_CFG(MAHIMAHI_GPIO_BT_SHUTDOWN_N, 0, GPIO_OUTPUT,
		      GPIO_PULL_DOWN, GPIO_4MA),
	PCOM_GPIO_CFG(MAHIMAHI_GPIO_BT_WAKE, 0, GPIO_OUTPUT,
		      GPIO_PULL_DOWN, GPIO_4MA),
	PCOM_GPIO_CFG(MAHIMAHI_GPIO_BT_HOST_WAKE, 0, GPIO_INPUT,
		      GPIO_PULL_DOWN, GPIO_4MA),
};

static uint32_t bt_gpio_table_rev_CX[] = {
	PCOM_GPIO_CFG(MAHIMAHI_GPIO_BT_UART1_RTS, 2, GPIO_OUTPUT,
		      GPIO_PULL_UP, GPIO_8MA),
	PCOM_GPIO_CFG(MAHIMAHI_GPIO_BT_UART1_CTS, 2, GPIO_INPUT,
		      GPIO_PULL_UP, GPIO_8MA),
	PCOM_GPIO_CFG(MAHIMAHI_GPIO_BT_UART1_RX, 2, GPIO_INPUT,
		      GPIO_PULL_UP, GPIO_8MA),
	PCOM_GPIO_CFG(MAHIMAHI_GPIO_BT_UART1_TX, 2, GPIO_OUTPUT,
		      GPIO_PULL_UP, GPIO_8MA),
	PCOM_GPIO_CFG(MAHIMAHI_GPIO_BT_RESET_N, 0, GPIO_OUTPUT,
		      GPIO_PULL_DOWN, GPIO_4MA),
	PCOM_GPIO_CFG(MAHIMAHI_GPIO_BT_SHUTDOWN_N, 0, GPIO_OUTPUT,
		      GPIO_PULL_DOWN, GPIO_4MA),
	PCOM_GPIO_CFG(MAHIMAHI_CDMA_GPIO_BT_WAKE, 0, GPIO_OUTPUT,
		      GPIO_PULL_DOWN, GPIO_4MA),
	PCOM_GPIO_CFG(MAHIMAHI_GPIO_BT_HOST_WAKE, 0, GPIO_INPUT,
		      GPIO_PULL_DOWN, GPIO_4MA),
};

static uint32_t misc_gpio_table[] = {
	PCOM_GPIO_CFG(MAHIMAHI_GPIO_LCD_RST_N, 0, GPIO_OUTPUT,
		      GPIO_NO_PULL, GPIO_2MA),
	PCOM_GPIO_CFG(MAHIMAHI_GPIO_LED_3V3_EN, 0, GPIO_OUTPUT,
		      GPIO_NO_PULL, GPIO_2MA),
	PCOM_GPIO_CFG(MAHIMAHI_GPIO_DOCK, 0, GPIO_OUTPUT,
		      GPIO_NO_PULL, GPIO_4MA),
};

static uint32_t key_int_shutdown_gpio_table[] = {
	PCOM_GPIO_CFG(MAHIMAHI_GPIO_35MM_KEY_INT_SHUTDOWN, 0, GPIO_OUTPUT,
		      GPIO_NO_PULL, GPIO_2MA),
};

static void mahimahi_headset_init(void)
{
	if (is_cdma_version(system_rev))
		return;
	config_gpio_table(key_int_shutdown_gpio_table,
			ARRAY_SIZE(key_int_shutdown_gpio_table));
	gpio_set_value(MAHIMAHI_GPIO_35MM_KEY_INT_SHUTDOWN, 0);
}

#define ATAG_BDADDR 0x43294329  /* mahimahi bluetooth address tag */
#define ATAG_BDADDR_SIZE 4
#define BDADDR_STR_SIZE 18

static char bdaddr[BDADDR_STR_SIZE];

module_param_string(bdaddr, bdaddr, sizeof(bdaddr), 0400);
MODULE_PARM_DESC(bdaddr, "bluetooth address");

static int __init parse_tag_bdaddr(const struct tag *tag)
{
	unsigned char *b = (unsigned char *)&tag->u;

	if (tag->hdr.size != ATAG_BDADDR_SIZE)
		return -EINVAL;

	snprintf(bdaddr, BDADDR_STR_SIZE, "%02X:%02X:%02X:%02X:%02X:%02X",
			b[0], b[1], b[2], b[3], b[4], b[5]);

        return 0;
}

__tagtable(ATAG_BDADDR, parse_tag_bdaddr);

static int __init mahimahi_board_serialno_setup(char *serialno)
{
#ifdef CONFIG_USB_ANDROID_RNDIS
	int i;
	char *src = serialno;

	/* create a fake MAC address from our serial number.
	 * first byte is 0x02 to signify locally administered.
	 */
	rndis_pdata.ethaddr[0] = 0x02;
	for (i = 0; *src; i++) {
		/* XOR the USB serial across the remaining bytes */
		rndis_pdata.ethaddr[i % (ETH_ALEN - 1) + 1] ^= *src++;
	}
#endif

	android_usb_pdata.serial_number = serialno;
/*	msm_hsusb_pdata.serial_number = serialno;*/
	return 1;
}
__setup("androidboot.serialno=", mahimahi_board_serialno_setup);

# if 0
static struct msm_acpu_clock_platform_data mahimahi_clock_data = {
	.acpu_switch_time_us	= 20,
	.max_speed_delta_khz	= 256000,
	.vdd_switch_time_us	= 62,
	.power_collapse_khz	= 245000,
	.wait_for_irq_khz	= 245000,
	.mpll_khz		= 245000
};

static struct msm_acpu_clock_platform_data mahimahi_cdma_clock_data = {
	.acpu_switch_time_us	= 20,
	.max_speed_delta_khz	= 256000,
	.vdd_switch_time_us	= 62,
	.power_collapse_khz	= 235930,
	.wait_for_irq_khz	= 235930,
	.mpll_khz		= 235930
};
#endif

#ifdef CONFIG_PERFLOCK
static unsigned mahimahi_perf_acpu_table[] = {
  245000000,
  576000000,
  998400000,
};

static struct perflock_platform_data mahimahi_perflock_data = {
  .perf_acpu_table = mahimahi_perf_acpu_table,
  .table_size = ARRAY_SIZE(mahimahi_perf_acpu_table),
};
#endif

static ssize_t mahimahi_virtual_keys_show(struct kobject *kobj,
			       struct kobj_attribute *attr, char *buf)
{
	if (system_rev > 2 && system_rev != 0xC0) {
		/* center: x: back: 60, menu: 172, home: 298, search 412, y: 840 */
		return sprintf(buf,
			__stringify(EV_KEY) ":" __stringify(KEY_BACK)  ":55:840:90:60"
		   ":" __stringify(EV_KEY) ":" __stringify(KEY_MENU)   ":172:840:125:60"
		   ":" __stringify(EV_KEY) ":" __stringify(KEY_HOME)   ":298:840:115:60"
		   ":" __stringify(EV_KEY) ":" __stringify(KEY_SEARCH) ":412:840:95:60"
		   "\n");
	} else {
		/* center: x: home: 55, menu: 185, back: 305, search 425, y: 835 */
		return sprintf(buf,
			__stringify(EV_KEY) ":" __stringify(KEY_HOME)  ":55:835:70:55"
		   ":" __stringify(EV_KEY) ":" __stringify(KEY_MENU)   ":185:835:100:55"
		   ":" __stringify(EV_KEY) ":" __stringify(KEY_BACK)   ":305:835:70:55"
		   ":" __stringify(EV_KEY) ":" __stringify(KEY_SEARCH) ":425:835:70:55"
		   "\n");
	}
}

static struct kobj_attribute mahimahi_virtual_keys_attr = {
	.attr = {
		.name = "virtualkeys.synaptics-rmi-touchscreen",
		.mode = S_IRUGO,
	},
	.show = &mahimahi_virtual_keys_show,
};

static struct attribute *mahimahi_properties_attrs[] = {
	&mahimahi_virtual_keys_attr.attr,
	NULL
};

static struct attribute_group mahimahi_properties_attr_group = {
	.attrs = mahimahi_properties_attrs,
};

static void mahimahi_reset(void)
{
	gpio_set_value(MAHIMAHI_GPIO_PS_HOLD, 0);
}

static void do_grp_reset(void)
{
	writel(0x20000, MSM_CLK_CTL_BASE + 0x214);
}

static void do_sdc1_reset(void)
{
	volatile uint32_t* sdc1_clk = MSM_CLK_CTL_BASE + 0x218;

	*sdc1_clk |= (1 << 9);
	mdelay(1);
	*sdc1_clk &= ~(1 << 9);
}

static struct msm_pm_boot_platform_data msm_pm_boot_pdata __initdata = {
	.mode = MSM_PM_BOOT_CONFIG_RESET_VECTOR_VIRT,
	.v_addr = (unsigned int *)PAGE_OFFSET,
};

#define GPIO_I2C_CLK 95
#define GPIO_I2C_DAT 96

static void msm_i2c_gpio_config(int adap_id, int config_type)
{
	unsigned id;


	if (adap_id > 0) return;

	if (config_type == 0)
	{
		id = GPIO_CFG(GPIO_I2C_CLK, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA);
		msm_proc_comm(PCOM_RPC_GPIO_TLMM_CONFIG_EX, &id, 0);
		id = GPIO_CFG(GPIO_I2C_DAT, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA);
		msm_proc_comm(PCOM_RPC_GPIO_TLMM_CONFIG_EX, &id, 0);
	}
	else
	{
		id = GPIO_CFG(GPIO_I2C_CLK, 1, GPIO_INPUT, GPIO_NO_PULL, GPIO_8MA);
		msm_proc_comm(PCOM_RPC_GPIO_TLMM_CONFIG_EX, &id, 0);
		id = GPIO_CFG(GPIO_I2C_DAT , 1, GPIO_INPUT, GPIO_NO_PULL, GPIO_8MA);
		msm_proc_comm(PCOM_RPC_GPIO_TLMM_CONFIG_EX, &id, 0);
	}
}

static struct msm_i2c_platform_data msm_i2c_pdata = 
{
	.clk_freq = 100000,
	.pri_clk = GPIO_I2C_CLK,
	.pri_dat = GPIO_I2C_DAT,
	.rmutex  = 0,
	.msm_i2c_config_gpio = msm_i2c_gpio_config,
};

static void __init msm_device_i2c_init(void)
{
	msm_i2c_gpio_init();
	msm_device_i2c.dev.platform_data = &msm_i2c_pdata;
}

static struct msm_pm_platform_data msm_pm_data[MSM_PM_SLEEP_MODE_NR] = {
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE] = {
		.idle_supported = 1,
		.suspend_supported = 1,
		.idle_enabled = 1,
		.suspend_enabled = 1,
		.latency = 8594,
		.residency = 23740,
	},
	[MSM_PM_SLEEP_MODE_APPS_SLEEP] = {
		.idle_supported = 1,
		.suspend_supported = 1,
		.idle_enabled = 1,
		.suspend_enabled = 1,
		.latency = 8594,
		.residency = 23740,
	},
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE_STANDALONE] = {
#ifdef CONFIG_MSM_STANDALONE_POWER_COLLAPSE
		.idle_supported = 1,
		.suspend_supported = 1,
		.idle_enabled = 1,
		.suspend_enabled = 0,
#else /*CONFIG_MSM_STANDALONE_POWER_COLLAPSE*/
		.idle_supported = 0,
		.suspend_supported = 0,
		.idle_enabled = 0,
		.suspend_enabled = 0,
#endif /*CONFIG_MSM_STANDALONE_POWER_COLLAPSE*/
		.latency = 500,
		.residency = 6000,
	},
	[MSM_PM_SLEEP_MODE_RAMP_DOWN_AND_WAIT_FOR_INTERRUPT] = {
		.idle_supported = 1,
		.suspend_supported = 1,
		.idle_enabled = 0,
		.suspend_enabled = 1,
		.latency = 443,
		.residency = 1098,
	},
	[MSM_PM_SLEEP_MODE_WAIT_FOR_INTERRUPT] = {
		.idle_supported = 1,
		.suspend_supported = 1,
		.idle_enabled = 1,
		.suspend_enabled = 1,
		.latency = 2,
		.residency = 0,
	},
};

static struct msm_gpio bma_spi_gpio_config_data[] = {
	{ GPIO_CFG(22, 0, GPIO_CFG_INPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA), "bma_irq" },
};

static int msm_bma_gpio_setup(struct device *dev)
{
	int rc;

	rc = msm_gpios_enable(bma_spi_gpio_config_data,
		ARRAY_SIZE(bma_spi_gpio_config_data));

	return rc;
}

static void msm_bma_gpio_teardown(struct device *dev)
{
	msm_gpios_disable_free(bma_spi_gpio_config_data,
		ARRAY_SIZE(bma_spi_gpio_config_data));
}

static struct bma150_platform_data bma_pdata = {
	.setup    = msm_bma_gpio_setup,
	.teardown = msm_bma_gpio_teardown,
};


static struct spi_board_info msm_spi_board_info[] __initdata = {
	{
		.modalias	= "bma150",
		.mode		= SPI_MODE_3,
		.irq		= MSM_GPIO_TO_INT(22),
		.bus_num	= 0,
		.chip_select	= 0,
		.max_speed_hz	= 10000000,
		.platform_data	= &bma_pdata,
	},
};

#define CT_CSR_PHYS		0xA8700000
#define TCSR_SPI_MUX		(ct_csr_base + 0x54)
static int msm_qsd_spi_dma_config(void)
{
	void __iomem *ct_csr_base = 0;
	u32 spi_mux;
	int ret = 0;

	ct_csr_base = ioremap(CT_CSR_PHYS, PAGE_SIZE);
	if (!ct_csr_base) {
		pr_err("%s: Could not remap %x\n", __func__, CT_CSR_PHYS);
		return -1;
	}

	spi_mux = readl(TCSR_SPI_MUX);
	switch (spi_mux) {
	case (1):
		qsd_spi_resources[4].start  = DMOV_HSUART1_RX_CHAN;
		qsd_spi_resources[4].end    = DMOV_HSUART1_TX_CHAN;
		qsd_spi_resources[5].start  = DMOV_HSUART1_RX_CRCI;
		qsd_spi_resources[5].end    = DMOV_HSUART1_TX_CRCI;
		break;
	case (2):
		qsd_spi_resources[4].start  = DMOV_HSUART2_RX_CHAN;
		qsd_spi_resources[4].end    = DMOV_HSUART2_TX_CHAN;
		qsd_spi_resources[5].start  = DMOV_HSUART2_RX_CRCI;
		qsd_spi_resources[5].end    = DMOV_HSUART2_TX_CRCI;
		break;
	case (3):
		qsd_spi_resources[4].start  = DMOV_CE_OUT_CHAN;
		qsd_spi_resources[4].end    = DMOV_CE_IN_CHAN;
		qsd_spi_resources[5].start  = DMOV_CE_OUT_CRCI;
		qsd_spi_resources[5].end    = DMOV_CE_IN_CRCI;
		break;
	default:
		ret = -1;
	}

	iounmap(ct_csr_base);
	return ret;
}

static uint32_t qsd_spi_gpio_config_data[] = {
	PCOM_GPIO_CFG(17, 1, GPIO_CFG_INPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	PCOM_GPIO_CFG(18, 1, GPIO_CFG_INPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	PCOM_GPIO_CFG(19, 1, GPIO_CFG_INPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	PCOM_GPIO_CFG(20, 1, GPIO_CFG_INPUT,  GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	PCOM_GPIO_CFG(21, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_16MA),
};

static int msm_qsd_spi_gpio_config(void)
{
	int rc;

	config_gpio_table(qsd_spi_gpio_config_data,
		ARRAY_SIZE(qsd_spi_gpio_config_data));

	/* Set direction for SPI_PWR */
	gpio_direction_output(21, 1);

	return 0;
}

static void msm_qsd_spi_gpio_release(void)
{
	msm_gpios_disable_free(qsd_spi_gpio_config_data,
		ARRAY_SIZE(qsd_spi_gpio_config_data));
}

static struct msm_spi_platform_data qsd_spi_pdata = {
	.max_clock_speed = 19200000,
	.gpio_config  = msm_qsd_spi_gpio_config,
	.gpio_release = msm_qsd_spi_gpio_release,
	.dma_config = msm_qsd_spi_dma_config,
};

static void __init msm_qsd_spi_init(void)
{
	int rc;
	rc = gpio_request(21, "spi_pwr");
	if (rc)
		pr_err("Failed requesting spi_pwr gpio\n");
	qsd_device_spi.dev.platform_data = &qsd_spi_pdata;
}

int mahimahi_init_mmc(int sysrev, unsigned debug_uart);

#if 0
static const struct smd_tty_channel_desc smd_cdma_default_channels[] = {
	{ .id = 0, .name = "SMD_DS" },
	{ .id = 19, .name = "SMD_DATA3" },
	{ .id = 27, .name = "SMD_GPSNMEA" }
};
#endif

static void __init mahimahi_init(void)
{
	int ret;
	struct kobject *properties_kobj;

	printk("mahimahi_init() revision=%d\n", system_rev);

#if 0
	if (is_cdma_version(system_rev))
		smd_set_channel_list(smd_cdma_default_channels,
					ARRAY_SIZE(smd_cdma_default_channels));
#endif

	msm_hw_reset_hook = mahimahi_reset;

	mahimahi_board_serialno_setup(board_serialno());

	do_grp_reset();
	do_sdc1_reset();

#if 0
	if (is_cdma_version(system_rev))
		msm_acpu_clock_init(&mahimahi_cdma_clock_data);
	else
		msm_acpu_clock_init(&mahimahi_clock_data);
#else
	msm_clock_init(&qds8x50_clock_init_data);
	acpuclk_init(&acpuclk_8x50_soc_data);
#endif

#ifdef CONFIG_PERFLOCK
	perflock_init(&mahimahi_perflock_data);
#endif

	msm_serial_debug_init(MSM_UART1_PHYS, INT_UART1,
			      &msm_device_uart1.dev, 1, MSM_GPIO_TO_INT(139));

	config_gpio_table(misc_gpio_table, ARRAY_SIZE(misc_gpio_table));

	if (is_cdma_version(system_rev)) {
		bcm_bt_lpm_pdata.gpio_wake = MAHIMAHI_CDMA_GPIO_BT_WAKE;
		mahimahi_flashlight_data.torch = MAHIMAHI_CDMA_GPIO_FLASHLIGHT_TORCH;
		config_gpio_table(bt_gpio_table_rev_CX, ARRAY_SIZE(bt_gpio_table_rev_CX));
	} else {
		config_gpio_table(bt_gpio_table, ARRAY_SIZE(bt_gpio_table));
	}

	gpio_request(MAHIMAHI_GPIO_TP_LS_EN, "tp_ls_en");
	gpio_direction_output(MAHIMAHI_GPIO_TP_LS_EN, 0);
	gpio_request(MAHIMAHI_GPIO_TP_EN, "tp_en");
	gpio_direction_output(MAHIMAHI_GPIO_TP_EN, 0);
	gpio_request(MAHIMAHI_GPIO_PROXIMITY_EN, "proximity_en");
	gpio_direction_output(MAHIMAHI_GPIO_PROXIMITY_EN, 1);
	gpio_request(MAHIMAHI_GPIO_COMPASS_RST_N, "compass_rst");
	gpio_direction_output(MAHIMAHI_GPIO_COMPASS_RST_N, 1);
	gpio_request(MAHIMAHI_GPIO_COMPASS_INT_N, "compass_int");
	gpio_direction_input(MAHIMAHI_GPIO_COMPASS_INT_N);

	gpio_request(MAHIMAHI_GPIO_DS2482_SLP_N, "ds2482_slp_n");

#if 0
	msm_device_hsusb.dev.platform_data = &msm_hsusb_pdata;
#endif
	msm_device_uart_dm1.dev.platform_data = &msm_uart_dm1_pdata;

	platform_add_devices(devices, ARRAY_SIZE(devices));

	platform_add_devices(msm_footswitch_devices,
			msm_num_footswitch_devices);

	msm_device_i2c_init();
	msm_qsd_spi_init();

	i2c_register_board_info(0, base_i2c_devices,
		ARRAY_SIZE(base_i2c_devices));

	spi_register_board_info(msm_spi_board_info,
		ARRAY_SIZE(msm_spi_board_info));

	if (system_rev == 0) {
		/* Only board after XB with Audience A1026 */
		i2c_register_board_info(0, rev0_i2c_devices,
			ARRAY_SIZE(rev0_i2c_devices));
	}

	if (system_rev > 0) {
		/* Only board after XB with Audience A1026 */
		i2c_register_board_info(0, rev1_i2c_devices,
			ARRAY_SIZE(rev1_i2c_devices));
	}

	if (is_cdma_version(system_rev)) {
		/* Only CDMA version with TI TPA2018D1 Speaker Amp. */
		i2c_register_board_info(0, rev_CX_i2c_devices,
			ARRAY_SIZE(rev_CX_i2c_devices));
		if ((system_rev & 0x0F) == 0x00) {
			a1026_data.gpio_a1026_clk = MAHIMAHI_CDMA_XA_AUD_A1026_CLK;
		} else if ((system_rev & 0x0F) >= 0x01) {
			a1026_data.gpio_a1026_wakeup = MAHIMAHI_CDMA_XB_AUD_A1026_WAKEUP;
			a1026_data.gpio_a1026_reset = MAHIMAHI_CDMA_XB_AUD_A1026_RESET;
			a1026_data.gpio_a1026_clk = MAHIMAHI_CDMA_XB_AUD_A1026_CLK;
		}
	}

	ret = mahimahi_init_mmc(system_rev, debug_uart);
	if (ret != 0)
		pr_crit("%s: Unable to initialize MMC\n", __func__);

	msm_pm_set_platform_data(msm_pm_data, ARRAY_SIZE(msm_pm_data));

	properties_kobj = kobject_create_and_add("board_properties", NULL);
	if (properties_kobj)
		ret = sysfs_create_group(properties_kobj,
					 &mahimahi_properties_attr_group);
	if (!properties_kobj || ret)
		pr_err("failed to create board_properties\n");

#ifdef CONFIG_USB_G_ANDROID
	mahimahi_add_usb_devices();
#endif

	mahimahi_audio_init();
	mahimahi_headset_init();

	if (system_rev > 0)
		platform_device_register(&mahimahi_timed_gpios);
	else
		msm_init_pmic_vibrator();

	ds2784_battery_init();
}

static void __init mahimahi_fixup(struct machine_desc *desc, struct tag *tags,
				 char **cmdline, struct meminfo *mi)
{
	mi->nr_banks = 2;
	mi->bank[0].start = PHYS_OFFSET;
	mi->bank[0].size = MSM_EBI1_BANK0_SIZE;
	mi->bank[1].start = MSM_EBI1_BANK1_BASE;
	mi->bank[1].size = MSM_EBI1_BANK1_SIZE;
}

static void __init mahimahi_init_irq(void)
{
	msm_init_irq();
	msm_init_sirc();
}

static void __init mahimahi_map_io(void)
{
#if 0
	msm_map_qsd8x50_io();
	msm_clock_init(msm_clocks_8x50, msm_num_clocks_8x50);
#else
	msm_map_qsd8x50_io();
#endif
	if (socinfo_init() < 0)
		printk(KERN_ERR "%s: socinfo_init() failed!\n",__func__);
}

extern struct sys_timer msm_timer;

MACHINE_START(MAHIMAHI, "mahimahi")
#ifdef CONFIG_MSM_DEBUG_UART
#endif
	.boot_params	= 0x20000100,
	.fixup		= mahimahi_fixup,
	.map_io		= mahimahi_map_io,
	.reserve	= mahimahi_reserve,
	.init_irq	= mahimahi_init_irq,
	.init_machine	= mahimahi_init,
	.timer		= &msm_timer,
MACHINE_END
