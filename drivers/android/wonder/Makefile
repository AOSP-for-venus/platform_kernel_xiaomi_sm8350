# SPDX-License-Identifier: GPL-2.0-only

ccflags-y := -DDYNAMIC_DEBUG_MODULE

obj-$(CONFIG_ANDROID_WONDER) += wonder.o
wonder-objs := \
	main.o			\
	mac80211.o		\
	wondertap.o		\
	nl80211_ven_cmd.o	\
	mac80211_txs.o		\
	ssr.o			\
	band_config.o

wonder-$(CONFIG_DEBUG_FS) += debugfs.o
