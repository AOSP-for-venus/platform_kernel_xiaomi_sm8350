/*
 * wlan_hdd_wondertap.c
 *
 * qcacld-3.0's vendor-side implementation of the "wondertap" interface
 * consumed by Google's "wonder" WLAN virtual soft-MAC driver, which
 * provides the AWDL-style transport backing AirDrop-compatible Quick
 * Share.
 *
 * ARCHITECTURE UPDATE: this now targets the Pixel gs-6.6 generation of
 * the wonder module (per 6d3db991e7c32b8a048beb4c7c45edee04a393cb.patch),
 * NOT the GKI-common auxiliary_bus generation this file originally
 * targeted. That's a real, load-bearing difference, not a cosmetic one:
 *
 *   - Binding is now platform_device + Linux component framework +
 *     device-tree, matching exactly how bcm4390_dhd_custom_google.c's
 *     dhd_wonder_driver/dhd_wonder_probe/dhd_wifi_comp_ops already do it
 *     (that sample turned out to be a *current, accurate* reference for
 *     this generation, not a legacy one -- WONDER_VERSION_1_4 is in the
 *     same version lineage as this generation's WONDER_VERSION_1_6_2,
 *     confirmed compatible via wonder's own wonder_ver_match_table).
 *   - No more auxiliary_device / KBUILD_MODNAME string matching, and so
 *     no more kernel-side wonder_aux_id_table[] patch needed at all --
 *     matching is now via a device-tree compatible string instead. See
 *     the new qcacld-wonder.dtsi this port now ships alongside this file.
 *   - struct wondertap_aux_dev -> struct wondertap_priv (no embedded
 *     auxiliary_device; just .ver and .wonder_ops, and it's a plain
 *     static const struct now, not something you init/add at runtime).
 *   - The interface itself shrank: get_channel_status_report and
 *     set_station_info (plus their backing structs) no longer exist on
 *     struct wondertap_ops in this generation. Removed below rather than
 *     stubbed -- there's nothing to stub, the struct fields are gone.
 *
 * qcacld-3.0 still has no standing "monitor_dev" the way dhd does, so a
 * monitor-mode adapter is still created on demand via
 * wlan_hdd_add_monitor_check() -- that part is unaffected by any of the
 * above and carries over unchanged, along with set_freq, set_fixed_tx_rate,
 * set_tx_rate_mask, get_mac_tsf, and channel_schedule_request's analysis.
 *
 * See INTEGRATION_NOTES.md for the current per-op table and
 * qcacld-wonder.dtsi + INTEGRATION_NOTES.md's DT section for the two
 * device-tree nodes this now requires.
 */

#ifdef WONDERTAP

#include <linux/platform_device.h>
#include <linux/component.h>
#include <linux/of.h>
#include <linux/version.h>
#include <wonder/wondertap.h>
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

/*
 * Must match the "compatible" string in the wlan-device node of
 * qcacld-wonder.dtsi. Chosen by analogy with Broadcom's
 * "android,bcmdhd_wlan-wonder" -- there's no fixed convention imposed by
 * the wonder module for this string (unlike "google,wonder-drv-v1", which
 * IS fixed and owned by wonder's own of_match_table), so this is a real
 * choice, not a discovered constant. Keep it in sync with the .dtsi if
 * you rename it.
 */
#define HDD_WONDERTAP_DT_COMPATIBLE "android,qcacld_wlan-wonder"

/*
 * Runtime state this file still owns (the monitor adapter, active flag).
 * Separate from the wondertap_priv sent to wonder below, because that one
 * is now a plain static const struct (no embedded device to init/add) --
 * see hdd_wondertap_probe().
 */
struct hdd_wondertap_state {
	struct hdd_adapter *mon_adapter;
	bool active;
};

static struct hdd_wondertap_state g_wondertap;

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
	 * over-advertising will make the generic wonder layer attempt
	 * operations this driver doesn't implement yet.
	 *
	 * CONFIRMED, not speculative, from wonder's own source (main.c /
	 * wondertap_internal.h): wonder_get_channel_status_report() and the
	 * channel-schedule path both explicitly gate on
	 * `wondertap->cap.bits.channel_hopping &&
	 *  wondertap->init_params.channel_hopping_enable` before ever
	 * calling channel_schedule_request(). So even once that op has a
	 * real implementation, it will never be invoked unless
	 * .channel_hopping is set to 1 here. Left at 0 for now because the
	 * op itself still isn't implemented -- flip this the same day you
	 * make channel_schedule_request actually work, not before.
	 */
	caps->bits.dynamic_freq = 1;
	caps->bits.dynamic_fixed_tx_rate = 1;
	caps->bits.channel_hopping = 0; /* see note above -- flip together with channel_schedule_request */
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
	 * multi-channel TSF-scheduled hop primitive on the QCA side.
	 *
	 * What request-> actually contains is now fully confirmed, though --
	 * from your own wonder module's deleted tools/gen_hop_cmd.py and
	 * tools/parse_channel_report.py (visible in the Pixel gs-6.6 diff):
	 *   request->channel_list_len       -- number of entries below
	 *   request->next_channel_index     -- which entry to start from
	 *   request->dwell_time_tu          -- time per slot, in TUs (1024us)
	 *   request->target_switch_time_tsf -- TSF value to start the schedule at
	 *   request->channel_list[]         -- per-slot {freq, bw, role}, where
	 *                                      freq==0 appears to mean "off /
	 *                                      revert to normal operation" for
	 *                                      that slot (a duty-cycle pattern
	 *                                      across a fixed-size slot list,
	 *                                      not a simple channel sequence)
	 *
	 * That closes "what does this function receive" for good. It does
	 * NOT close "how do I make QCA firmware actually do this" -- that's
	 * still the open, unresolved question, and nothing found so far
	 * answers it.
	 *
	 * IMPORTANT, also confirmed from the same source (not previously
	 * known): wonder itself gates on
	 * `wondertap->cap.bits.channel_hopping &&
	 *  wondertap->init_params.channel_hopping_enable` before ever
	 * calling this op. hdd_wondertap_get_capabilities() above
	 * deliberately leaves channel_hopping at 0 until this function is
	 * real -- flip both together.
	 *
	 * Two ways forward from here, genuinely different in kind:
	 *   1. Firmware-native: would need an actual QCA WMI command this
	 *      port hasn't found in any file reviewed. At this point that
	 *      likely means it doesn't exist in the public/shared parts of
	 *      this codebase, if it exists at all on this chip.
	 *   2. Host-side emulation: a kernel timer (hrtimer or delayed_work)
	 *      that walks request->channel_list[] and calls
	 *      hdd_wondertap_set_freq() at each dwell_time_tu boundary,
	 *      phased against hdd_tsf_get_sync() to land close to
	 *      target_switch_time_tsf. This is buildable with what's already
	 *      in this file. Be aware going in: host-timer jitter (scheduler
	 *      latency, interrupt load) is realistically millisecond-scale,
	 *      while firmware-native hopping for a protocol like this is
	 *      typically expected to hold much tighter (closer to
	 *      microsecond-scale) synchronization with the peer. This may
	 *      simply not be tight enough for the far end to reliably
	 *      rendezvous with your device on-channel -- worth trying and
	 *      measuring rather than assuming either way.
	 *
	 * Say the word on (2) and I'll build it -- it's a different kind of
	 * task than the rest of this file (real-time scheduling logic, not
	 * API plumbing), so I'd rather you decide deliberately than have it
	 * bundled into "yes, continue."
	 */
	return -EOPNOTSUPP;
}

/*
 * get_channel_status_report and set_station_info (and their backing
 * structs: wondertap_channel_status_report, wondertap_channel_status,
 * wondertap_station_info, wondertap_station_action) no longer exist on
 * struct wondertap_ops in this generation of wondertap.h -- confirmed
 * removed in the Pixel gs-6.6 diff, not just unimplemented. There is
 * nothing to stub; the struct fields are gone, so designated initializers
 * for them below would be a compile error. The wma_create_peer() /
 * sme_nan_datapath.h investigation from the previous round is moot for
 * this generation -- noted here so that history isn't lost, not because
 * it's still actionable.
 */

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
};

/*
 * Static const, not a per-probe allocation -- matches
 * bcm4390_dhd_custom_google.c's dhd_wonder_priv exactly in shape.
 * WONDER_VERSION_1_4 (same value Broadcom's reference uses) is confirmed
 * accepted by this generation's wonder_ver_match_table for the
 * WONDER_VERSION_1_6_2 master, so there's no reason to invent a different
 * version number here.
 */
static const struct wondertap_priv hdd_wondertap_priv = {
	.ver = WONDER_VERSION_1_4,
	.wonder_ops = &hdd_wondertap_ops,
};

/*
 * Trivial on purpose -- mirrors dhd_wondertap_bind()/_unbind() in
 * bcm4390_dhd_custom_google.c. All the real work (reading our drvdata,
 * pulling out .ver/.wonder_ops, calling get_capabilities) happens on
 * wonder's own master-side bind function, not here; this side of the
 * component pair just needs to exist and report success so
 * component_bind_all() in wonder's probe doesn't stall waiting on it.
 */
static int hdd_wondertap_bind(struct device *dev, struct device *master,
			       void *data)
{
	dev_info(dev, "wondertap: bound to master %s\n", dev_name(master));
	return 0;
}

static void hdd_wondertap_unbind(struct device *dev, struct device *master,
				  void *data)
{
	dev_info(dev, "wondertap: unbound from master %s\n", dev_name(master));
}

static const struct component_ops hdd_wondertap_comp_ops = {
	.bind = hdd_wondertap_bind,
	.unbind = hdd_wondertap_unbind,
};

static int hdd_wondertap_probe(struct platform_device *pdev)
{
	dev_info(&pdev->dev, "wondertap: probe\n");

	/*
	 * Same pattern as dhd_wonder_probe(): stash our static priv as
	 * drvdata (wonder's master_bind does
	 * platform_get_drvdata(of_find_device_by_node(...)) to retrieve it),
	 * then register as a component so wonder's component_master_add
	 * match (by of_node, against the phandle in its own DT node) can
	 * find us.
	 */
	platform_set_drvdata(pdev, (void *)&hdd_wondertap_priv);
	return component_add(&pdev->dev, &hdd_wondertap_comp_ops);
}

static void hdd_wondertap_remove(struct platform_device *pdev)
{
	dev_info(&pdev->dev, "wondertap: remove\n");
	component_del(&pdev->dev, &hdd_wondertap_comp_ops);
}

/*
 * platform_driver.remove's return type changed (void, no longer int) in
 * upstream commit around 6.11 -- same compatibility shim
 * bcm4390_dhd_custom_google.c uses, needed because your kernel (5.4) is
 * well below that line.
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 11, 0)
static int hdd_wondertap_remove_wrapper(struct platform_device *pdev)
{
	hdd_wondertap_remove(pdev);
	return 0;
}
#define hdd_wondertap_remove hdd_wondertap_remove_wrapper
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(6, 11, 0) */

static const struct of_device_id hdd_wondertap_dt_ids[] = {
	{ .compatible = HDD_WONDERTAP_DT_COMPATIBLE },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, hdd_wondertap_dt_ids);

static struct platform_driver hdd_wondertap_driver = {
	.probe = hdd_wondertap_probe,
	.remove = hdd_wondertap_remove,
	.driver = {
		.name = "qcacld_wonder_dev",
		.of_match_table = hdd_wondertap_dt_ids,
	},
};

int hdd_wondertap_register(void)
{
	memset(&g_wondertap, 0, sizeof(g_wondertap));

	/*
	 * Unlike the old auxiliary_device_init()/_add() pair, there's no
	 * device to construct here -- the device comes from the devicetree
	 * node (qcacld-wonder.dtsi's wlan-device) once it's populated by the
	 * kernel's standard of_platform_default_populate() path. Registering
	 * the driver just makes it available to match against that node
	 * when/if it exists. If the DT node isn't present or isn't populated,
	 * this call still succeeds -- probe() simply never fires, and you
	 * won't see a "wondertap: probe" log line. That's the first thing to
	 * check if this doesn't work, same as before.
	 */
	return platform_driver_register(&hdd_wondertap_driver);
}

void hdd_wondertap_unregister(void)
{
	platform_driver_unregister(&hdd_wondertap_driver);
}

#endif /* WONDERTAP */
