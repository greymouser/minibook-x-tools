// SPDX-License-Identifier: GPL-2.0-only
/*
 * CHUWI Minibook X platform driver
 *
 * Copyright (c) 2025
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/acpi.h>

#define CMX_DRV_NAME "cmx"

static int __init cmx_init(void)
{
	pr_info("CHUWI Minibook X platform driver loaded\n");
	return 0;
}

static void __exit cmx_exit(void)
{
	pr_info("CHUWI Minibook X platform driver unloaded\n");
}

module_init(cmx_init);
module_exit(cmx_exit);

MODULE_AUTHOR("Armando DiCianno <armando@noonshy.com>");
MODULE_DESCRIPTION("CHUWI Minibook X Platform Driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:" CMX_DRV_NAME);
