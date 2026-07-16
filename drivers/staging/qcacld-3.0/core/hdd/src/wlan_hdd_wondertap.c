/*
 * wlan_hdd_wondertap.c
 *
 * qcacld-3.0's vendor-side implementation of the "wondertap" interface
 * consumed by the Android common-kernel "wonder" driver
 * (drivers/android/wonder/), which provides the AWDL-style transport
 * backing AirDrop-compatible Quick Share.
 *
 * This is the QCA analogue of Broadcom's dhd_custom_google.c WONDERTAP
 * block (bcm4390_dhd_custom_google.c in your upload), adapted for two
 * differences:
 *
 *   1. qcacld-3.0 has no standing "monitor_dev" the way dhd does --
 *      a monitor-mode adapter is created on demand here via
 *      wlan_hdd_add_monitor_check(), mirroring how Broadcom re-used its
 *      pre-existing monitor netdev.
 *
 *   2. The GKI wonder module binds over the auxiliary_bus
 *      (auxiliary_device / auxiliary_driver), not the older
 *      platform_driver + component_ops model your bcm4390 sample used
 *      at WONDER_VERSION_1_4.
 *
 * HONEST STATUS after this round (driver_cmd_nl80211.c, wlan_hdd_ioctl.*,
 * wlan_hdd_hostapd.c, wlan_vdev_mgr_utils_api.*, wlan_hdd_twt.c,
 * wlan_mlme_twt_*, wma_twt.*, sme_nan_datapath.h, cfg_mlme_twt.h):
 *
 * BUGFIX: set_tx_rate_mask previously read params->preamble/nss/mcs --
 * those fields don't exist on struct wondertap_tx_rate_mask_params (it's
 * max_preamble/max_nss/max_mcs, a different struct than
 * wondertap_fixed_tx_rate_params, which does use the unprefixed names).
 * My error, caught by an actual compile -- fixed below.
 *
 * set_tx_rate_mask: send path now confirmed for real --
 * wlan_util_vdev_mlme_set_ratemask_config(vdev_mlme) (exported from
 * wlan_vdev_mgr_utils_api.c) reads the mask to send out of
 * vdev_mlme->mgmt.rate_info and pushes it via WMI_VDEV_RATEMASK_CMDID.
 * Two narrow gaps remain: the adapter->vdev -> vdev_mlme_obj accessor
 * (not in any file seen so far), and the bit<->MCS/NSS layout within the
 * mask words (still undefined anywhere reviewed).
 *
 * channel_schedule_request: THIRD candidate (TWT) now checked and ruled
 * out -- TWT is single-channel STA<->AP power-save scheduling, unrelated
 * to multi-channel hopping; "channel" only appears in TWT's files as a
 * status code for colliding with an unrelated CSA event. P2P-LO, NAN, and
 * TWT are the three realistic candidates in this codebase and all three
 * are now ruled out for different reasons. See the function body for
 * what that implies going forward.
 *
 * set_station_info: sme_nan_datapath.h (re-sent) confirms there's no
 * explicit "add remote peer" SME entry point at all for NDI -- NDI peer
 * creation happens implicitly through csr_roam_start_ndi()'s NDI BSS
 * start, not via an arbitrary-MAC add call. Reinforces the earlier
 * finding that the NAN/NDI peer path doesn't cleanly transfer to
 * wondertap. Still stubbed.
 *
 * See INTEGRATION_NOTES.md for the current per-op table.
 */

#ifdef WONDERTAP

#include <linux/auxiliary_bus.h>
#include <linux/android/wondertap.h>
#include <net/cfg80211.h>

#include "wlan_hdd_main.h"
#include "wlan_hdd_regulatory.h"
#include "wlan_hdd_wondertap.h"
#include "wlan_hdd_tsf.h"
/* wlan_util_vdev_mlme_set_ratemask_config(), used in
 * hdd_wondertap_set_tx_rate_mask() below, is declared here.
 */
#include "wlan_vdev_mgr_utils_api.h"
/* wma_cli_set_command()/wma_cli_get_command() and the VDEV_CMD/GEN_CMD
 * macros are declared here (confirmed against your upload) -- included
 * explicitly rather than relying on wlan_hdd_main.h to pull it in
 * transitively.
 */
#include "wma_api.h"
/* WMI_RATE_PREAMBLE_* (used below) is NOT in wma_api.h or
 * wmi_unified_param.h -- it's pulled in the same way wlan_hdd_main.c gets
 * it, via wni_api.h. See the comment on hdd_wondertap_set_fixed_tx_rate()
 * for what's still unverified about it.
 */
#include <wni_api.h>
/* enum wmi_ratemask_type (WMI_RATEMASK_TYPE_CCK/HT/VHT/HE), used in
 * hdd_wondertap_set_tx_rate_mask() below, is confirmed in this header.
 */
#include "wmi_unified_param.h"

/* Sub-device name; full auxiliary device name becomes
 * "<KBUILD_MODNAME>.wondertap" (e.g. "wlan.wondertap" for the default
 * qcacld-3.0 MODNAME). This is the string that must appear in
 * drivers/android/wonder/main.c's wonder_aux_id_table[].
 */
#define HDD_WONDERTAP_DEV_NAME "wondertap"

struct hdd_wondertap_priv {
	struct wondertap_aux_dev adev;
	struct hdd_adapter *mon_adapter;
	bool registered;
	bool active;
};

static struct hdd_wondertap_priv g_wondertap;

static void hdd_wondertap_release(struct device *dev)
{
	/* g_wondertap.adev is static, nothing to free here. */
}

/* --------------------------------------------------------------------- *
 * wondertap_ops callbacks
 * --------------------------------------------------------------------- */

static int hdd_wondertap_set_freq(void *handle,
				   const struct wondertap_set_freq_params *params)
{
	struct hdd_context *hdd_ctx = handle;
	struct ieee80211_channel *chan;
	struct cfg80211_chan_def chandef = {0};
	enum nl80211_chan_width width;

	if (!g_wondertap.mon_adapter) {
		hdd_err("wondertap: set_freq called with no monitor adapter");
		return -ENODEV;
	}

	chan = ieee80211_get_channel(hdd_ctx->wiphy, params->freq);
	if (!chan) {
		hdd_err("wondertap: freq %u not found on this wiphy",
			params->freq);
		return -EINVAL;
	}

	switch (params->bandwidth) {
	case WONDERTAP_RATE_BW_40:
		width = NL80211_CHAN_WIDTH_40;
		break;
	case WONDERTAP_RATE_BW_80:
		width = NL80211_CHAN_WIDTH_80;
		break;
	case WONDERTAP_RATE_BW_160:
		width = NL80211_CHAN_WIDTH_160;
		break;
	default:
		width = NL80211_CHAN_WIDTH_20;
		break;
	}

	cfg80211_chandef_create(&chandef, chan, NL80211_CHAN_NO_HT);
	chandef.width = width;

	/*
	 * NOTE: wlan_hdd_set_channel() is qcacld's cfg80211 .set_channel
	 * callback. It's normally invoked by the nl80211 layer against a
	 * fully set-up net_device in response to a real user-space request.
	 * Driving it directly here works for the monitor-mode case in
	 * testing, but its concurrency/lock assumptions (e.g. anything that
	 * expects RTNL or a specific osif_sync transition to already be
	 * held) haven't been verified against this call site -- that
	 * implementation lives in wlan_hdd_cfg80211.c, which wasn't in the
	 * files provided. If you see lock asserts here, start there.
	 */
	return wlan_hdd_set_channel(hdd_ctx->wiphy, g_wondertap.mon_adapter->dev,
				     &chandef, NL80211_CHAN_HT20);
}

static int hdd_wondertap_set_fixed_tx_rate(void *handle,
			const struct wondertap_fixed_tx_rate_params *params)
{
	struct hdd_adapter *adapter = g_wondertap.mon_adapter;
	uint8_t preamble, nss, rix;
	int set_value;

	if (!adapter)
		return -ENODEV;

	/*
	 * Status after reviewing wma_api.h + wmi_unified_param.h:
	 *
	 * - VDEV_CMD, wma_cli_set_command(), and the enum member behind
	 *   WMI_VDEV_PARAM_FIXED_RATE (wmi_vdev_param_fixed_rate, at
	 *   wmi_unified_param.h) are now directly confirmed.
	 * - WMI_RATE_PREAMBLE_* itself is still NOT in either file --
	 *   wmi_unified_param.h only has a *different*, differently-valued
	 *   enum for a different purpose (wmi_host_preamble_type: OFDM=0,
	 *   CCK=1, HT=2, VHT=3, HE=4, used for packet_power_info_params /
	 *   rate-mask config, not fixed-rate assembly). WMI_RATE_PREAMBLE_HE
	 *   is only confirmed correct because wlan_hdd_main.c's
	 *   hdd_set_11ax_rate() actually uses it that way; HT/VHT/CCK below
	 *   are inferred by naming symmetry, not confirmed. Send wni_api.h
	 *   (included above, but not yet in hand) if you want these nailed
	 *   down instead of inferred.
	 */
	switch (params->preamble) {
	case WONDERTAP_RATE_PREAMBLE_HT:
		preamble = WMI_RATE_PREAMBLE_HT;
		break;
	case WONDERTAP_RATE_PREAMBLE_VHT:
		preamble = WMI_RATE_PREAMBLE_VHT;
		break;
	case WONDERTAP_RATE_PREAMBLE_HE:
		preamble = WMI_RATE_PREAMBLE_HE;
		break;
	case WONDERTAP_RATE_PREAMBLE_LEGACY:
	default:
		preamble = WMI_RATE_PREAMBLE_CCK;
		break;
	}

	nss = params->nss ? (params->nss - 1) : 0;
	rix = params->mcs;

	set_value = hdd_assemble_rate_code(preamble, nss, rix);

	hdd_debug("wondertap: fixed tx rate preamble=%u nss=%u mcs=%u -> code=0x%x",
		  preamble, nss, rix, set_value);

	return wma_cli_set_command(adapter->vdev_id, WMI_VDEV_PARAM_FIXED_RATE,
				    set_value, VDEV_CMD);
}

static int hdd_wondertap_set_tx_rate_mask(void *handle,
				const struct wondertap_tx_rate_mask_params *params)
{
	struct hdd_adapter *adapter = g_wondertap.mon_adapter;
	uint8_t type;

	if (!adapter || !adapter->vdev)
		return -ENODEV;

	/*
	 * Send path now confirmed for real from wlan_vdev_mgr_utils_api.c:
	 * wlan_util_vdev_mlme_set_ratemask_config(struct vdev_mlme_obj *) is
	 * a real, exported (qdf_export_symbol) function that reads the mask
	 * to send OUT OF the vdev_mlme object itself
	 * (vdev_mlme->mgmt.rate_info.{type,lower32,higher32,lower32_2}) and
	 * pushes it via WMI_VDEV_RATEMASK_CMDID -- it does NOT take the mask
	 * as a parameter. So the real calling convention is: populate those
	 * four fields on the vdev's mlme object, then call this function.
	 *
	 * One real, interesting finding along the way: that same source file
	 * never copies higher32_2 into the param it sends, even though
	 * struct config_ratemask_params has that field -- worth checking
	 * whether that's a known limitation on your version or something to
	 * flag upstream, not something to silently paper over here.
	 *
	 * Two things still genuinely missing, both narrow and specific:
	 *   1. The accessor from adapter->vdev (struct wlan_objmgr_vdev *)
	 *      to its struct vdev_mlme_obj * isn't in any file sent so far.
	 *      By general convention in this codebase it's likely something
	 *      like wlan_vdev_mlme_get_cmpt_obj(vdev), but that's inference,
	 *      not a confirmed symbol -- I don't want to call an invented
	 *      function name in code that's headed for a real device.
	 *   2. The bit<->MCS/NSS layout inside lower32/higher32/lower32_2 is
	 *      still not defined anywhere reviewed -- mechanical plumbing
	 *      only, not semantics.
	 *
	 * Once (1) is confirmed, the body of this function is just:
	 *   vdev_mlme->mgmt.rate_info.type = type;
	 *   vdev_mlme->mgmt.rate_info.lower32 = ...;
	 *   vdev_mlme->mgmt.rate_info.higher32 = ...;
	 *   vdev_mlme->mgmt.rate_info.lower32_2 = ...;
	 *   return qdf_status_to_os_return(
	 *           wlan_util_vdev_mlme_set_ratemask_config(vdev_mlme));
	 * -- (2) still needs solving separately before the mask values
	 * themselves mean anything.
	 */
	switch (params->max_preamble) {
	case WONDERTAP_RATE_PREAMBLE_HT:
		type = WMI_RATEMASK_TYPE_HT;
		break;
	case WONDERTAP_RATE_PREAMBLE_VHT:
		type = WMI_RATEMASK_TYPE_VHT;
		break;
	case WONDERTAP_RATE_PREAMBLE_HE:
		type = WMI_RATEMASK_TYPE_HE;
		break;
	case WONDERTAP_RATE_PREAMBLE_LEGACY:
	default:
		type = WMI_RATEMASK_TYPE_CCK;
		break;
	}
	(void)type;

	return -EOPNOTSUPP;
}

static int hdd_wondertap_set_filter(void *handle,
				     enum wondertap_filter_type filter_type,
				     const void *params)
{
	/*
	 * TODO: WONDERTAP_FILTER_TYPE_FRAME needs an RX frame type/subtype
	 * filter on the monitor vdev. Still not found.
	 *
	 * Checked and ruled out: wmi_unified_p2p_tlv.c has a real MAC-address
	 * RX filter (WMI_VDEV_ADD_MAC_ADDR_TO_RX_FILTER_CMDID, vdev_id + freq
	 * + mac + enable) that looked promising at first, but it's P2P's
	 * randomized-MAC action-frame filter, not a fit here. More
	 * importantly: re-reading the actual wondertap.h enum from your GKI
	 * patch, wondertap_filter_type only has ONE value
	 * (WONDERTAP_FILTER_TYPE_FRAME) -- BSSID filtering is a *separate*,
	 * non-vendor-facing function (wondertap_set_bssid_filter(), internal
	 * to the wonder module itself, presumably filtering already-received
	 * frames in software rather than asking the vendor driver to do it
	 * in hardware). So this op only ever needs to handle frame
	 * type/subtype, and the MAC-filter command doesn't apply regardless.
	 *
	 * wma_add_beacon_filter()/wma_remove_beacon_filter() also exist but
	 * operate on beacon-specific filtering (struct beacon_filter_param,
	 * not defined in the files given) -- a different, narrower thing.
	 */
	return -EOPNOTSUPP;
}

static int hdd_wondertap_get_capabilities(void *handle,
					   struct wondertap_capability *caps)
{
	memset(caps, 0, sizeof(*caps));
	caps->version = 0;

	/*
	 * Deliberately conservative, mirroring Broadcom's own
	 * dhd_wondertap_get_capabilities() (which only sets
	 * amsdu_aggregation). Only flip bits here once you've actually
	 * verified qcacld supports that behavior on this vdev --
	 * over-advertising (e.g. channel_hopping, nan_coexist) will make the
	 * generic wonder layer attempt operations this driver doesn't
	 * implement yet.
	 */
	caps->bits.dynamic_freq = 1;
	caps->bits.dynamic_fixed_tx_rate = 1;
	caps->maximum_channel_switch_time_us = 0; /* unknown -- measure and fill in */

	return 0;
}

static int hdd_wondertap_get_mac_tsf(void *handle, u32 *mac_tsf)
{
	struct hdd_adapter *adapter = g_wondertap.mon_adapter;
	uint64_t tsf = 0;
	int errno;

	if (!adapter)
		return -ENODEV;

	/*
	 * Closed for real using wlan_hdd_tsf.c, not just wlan_hdd_tsf.h this
	 * time. hdd_tsf_get_sync() is a NEW function added to wlan_hdd_tsf.c
	 * (not upstream) -- see INTEGRATION_NOTES.md for the exact addition.
	 * It wraps the same capture->wait->indicate sequence already used by
	 * QCA_TSF_SYNC_GET, exposed as a plain callable function instead of
	 * only reachable via the vendor command path.
	 *
	 * Confirmed from wlan_hdd_tsf.c itself: hdd_tsf_check_conn_state()
	 * only actively rejects STA/P2P-client (not associated) and
	 * SAP/P2P-GO (not beaconing) -- QDF_MONITOR_MODE falls through and
	 * is allowed, so this should work on wondertap's monitor adapter.
	 *
	 * Real remaining risk, not a guess: tsf_sync_get_completion_evt
	 * inside wlan_hdd_tsf.c is a single *global* completion, not
	 * per-adapter. A concurrent capture on another adapter (e.g. a STA
	 * doing ordinary TSF-PTP sync at the same time) can race with this
	 * call. No locking added for that -- add it if concurrent use is
	 * realistic on your tree.
	 */
	errno = hdd_tsf_get_sync(adapter, &tsf);
	if (errno)
		return errno;

	*mac_tsf = (u32)tsf;
	return 0;
}

static int hdd_wondertap_channel_schedule_request(void *handle,
				const struct channel_schedule_request *request)
{
	/*
	 * Third and final realistic candidate now checked: TWT (Target Wake
	 * Time). Definitively RULED OUT, and more cleanly than the other
	 * two -- TWT has nothing to do with channels at all. It's a
	 * single-channel wake/sleep schedule negotiated between a STA and
	 * its already-associated AP for power saving (wma_twt.c,
	 * wlan_mlme_twt_api.c, wlan_hdd_twt.c). The only place "channel"
	 * appears in any of those files is
	 * WMI_HOST_*_STATUS_CHAN_SW_IN_PROGRESS -- TWT commands reporting
	 * that they collided with an unrelated CSA channel switch, not TWT
	 * doing any scheduling of its own.
	 *
	 * So: P2P listen offload, NAN discovery, and TWT -- three real
	 * candidates, three ruled out, each for a different reason (wrong
	 * shape, firmware-internal, or wrong feature entirely). Nothing
	 * across any file reviewed exposes a generic host-programmable
	 * multi-channel TSF-scheduled hop primitive. At this point the more
	 * likely conclusion is that this chip/firmware genuinely has no
	 * native equivalent to Broadcom's wondertap hopping, and the
	 * remaining path is host-side emulation: a kernel hrtimer driving
	 * repeated set_freq() calls timed against hdd_tsf_get_sync() reads.
	 * That has materially worse precision than firmware scheduling and
	 * is a real design task, not a port -- happy to build it if you
	 * want to go that direction, but it's a different kind of work than
	 * everything else in this file.
	 *
	 * Not faking success here -- report unsupported.
	 */
	return -EOPNOTSUPP;
}

static int hdd_wondertap_get_channel_status_report(void *handle,
				struct wondertap_channel_status_report *report)
{
	return -EOPNOTSUPP;
}

static int hdd_wondertap_set_station_info(void *handle,
					   const enum wondertap_station_action action,
					   struct wondertap_station_info *info)
{
	/*
	 * Real, concrete lead this time, from wma_nan_datapath.c:
	 * wma_add_sta_ndi_mode() shows exactly what a "remote peer add" for
	 * a non-infrastructure vdev looks like on qcacld --
	 * wma_create_peer(wma, mac_addr, WMI_PEER_TYPE_NAN_DATA, vdev_id) /
	 * wma_remove_peer(...). That's the right shape (arbitrary MAC,
	 * arbitrary vdev, no association/4-way-handshake involved) --
	 * exactly what a raw AWDL-style peer needs.
	 *
	 * Two real gaps remain, not guesses:
	 *   1. wma_create_peer()/wma_remove_peer() aren't in wma_api.h (the
	 *      public HDD-facing surface) -- they're WMA-internal. HDD code
	 *      can't call them directly. wma_add_sta_ndi_mode() itself is
	 *      only reached via the SME/LIM message-passing path
	 *      (WMA_ADD_STA_RSP etc.), triggered from an SME-layer entry
	 *      point that lives in "sme_nan_datapath.h" -- included by
	 *      wma_nan_datapath.h, but not itself shared.
	 *   2. Even with that entry point, WMI_PEER_TYPE_NAN_DATA may carry
	 *      NDP-specific firmware behavior (keepalive timers, datapath
	 *      teardown semantics, etc.) that isn't appropriate to silently
	 *      reuse for a wondertap peer -- would need a peer type that
	 *      actually fits, not just copy the NAN one because it's the
	 *      only example available.
	 *
	 * Send sme_nan_datapath.h (+ .c, if the request/response structs
	 * aren't all in the header) if you want this taken further.
	 */
	return -EOPNOTSUPP;
}

static int hdd_wondertap_init(void **handle,
			       const struct wondertap_init_params *params)
{
	struct hdd_context *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	int errno;

	if (!hdd_ctx)
		return -ENODEV;

	*handle = hdd_ctx;

	if (!g_wondertap.mon_adapter) {
		errno = wlan_hdd_add_monitor_check(hdd_ctx,
						    &g_wondertap.mon_adapter,
						    "wondertap0", false, 0);
		if (errno) {
			hdd_err("wondertap: failed to create monitor adapter: %d",
				errno);
			return errno;
		}
	}

	/* country_code isn't null-terminated per the wondertap ABI (3 bytes,
	 * last is a padding/reserved byte in some versions) -- hdd_reg_set_country
	 * expects a C string, so make sure it's terminated before use.
	 */
	((char *)params->country_code)[2] = '\0';
	hdd_reg_set_country(hdd_ctx, (char *)params->country_code);

	errno = hdd_wondertap_set_freq(hdd_ctx, &params->channel);
	if (errno)
		hdd_err("wondertap: initial set_freq failed: %d", errno);

	errno = hdd_start_adapter(g_wondertap.mon_adapter);
	if (errno) {
		hdd_err("wondertap: failed to start monitor adapter: %d", errno);
		return errno;
	}

	hdd_wondertap_set_fixed_tx_rate(hdd_ctx, &params->tx_rate);

	g_wondertap.active = true;
	return 0;
}

static void hdd_wondertap_deinit(void *handle,
				  const struct wondertap_deinit_params *params)
{
	struct hdd_context *hdd_ctx = handle;

	g_wondertap.active = false;

	if (params->country_code[0]) {
		((char *)params->country_code)[2] = '\0';
		hdd_reg_set_country(hdd_ctx, (char *)params->country_code);
	}

	/*
	 * Deliberately not tearing down g_wondertap.mon_adapter here, to
	 * mirror Broadcom's approach of leaving the underlying netdev
	 * allocated and just stopping it (dhd_wondertap_ops_deinit() calls
	 * dev_close(), not a full adapter teardown). If your monitor-mode
	 * lifecycle expects an explicit close on every deinit, add it here
	 * -- that's a policy choice, not something to guess at.
	 */
}

static const struct wondertap_ops hdd_wondertap_ops = {
	.init = hdd_wondertap_init,
	.deinit = hdd_wondertap_deinit,
	.set_freq = hdd_wondertap_set_freq,
	.set_filter = hdd_wondertap_set_filter,
	.set_fixed_tx_rate = hdd_wondertap_set_fixed_tx_rate,
	.set_tx_rate_mask = hdd_wondertap_set_tx_rate_mask,
	.get_capabilities = hdd_wondertap_get_capabilities,
	.channel_schedule_request = hdd_wondertap_channel_schedule_request,
	.get_mac_tsf = hdd_wondertap_get_mac_tsf,
	.get_channel_status_report = hdd_wondertap_get_channel_status_report,
	.set_station_info = hdd_wondertap_set_station_info,
};

int hdd_wondertap_register(void)
{
	struct wondertap_aux_dev *adev = &g_wondertap.adev;
	int errno;

	memset(&g_wondertap, 0, sizeof(g_wondertap));

	adev->ver = WONDER_VERSION_3_6_5;
	adev->wonder_ops = &hdd_wondertap_ops;
	adev->adev.name = HDD_WONDERTAP_DEV_NAME;
	adev->adev.id = 0;
	adev->adev.dev.release = hdd_wondertap_release;

	errno = auxiliary_device_init(&adev->adev);
	if (errno) {
		hdd_err("wondertap: auxiliary_device_init failed: %d", errno);
		return errno;
	}

	/* auxiliary_device_add() is a macro that supplies KBUILD_MODNAME
	 * automatically -- the resulting device name will be
	 * "<KBUILD_MODNAME>.wondertap.0", matched (up to the trailing id)
	 * against wonder_aux_id_table[] in the wonder module. See the
	 * accompanying kernel-side patch note.
	 */
	errno = auxiliary_device_add(&adev->adev);
	if (errno) {
		hdd_err("wondertap: auxiliary_device_add failed: %d", errno);
		auxiliary_device_uninit(&adev->adev);
		return errno;
	}

	g_wondertap.registered = true;
	return 0;
}

void hdd_wondertap_unregister(void)
{
	struct wondertap_aux_dev *adev = &g_wondertap.adev;

	if (!g_wondertap.registered)
		return;

	auxiliary_device_delete(&adev->adev);
	auxiliary_device_uninit(&adev->adev);
	g_wondertap.registered = false;
}

#endif /* WONDERTAP */
