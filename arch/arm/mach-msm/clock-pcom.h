/* Copyright (c) 2009, Code Aurora Forum. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Code Aurora Forum nor
 *       the names of its contributors may be used to endorse or promote
 *       products derived from this software without specific prior written
 *       permission.
 *
 * Alternatively, provided that this notice is retained in full, this software
 * may be relicensed by the recipient under the terms of the GNU General Public
 * License version 2 ("GPL") and only version 2, in which case the provisions of
 * the GPL apply INSTEAD OF those given above.  If the recipient relicenses the
 * software under the GPL, then the identification text in the MODULE_LICENSE
 * macro must be changed to reflect "GPLv2" instead of "Dual BSD/GPL".  Once a
 * recipient changes the license terms to the GPL, subsequent recipients shall
 * not relicense under alternate licensing terms, including the BSD or dual
 * BSD/GPL terms.  In addition, the following license statement immediately
 * below and between the words START and END shall also then apply when this
 * software is relicensed under the GPL:
 *
 * START
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 2 and only version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * END
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef __ARCH_ARM_MACH_MSM_CLOCK_PCOM_H
#define __ARCH_ARM_MACH_MSM_CLOCK_PCOM_H

/* clock IDs used by the modem processor */

#define P_ACPU_CLK	0   /* Applications processor clock */
#define P_ADM_CLK	1   /* Applications data mover clock */
#define P_ADSP_CLK	2   /* ADSP clock */
#define P_EBI1_CLK	3   /* External bus interface 1 clock */
#define P_EBI2_CLK	4   /* External bus interface 2 clock */
#define P_ECODEC_CLK	5   /* External CODEC clock */
#define P_EMDH_CLK	6   /* External MDDI host clock */
#define P_GP_CLK	7   /* General purpose clock */
#define P_GRP_CLK	8   /* Graphics clock */
#define P_I2C_CLK	9   /* I2C clock */
#define P_ICODEC_RX_CLK	10  /* Internal CODEX RX clock */
#define P_ICODEC_TX_CLK	11  /* Internal CODEX TX clock */
#define P_IMEM_CLK	12  /* Internal graphics memory clock */
#define P_MDC_CLK	13  /* MDDI client clock */
#define P_MDP_CLK	14  /* Mobile display processor clock */
#define P_PBUS_CLK	15  /* Peripheral bus clock */
#define P_PCM_CLK	16  /* PCM clock */
#define P_PMDH_CLK	17  /* Primary MDDI host clock */
#define P_SDAC_CLK	18  /* Stereo DAC clock */
#define P_SDC1_CLK	19  /* Secure Digital Card clocks */
#define P_SDC1_PCLK	20
#define P_SDC2_CLK	21
#define P_SDC2_PCLK	22
#define P_SDC3_CLK	23
#define P_SDC3_PCLK	24
#define P_SDC4_CLK	25
#define P_SDC4_PCLK	26
#define P_TSIF_CLK	27  /* Transport Stream Interface clocks */
#define P_TSIF_REF_CLK	28
#define P_TV_DAC_CLK	29  /* TV clocks */
#define P_TV_ENC_CLK	30
#define P_UART1_CLK	31  /* UART clocks */
#define P_UART2_CLK	32
#define P_UART3_CLK	33
#define P_UART1DM_CLK	34
#define P_UART2DM_CLK	35
#define P_USB_HS_CLK	36  /* High speed USB core clock */
#define P_USB_HS_PCLK	37  /* High speed USB pbus clock */
#define P_USB_OTG_CLK	38  /* Full speed USB clock */
#define P_VDC_CLK	39  /* Video controller clock */
#define P_VFE_MDC_CLK	40  /* Camera / Video Front End clock */
#define P_VFE_CLK	41  /* VFE MDDI client clock */
#define P_MDP_LCDC_PCLK_CLK	42
#define P_MDP_LCDC_PAD_PCLK_CLK 43
#define P_MDP_VSYNC_CLK	44
#define P_SPI_CLK	45
#define P_VFE_AXI_CLK	46
#define P_USB_HS2_CLK	47  /* High speed USB 2 core clock */
#define P_USB_HS2_PCLK	48  /* High speed USB 2 pbus clock */
#define P_USB_HS3_CLK	49  /* High speed USB 3 core clock */
#define P_USB_HS3_PCLK	50  /* High speed USB 3 pbus clock */
#define P_GRP_PCLK	51  /* Graphics pbus clock */
#define P_USB_PHY_CLK	52  /* USB PHY clock */
#define P_USB_HS_CORE_CLK	53  /* High speed USB 1 core clock */
#define P_USB_HS2_CORE_CLK	54  /* High speed USB 2 core clock */
#define P_USB_HS3_CORE_CLK	55  /* High speed USB 3 core clock */
#define P_CAM_MCLK_CLK		56
#define P_CAMIF_PAD_PCLK	57
#define P_GRP_2D_CLK		58
#define P_GRP_2D_PCLK		59
#define P_I2S_CLK		60
#define P_JPEG_CLK		61
#define P_JPEG_PCLK		62
#define P_LPA_CODEC_CLK		63
#define P_LPA_CORE_CLK		64
#define P_LPA_PCLK		65
#define P_MDC_IO_CLK		66
#define P_MDC_PCLK		67
#define P_MFC_CLK		68
#define P_MFC_DIV2_CLK		69
#define P_MFC_PCLK		70
#define P_QUP_I2C_CLK		71
#define P_ROTATOR_IMEM_CLK	72
#define P_ROTATOR_PCLK		73
#define P_VFE_CAMIF_CLK		74
#define P_VFE_PCLK		75
#define P_VPE_CLK		76
#define P_I2C_2_CLK		77
#define P_MI2S_CODEC_RX_SCLK	78
#define P_MI2S_CODEC_RX_MCLK	79
#define P_MI2S_CODEC_TX_SCLK	80
#define P_MI2S_CODEC_TX_MCLK	81
#define P_PMDH_PCLK		82
#define P_EMDH_PCLK		83
#define P_SPI_PCLK		84
#define P_TSIF_PCLK		85
#define P_MDP_PCLK		86
#define P_SDAC_MCLK		87
#define P_MI2S_HDMI_CLK		88
#define P_MI2S_HDMI_MCLK	89

#define P_NR_CLKS		90

struct clk_ops;
extern struct clk_ops clk_ops_pcom;

#define CLK_PCOM(clk_name, clk_id, clk_dev, clk_flags) {	\
	.name = clk_name, \
	.id = P_##clk_id, \
	.ops = &clk_ops_pcom, \
	.flags = clk_flags, \
	.dev = clk_dev, \
	.dbg_name = #clk_id, \
	}

#endif
