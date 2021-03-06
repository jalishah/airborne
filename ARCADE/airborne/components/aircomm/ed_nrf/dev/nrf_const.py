# -*- coding: utf-8 -*-
"""
	a list of NRF-registers and commands
"""
__author__	= """Alexander Krause <alexander.krause@ed-solutions.de>"""
__date__ 		= "2012-04-30"
__version__	= "0.1.0"

NRF_C_R_REGISTER					=0x00
NRF_C_W_REGISTER					=0x20
NRF_C_R_RX_PAYLOAD				=0x61
NRF_C_W_TX_PAYLOAD				=0xA0
NRF_C_W_TX_PAYLOAD_NOACK	=0xB0
NRF_C_FLUSH_TX						=0xE1
NRF_C_FLUSH_RX						=0xE2
NRF_C_REUSE_TX_PL					=0xE3
NRF_C_R_RX_PL_WID					=0x60
NRF_C_W_ACK_PAYLOAD				=0xA8
NRF_C_NOP									=0xFF

#Registers
NRF_R_CONFIG							=0x00
NRF_R_EN_AA								=0x01
NRF_R_EN_RXADDR						=0x02
NRF_R_SETUP_AW						=0x03
NRF_R_SETUP_RETR					=0x04
NRF_R_RF_CH								=0x05
NRF_R_RF_SETUP						=0x06
NRF_R_STATUS							=0x07
NRF_R_OBSERVE_TX					=0x08
NRF_R_RPD									=0x09
NRF_R_FEATURE							=0x1D
def	NRF_R_RX_ADDR_P(x):
	return (0x0A + (x))
def NRF_R_RX_ADDR_P_LEN(x, y)	:
	return (y if (x < 2) else 1)
NRF_R_TX_ADDR							=0x10
NRF_R_TX_ADDR_LEN					=0x05
def NRF_R_RX_PW_P(x):
	return (0x11 + (x))
NRF_R_FIFO_STATUS					=0x17
NRF_R_DYNPD								=0x1C

#NRF_R_CONFIG
NRF_R_CONFIG_RESERVED			=0x80
NRF_R_CONFIG_MASK_RX_DR		=0x40
NRF_R_CONFIG_MASK_TX_DS		=0x20
NRF_R_CONFIG_MASK_MAX_RT	=0x10
NRF_R_CONFIG_EN_CRC				=0x08
NRF_R_CONFIG_CRCO					=0x04
NRF_R_CONFIG_PWR_UP				=0x02
NRF_R_CONFIG_PRIM_RX			=0x01

#NRF_R_EN_AA
def NRF_R_EN_AA_ENAA_P(x):
	return (1 << (x))
NRF_R_EN_AA_ENAA_NONE			=0x00

#NRF_R_EN_RXADDR
def NRF_R_EN_RXADDR_ERX_P(x):
	return (1 << (x))
NRF_R_EN_RXADDR_ERX_NONE	=0x00

#NRF_R_RF_CH
NRF_R_RF_CH_BITS					=0x7f

#NRF_R_RF_SETUP
NRF_R_RF_CONT_WAVE				=0x80
NRF_R_RF_SETUP_RF_DR_LOW	=0x20
NRF_R_RF_SETUP_PLL_LOCK		=0x10
NRF_R_RF_SETUP_RF_DR_HIGH	=0x08
def NRF_R_RF_SETUP_RF_PWR(x):
	return (((x) & 0x3) << 1)
NRF_R_RF_SETUP_DR_1M			=0x00
NRF_R_RF_SETUP_DR_2M			=0x08
NRF_R_RF_SETUP_DR_250K		=0x20

#NRF_R_SETUP_AW
def NRF_R_SETUP_AW_AW(x):
	return ((x - 2) & 0x3)

#NRF_R_STATUS
NRF_R_STATUS_RX_DR					=0x40
NRF_R_STATUS_TX_DS					=0x20
NRF_R_STATUS_MAX_RT					=0x10
NRF_R_STATUS_RX_P_NO				=0x0E
def NRF_R_STATUS_GET_RX_P_NO(x)	:
	return (((x) & R_STATUS_RX_P_NO) >> 1)
NRF_R_STATUS_RX_FIFO_EMPTY	=0x0E
NRF_R_STATUS_TX_FULL				=0x01

#NRF_R_SETUP_RETR
def NRF_R_SETUP_RETR_ARD(x)	:
	return (((x) & 0x0f) << 4)
def NRF_R_SETUP_RETR_ARC(x)	:
	return ((x) & 0x0f)

#NRF_R_FEATURE
NRF_R_FEATURE_EN_DPL				=0x04
NRF_R_FEATURE_EN_ACK_PAY		=0x02
NRF_R_FEATURE_EN_DYN_ACK		=0x01

#NRF_R_DYNPD
def NRF_R_DYNPD_DPL_P(x):
	return (0x1 << (x))
NRF_R_DYNPD_DPL_NONE				=0x00

#NRF_R_FIFO_STATUS
NRF_R_FIFO_STATUS_RX_EMPTY 	=0x1
NRF_R_FIFO_STATUS_RX_FULL		=0x2
NRF_R_FIFO_STATUS_TX_EMPTY	=0x10
NRF_R_FIFO_STATUS_TX_FULL		=0x20
NRF_R_FIFO_STATUS_TX_REUSE	=0x40

#NRF_R_RF_CH
def NRF_R_RF_CH_RF_CH(v):
	return ((v) & 0x7f)

#delays
NRF_T_SND_DELAY							=10
NRF_T_PWR_UP								=150		#External Clock

REGISTER_NAMES={
	NRF_R_SETUP_AW		: 'NRF_R_SETUP_AW',
	NRF_R_RF_CH				: 'NRF_R_RF_CH',
	NRF_R_RF_SETUP		: 'NRF_R_RF_SETUP',
	NRF_R_SETUP_RETR	: 'NRF_R_SETUP_RETR',
	NRF_R_CONFIG			: 'NRF_R_CONFIG',
	NRF_R_FEATURE			: 'NRF_R_FEATURE',
	NRF_R_EN_RXADDR		: 'NRF_R_EN_RXADDR',
	NRF_R_EN_AA				: 'NRF_R_EN_AA',
	NRF_R_DYNPD				: 'NRF_R_DYNPD',
	NRF_R_STATUS			: 'NRF_R_STATUS',
	NRF_R_RX_PW_P(0)	: 'NRF_R_RX_PW_P(0)',
	NRF_R_RX_PW_P(1)	: 'NRF_R_RX_PW_P(1)',
	NRF_R_RX_PW_P(2)	: 'NRF_R_RX_PW_P(2)',
	NRF_R_RX_PW_P(3)	: 'NRF_R_RX_PW_P(3)',
	NRF_R_RX_PW_P(4)	: 'NRF_R_RX_PW_P(4)',
	NRF_R_EN_RXADDR		: 'NRF_R_EN_RXADDR',
}