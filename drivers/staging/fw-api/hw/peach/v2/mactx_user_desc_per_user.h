/*
 * Copyright (c) 2023-2024 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */


#ifndef _MACTX_USER_DESC_PER_USER_H_
#define _MACTX_USER_DESC_PER_USER_H_

#define NUM_OF_DWORDS_MACTX_USER_DESC_PER_USER 4

struct mactx_user_desc_per_user {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t psdu_length                                             : 24,
                      reserved_0a                                             :  8;
             uint32_t ru_start_index                                          :  8,
                      ru_size                                                 :  4,
                      reserved_1b                                             :  4,
                      ofdma_mu_mimo_enabled                                   :  1,
                      nss                                                     :  3,
                      stream_offset                                           :  3,
                      reserved_1c                                             :  1,
                      mcs                                                     :  4,
                      dcm                                                     :  1,
                      reserved_1d                                             :  3;
             uint32_t fec_type                                                :  1,
                      reserved_2a                                             :  7,
                      user_bf_type                                            :  2,
                      reserved_2b                                             :  6,
                      drop_user_cbf                                           :  1,
                      reserved_2c                                             :  7,
                      ldpc_extra_symbol                                       :  1,
                      force_extra_symbol                                      :  1,
                      reserved_2d                                             :  6;
             uint32_t sw_peer_id                                              : 16,
                      per_user_subband_mask                                   : 16;
#else
             uint32_t reserved_0a                                             :  8,
                      psdu_length                                             : 24;
             uint32_t reserved_1d                                             :  3,
                      dcm                                                     :  1,
                      mcs                                                     :  4,
                      reserved_1c                                             :  1,
                      stream_offset                                           :  3,
                      nss                                                     :  3,
                      ofdma_mu_mimo_enabled                                   :  1,
                      reserved_1b                                             :  4,
                      ru_size                                                 :  4,
                      ru_start_index                                          :  8;
             uint32_t reserved_2d                                             :  6,
                      force_extra_symbol                                      :  1,
                      ldpc_extra_symbol                                       :  1,
                      reserved_2c                                             :  7,
                      drop_user_cbf                                           :  1,
                      reserved_2b                                             :  6,
                      user_bf_type                                            :  2,
                      reserved_2a                                             :  7,
                      fec_type                                                :  1;
             uint32_t per_user_subband_mask                                   : 16,
                      sw_peer_id                                              : 16;
#endif
};

#define MACTX_USER_DESC_PER_USER_PSDU_LENGTH_OFFSET                                 0x00000000
#define MACTX_USER_DESC_PER_USER_PSDU_LENGTH_LSB                                    0
#define MACTX_USER_DESC_PER_USER_PSDU_LENGTH_MSB                                    23
#define MACTX_USER_DESC_PER_USER_PSDU_LENGTH_MASK                                   0x00ffffff

#define MACTX_USER_DESC_PER_USER_RESERVED_0A_OFFSET                                 0x00000000
#define MACTX_USER_DESC_PER_USER_RESERVED_0A_LSB                                    24
#define MACTX_USER_DESC_PER_USER_RESERVED_0A_MSB                                    31
#define MACTX_USER_DESC_PER_USER_RESERVED_0A_MASK                                   0xff000000

#define MACTX_USER_DESC_PER_USER_RU_START_INDEX_OFFSET                              0x00000004
#define MACTX_USER_DESC_PER_USER_RU_START_INDEX_LSB                                 0
#define MACTX_USER_DESC_PER_USER_RU_START_INDEX_MSB                                 7
#define MACTX_USER_DESC_PER_USER_RU_START_INDEX_MASK                                0x000000ff

#define MACTX_USER_DESC_PER_USER_RU_SIZE_OFFSET                                     0x00000004
#define MACTX_USER_DESC_PER_USER_RU_SIZE_LSB                                        8
#define MACTX_USER_DESC_PER_USER_RU_SIZE_MSB                                        11
#define MACTX_USER_DESC_PER_USER_RU_SIZE_MASK                                       0x00000f00

#define MACTX_USER_DESC_PER_USER_RESERVED_1B_OFFSET                                 0x00000004
#define MACTX_USER_DESC_PER_USER_RESERVED_1B_LSB                                    12
#define MACTX_USER_DESC_PER_USER_RESERVED_1B_MSB                                    15
#define MACTX_USER_DESC_PER_USER_RESERVED_1B_MASK                                   0x0000f000

#define MACTX_USER_DESC_PER_USER_OFDMA_MU_MIMO_ENABLED_OFFSET                       0x00000004
#define MACTX_USER_DESC_PER_USER_OFDMA_MU_MIMO_ENABLED_LSB                          16
#define MACTX_USER_DESC_PER_USER_OFDMA_MU_MIMO_ENABLED_MSB                          16
#define MACTX_USER_DESC_PER_USER_OFDMA_MU_MIMO_ENABLED_MASK                         0x00010000

#define MACTX_USER_DESC_PER_USER_NSS_OFFSET                                         0x00000004
#define MACTX_USER_DESC_PER_USER_NSS_LSB                                            17
#define MACTX_USER_DESC_PER_USER_NSS_MSB                                            19
#define MACTX_USER_DESC_PER_USER_NSS_MASK                                           0x000e0000

#define MACTX_USER_DESC_PER_USER_STREAM_OFFSET_OFFSET                               0x00000004
#define MACTX_USER_DESC_PER_USER_STREAM_OFFSET_LSB                                  20
#define MACTX_USER_DESC_PER_USER_STREAM_OFFSET_MSB                                  22
#define MACTX_USER_DESC_PER_USER_STREAM_OFFSET_MASK                                 0x00700000

#define MACTX_USER_DESC_PER_USER_RESERVED_1C_OFFSET                                 0x00000004
#define MACTX_USER_DESC_PER_USER_RESERVED_1C_LSB                                    23
#define MACTX_USER_DESC_PER_USER_RESERVED_1C_MSB                                    23
#define MACTX_USER_DESC_PER_USER_RESERVED_1C_MASK                                   0x00800000

#define MACTX_USER_DESC_PER_USER_MCS_OFFSET                                         0x00000004
#define MACTX_USER_DESC_PER_USER_MCS_LSB                                            24
#define MACTX_USER_DESC_PER_USER_MCS_MSB                                            27
#define MACTX_USER_DESC_PER_USER_MCS_MASK                                           0x0f000000

#define MACTX_USER_DESC_PER_USER_DCM_OFFSET                                         0x00000004
#define MACTX_USER_DESC_PER_USER_DCM_LSB                                            28
#define MACTX_USER_DESC_PER_USER_DCM_MSB                                            28
#define MACTX_USER_DESC_PER_USER_DCM_MASK                                           0x10000000

#define MACTX_USER_DESC_PER_USER_RESERVED_1D_OFFSET                                 0x00000004
#define MACTX_USER_DESC_PER_USER_RESERVED_1D_LSB                                    29
#define MACTX_USER_DESC_PER_USER_RESERVED_1D_MSB                                    31
#define MACTX_USER_DESC_PER_USER_RESERVED_1D_MASK                                   0xe0000000

#define MACTX_USER_DESC_PER_USER_FEC_TYPE_OFFSET                                    0x00000008
#define MACTX_USER_DESC_PER_USER_FEC_TYPE_LSB                                       0
#define MACTX_USER_DESC_PER_USER_FEC_TYPE_MSB                                       0
#define MACTX_USER_DESC_PER_USER_FEC_TYPE_MASK                                      0x00000001

#define MACTX_USER_DESC_PER_USER_RESERVED_2A_OFFSET                                 0x00000008
#define MACTX_USER_DESC_PER_USER_RESERVED_2A_LSB                                    1
#define MACTX_USER_DESC_PER_USER_RESERVED_2A_MSB                                    7
#define MACTX_USER_DESC_PER_USER_RESERVED_2A_MASK                                   0x000000fe

#define MACTX_USER_DESC_PER_USER_USER_BF_TYPE_OFFSET                                0x00000008
#define MACTX_USER_DESC_PER_USER_USER_BF_TYPE_LSB                                   8
#define MACTX_USER_DESC_PER_USER_USER_BF_TYPE_MSB                                   9
#define MACTX_USER_DESC_PER_USER_USER_BF_TYPE_MASK                                  0x00000300

#define MACTX_USER_DESC_PER_USER_RESERVED_2B_OFFSET                                 0x00000008
#define MACTX_USER_DESC_PER_USER_RESERVED_2B_LSB                                    10
#define MACTX_USER_DESC_PER_USER_RESERVED_2B_MSB                                    15
#define MACTX_USER_DESC_PER_USER_RESERVED_2B_MASK                                   0x0000fc00

#define MACTX_USER_DESC_PER_USER_DROP_USER_CBF_OFFSET                               0x00000008
#define MACTX_USER_DESC_PER_USER_DROP_USER_CBF_LSB                                  16
#define MACTX_USER_DESC_PER_USER_DROP_USER_CBF_MSB                                  16
#define MACTX_USER_DESC_PER_USER_DROP_USER_CBF_MASK                                 0x00010000

#define MACTX_USER_DESC_PER_USER_RESERVED_2C_OFFSET                                 0x00000008
#define MACTX_USER_DESC_PER_USER_RESERVED_2C_LSB                                    17
#define MACTX_USER_DESC_PER_USER_RESERVED_2C_MSB                                    23
#define MACTX_USER_DESC_PER_USER_RESERVED_2C_MASK                                   0x00fe0000

#define MACTX_USER_DESC_PER_USER_LDPC_EXTRA_SYMBOL_OFFSET                           0x00000008
#define MACTX_USER_DESC_PER_USER_LDPC_EXTRA_SYMBOL_LSB                              24
#define MACTX_USER_DESC_PER_USER_LDPC_EXTRA_SYMBOL_MSB                              24
#define MACTX_USER_DESC_PER_USER_LDPC_EXTRA_SYMBOL_MASK                             0x01000000

#define MACTX_USER_DESC_PER_USER_FORCE_EXTRA_SYMBOL_OFFSET                          0x00000008
#define MACTX_USER_DESC_PER_USER_FORCE_EXTRA_SYMBOL_LSB                             25
#define MACTX_USER_DESC_PER_USER_FORCE_EXTRA_SYMBOL_MSB                             25
#define MACTX_USER_DESC_PER_USER_FORCE_EXTRA_SYMBOL_MASK                            0x02000000

#define MACTX_USER_DESC_PER_USER_RESERVED_2D_OFFSET                                 0x00000008
#define MACTX_USER_DESC_PER_USER_RESERVED_2D_LSB                                    26
#define MACTX_USER_DESC_PER_USER_RESERVED_2D_MSB                                    31
#define MACTX_USER_DESC_PER_USER_RESERVED_2D_MASK                                   0xfc000000

#define MACTX_USER_DESC_PER_USER_SW_PEER_ID_OFFSET                                  0x0000000c
#define MACTX_USER_DESC_PER_USER_SW_PEER_ID_LSB                                     0
#define MACTX_USER_DESC_PER_USER_SW_PEER_ID_MSB                                     15
#define MACTX_USER_DESC_PER_USER_SW_PEER_ID_MASK                                    0x0000ffff

#define MACTX_USER_DESC_PER_USER_PER_USER_SUBBAND_MASK_OFFSET                       0x0000000c
#define MACTX_USER_DESC_PER_USER_PER_USER_SUBBAND_MASK_LSB                          16
#define MACTX_USER_DESC_PER_USER_PER_USER_SUBBAND_MASK_MSB                          31
#define MACTX_USER_DESC_PER_USER_PER_USER_SUBBAND_MASK_MASK                         0xffff0000

#endif
