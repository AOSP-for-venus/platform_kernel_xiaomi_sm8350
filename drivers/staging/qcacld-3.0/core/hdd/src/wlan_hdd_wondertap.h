/*
 * wlan_hdd_wondertap.h
 *
 * qcacld-3.0 <-> Android "wonder" auxiliary-bus glue.
 *
 * This registers an auxiliary_device named "wondertap" so the GKI
 * drivers/android/wonder module can bind to this driver and drive it
 * through struct wondertap_ops, the same way it drives bcmdhd4390 on
 * Pixel. See wlan_hdd_wondertap.c for the implementation and for the
 * list of callbacks that are still stubbed out pending lower-layer
 * (WMA/WMI/TSF) support.
 *
 * IMPORTANT: for the "wonder" module's auxiliary_driver to actually probe
 * this device, drivers/android/wonder/main.c's wonder_aux_id_table[] must
 * contain an entry matching "<your module KBUILD_MODNAME>.wondertap"
 * (e.g. "wlan.wondertap" for the stock qcacld-3.0 MODNAME). That is a
 * kernel-tree change, not a qcacld change -- see the accompanying patch
 * note.
 */

#ifndef __WLAN_HDD_WONDERTAP_H__
#define __WLAN_HDD_WONDERTAP_H__

#ifdef WONDERTAP

/**
 * hdd_wondertap_register() - create and register the "wondertap"
 *                             auxiliary_device so the wonder module can
 *                             bind to it.
 *
 * Call from hdd_driver_load(), after wlan_hdd_register_driver() has
 * succeeded (so hdd_ctx / wiphy are guaranteed to exist by the time the
 * wonder module's probe() runs and calls back into get_capabilities()).
 *
 * Return: 0 on success, negative errno on failure.
 */
int hdd_wondertap_register(void);

/**
 * hdd_wondertap_unregister() - tear down the "wondertap" auxiliary_device.
 *
 * Call from hdd_driver_unload(), before wlan_hdd_unregister_driver(), so
 * the wonder module releases its reference to our ops before the HDD
 * context underneath them goes away.
 */
void hdd_wondertap_unregister(void);

#else

static inline int hdd_wondertap_register(void)
{
	return 0;
}

static inline void hdd_wondertap_unregister(void)
{
}

#endif /* WONDERTAP */

#endif /* __WLAN_HDD_WONDERTAP_H__ */
