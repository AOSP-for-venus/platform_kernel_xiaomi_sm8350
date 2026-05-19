// SPDX-License-Identifier: GPL-2.0
/*
 * Google Wonder WiFi Virtual Soft-MAC Driver
 *
 * Main entry point for the Wonder driver module. This file handles the
 * initialization and cleanup of the driver, locating the physical
 * network device and kicking off the mac80211 registration process.
 */

#include <linux/auxiliary_bus.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/component.h>

#include "core.h"
#include "mac80211.h"
#include "wondertap_internal.h"

/* Module parameter for setting the physical device name */
module_param(physical_name, charp, 0444);
MODULE_PARM_DESC(physical_name, "Interface name to use (e.g., wlan0, radiotap0, ...)");

#define WONDER_MAX_COMPAT_VERSIONS 6
static int wonder_ver_match_table[WONDER_MAX_COMPAT_VERSIONS] = {
	WONDER_VERSION_3_4,
	WONDER_VERSION_3_5,
	WONDER_VERSION_3_6_4,
	WONDER_VERSION_3_6_3,
	WONDER_VERSION_3_6_5,
	-1,
};

static bool wonder_ver_can_support(enum wondertap_ver device_ver, enum wondertap_ver driver_ver)
{
	int i;
	int ver = driver_ver - WONDER_VERSION_AUX_BASE;

	if (ver < 0 || driver_ver >= WONDER_VERSION_MAX)
		return false;

	for (i = 0; i < WONDER_MAX_COMPAT_VERSIONS; i++) {
		if (wonder_ver_match_table[i] == -1)
			break;
		if (wonder_ver_match_table[i] == device_ver)
			return true;
	}
	return false;
}

static int wonder_probe(struct auxiliary_device *adev,
			const struct auxiliary_device_id *id)
{
	struct wondertap_aux_dev *wonder_adev = container_of(adev, struct wondertap_aux_dev, adev);
	struct wonder_data *wonder;
	struct wondertap_data *wondertap;
	struct device *dev = &wonder_adev->adev.dev;
	int ret;

	wonder = wonder_mac80211_init();
	if (!wonder)
		return -ENODEV;

	wondertap = &wonder->wondertap_data;
	/* Assign wondertap interface version will be used in the match process. */
	wondertap->ver = WONDER_VERSION_3_6_5;
	auxiliary_set_drvdata(&wonder_adev->adev, wonder);

	if (!wonder_ver_can_support(wonder_adev->ver, wondertap->ver)) {
		dev_err(dev, "%s(): wondertap interface version mismatch(%d,%d)!\n",
			__func__, wonder_adev->ver, wondertap->ver);
		wonder_mac80211_exit(wonder);
		return -EINVAL;
	}

	/* All matched, hook the ops to wondertap interface. */
	wondertap->wonder_ops = wonder_adev->wonder_ops;
	wondertap->wifi_ver = wonder_adev->ver;
	dev_dbg(dev, "%s(): Connected to wlan ver %d (cur: wonder ver %d)!\n",
		__func__, wonder_adev->ver, wondertap->ver);

	ret = wondertap_get_capabilities(wondertap, &wondertap->cap);
	if (ret) {
		dev_err(dev, "Failed to get wondertap capabilities, error: %d\n", ret);
		return ret;
	}
	wonder_debugfs_init(wonder);

	return 0;
}

static void wonder_remove(struct auxiliary_device *adev)
{
	struct wonder_data *wonder = auxiliary_get_drvdata(adev);

	wonder_debugfs_exit();
	wonder_mac80211_exit(wonder);
}

static const struct auxiliary_device_id wonder_aux_id_table[] = {
	{ .name = "bcmdhd4390.wondertap" },
	{},
};
MODULE_DEVICE_TABLE(auxiliary, wonder_aux_id_table);

static struct auxiliary_driver wonder_driver = {
	.name = "wondertap",
	.probe = wonder_probe,
	.remove = wonder_remove,
	.driver = {
		.name = "wonder",
	},
	.id_table = wonder_aux_id_table,
};
module_auxiliary_driver(wonder_driver);

MODULE_DESCRIPTION("Google Wonder Virtual mac80211 Driver");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Google Android WiFi Team");
