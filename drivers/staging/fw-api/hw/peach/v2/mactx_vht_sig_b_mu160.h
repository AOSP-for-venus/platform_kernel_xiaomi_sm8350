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


#ifndef _MACTX_VHT_SIG_B_MU160_H_
#define _MACTX_VHT_SIG_B_MU160_H_

#include "vht_sig_b_mu160_info.h"
#define NUM_OF_DWORDS_MACTX_VHT_SIG_B_MU160 8

struct mactx_vht_sig_b_mu160 {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             struct   vht_sig_b_mu160_info                                      mactx_vht_sig_b_mu160_info_details;
#else
             struct   vht_sig_b_mu160_info                                      mactx_vht_sig_b_mu160_info_details;
#endif
};

#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_LENGTH_OFFSET      0x00000000
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_LENGTH_LSB         0
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_LENGTH_MSB         18
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_LENGTH_MASK        0x0007ffff

#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_MCS_OFFSET         0x00000000
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_MCS_LSB            19
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_MCS_MSB            22
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_MCS_MASK           0x00780000

#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_TAIL_OFFSET        0x00000000
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_TAIL_LSB           23
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_TAIL_MSB           28
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_TAIL_MASK          0x1f800000

#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_RESERVED_0_OFFSET  0x00000000
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_RESERVED_0_LSB     29
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_RESERVED_0_MSB     31
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_RESERVED_0_MASK    0xe0000000

#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_LENGTH_COPY_A_OFFSET 0x00000004
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_LENGTH_COPY_A_LSB  0
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_LENGTH_COPY_A_MSB  18
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_LENGTH_COPY_A_MASK 0x0007ffff

#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_MCS_COPY_A_OFFSET  0x00000004
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_MCS_COPY_A_LSB     19
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_MCS_COPY_A_MSB     22
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_MCS_COPY_A_MASK    0x00780000

#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_TAIL_COPY_A_OFFSET 0x00000004
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_TAIL_COPY_A_LSB    23
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_TAIL_COPY_A_MSB    28
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_TAIL_COPY_A_MASK   0x1f800000

#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_RESERVED_1_OFFSET  0x00000004
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_RESERVED_1_LSB     29
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_RESERVED_1_MSB     31
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_RESERVED_1_MASK    0xe0000000

#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_LENGTH_COPY_B_OFFSET 0x00000008
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_LENGTH_COPY_B_LSB  0
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_LENGTH_COPY_B_MSB  18
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_LENGTH_COPY_B_MASK 0x0007ffff

#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_MCS_COPY_B_OFFSET  0x00000008
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_MCS_COPY_B_LSB     19
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_MCS_COPY_B_MSB     22
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_MCS_COPY_B_MASK    0x00780000

#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_TAIL_COPY_B_OFFSET 0x00000008
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_TAIL_COPY_B_LSB    23
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_TAIL_COPY_B_MSB    28
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_TAIL_COPY_B_MASK   0x1f800000

#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_RESERVED_2_OFFSET  0x00000008
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_RESERVED_2_LSB     29
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_RESERVED_2_MSB     31
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_RESERVED_2_MASK    0xe0000000

#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_LENGTH_COPY_C_OFFSET 0x0000000c
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_LENGTH_COPY_C_LSB  0
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_LENGTH_COPY_C_MSB  18
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_LENGTH_COPY_C_MASK 0x0007ffff

#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_MCS_COPY_C_OFFSET  0x0000000c
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_MCS_COPY_C_LSB     19
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_MCS_COPY_C_MSB     22
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_MCS_COPY_C_MASK    0x00780000

#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_TAIL_COPY_C_OFFSET 0x0000000c
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_TAIL_COPY_C_LSB    23
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_TAIL_COPY_C_MSB    28
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_TAIL_COPY_C_MASK   0x1f800000

#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_RESERVED_3_OFFSET  0x0000000c
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_RESERVED_3_LSB     29
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_RESERVED_3_MSB     31
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_RESERVED_3_MASK    0xe0000000

#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_LENGTH_COPY_D_OFFSET 0x00000010
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_LENGTH_COPY_D_LSB  0
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_LENGTH_COPY_D_MSB  18
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_LENGTH_COPY_D_MASK 0x0007ffff

#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_MCS_COPY_D_OFFSET  0x00000010
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_MCS_COPY_D_LSB     19
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_MCS_COPY_D_MSB     22
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_MCS_COPY_D_MASK    0x00780000

#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_TAIL_COPY_D_OFFSET 0x00000010
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_TAIL_COPY_D_LSB    23
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_TAIL_COPY_D_MSB    28
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_TAIL_COPY_D_MASK   0x1f800000

#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_RESERVED_4_OFFSET  0x00000010
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_RESERVED_4_LSB     29
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_RESERVED_4_MSB     31
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_RESERVED_4_MASK    0xe0000000

#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_LENGTH_COPY_E_OFFSET 0x00000014
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_LENGTH_COPY_E_LSB  0
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_LENGTH_COPY_E_MSB  18
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_LENGTH_COPY_E_MASK 0x0007ffff

#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_MCS_COPY_E_OFFSET  0x00000014
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_MCS_COPY_E_LSB     19
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_MCS_COPY_E_MSB     22
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_MCS_COPY_E_MASK    0x00780000

#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_TAIL_COPY_E_OFFSET 0x00000014
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_TAIL_COPY_E_LSB    23
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_TAIL_COPY_E_MSB    28
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_TAIL_COPY_E_MASK   0x1f800000

#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_RESERVED_5_OFFSET  0x00000014
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_RESERVED_5_LSB     29
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_RESERVED_5_MSB     31
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_RESERVED_5_MASK    0xe0000000

#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_LENGTH_COPY_F_OFFSET 0x00000018
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_LENGTH_COPY_F_LSB  0
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_LENGTH_COPY_F_MSB  18
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_LENGTH_COPY_F_MASK 0x0007ffff

#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_MCS_COPY_F_OFFSET  0x00000018
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_MCS_COPY_F_LSB     19
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_MCS_COPY_F_MSB     22
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_MCS_COPY_F_MASK    0x00780000

#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_TAIL_COPY_F_OFFSET 0x00000018
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_TAIL_COPY_F_LSB    23
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_TAIL_COPY_F_MSB    28
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_TAIL_COPY_F_MASK   0x1f800000

#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_MU_USER_NUMBER_OFFSET 0x00000018
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_MU_USER_NUMBER_LSB 29
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_MU_USER_NUMBER_MSB 31
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_MU_USER_NUMBER_MASK 0xe0000000

#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_LENGTH_COPY_G_OFFSET 0x0000001c
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_LENGTH_COPY_G_LSB  0
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_LENGTH_COPY_G_MSB  18
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_LENGTH_COPY_G_MASK 0x0007ffff

#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_MCS_COPY_G_OFFSET  0x0000001c
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_MCS_COPY_G_LSB     19
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_MCS_COPY_G_MSB     22
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_MCS_COPY_G_MASK    0x00780000

#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_TAIL_COPY_G_OFFSET 0x0000001c
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_TAIL_COPY_G_LSB    23
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_TAIL_COPY_G_MSB    28
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_TAIL_COPY_G_MASK   0x1f800000

#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_RESERVED_7_OFFSET  0x0000001c
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_RESERVED_7_LSB     29
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_RESERVED_7_MSB     31
#define MACTX_VHT_SIG_B_MU160_MACTX_VHT_SIG_B_MU160_INFO_DETAILS_RESERVED_7_MASK    0xe0000000

#endif
