/******************************************************************************
*
* Copyright (c) 2010 TP-LINK Technologies CO.,LTD.
* All rights reserved.
*
* FILE NAME  :   wlan_twin_ip.h
* VERSION    :   1.0
* DESCRIPTION:   处理从ath0收到的报文的相关操作，为韩国Twin IP功能服务.
*
* AUTHOR     :   huangwenzhong <huangwenzhong@tp-link.net>
* CREATE DATE:   08/24/2012
*
* HISTORY    :
* 01   08/24/2012  huangwenzhong     Create.
*
******************************************************************************/
#ifndef TP_TWIN_IP_H
#define TP_TWIN_IP_H

//#define TP_TWIN_IP_MODULE 1


#define	TP_TWIN_IP_DIR_NAME				"tp_twin_ip"
#define	TP_TWIN_IP_ENABLE_FILE_NAME		"tp_twin_ip_enable"
#define	TP_TWIN_IP_MAC_FILE_NAME		"tp_twin_ip_mac"
#define TP_TWIN_IP_WAN_IP_FILE_NAME		"tp_twin_ip_wan_ip"
#define TP_TWIN_IP_VIRTUAL_IP_FILE_NAME	"tp_twin_ip_vir_ip"
#define TP_MAC_STR_SIZE					(18)
#define	TP_MAC_ADDR_SIZE				(6)

struct twin_ip_cfg
{
	int wlan_twin_ip_enable;
	unsigned int wlan_twin_wan_ip;
	unsigned int wlan_twin_virtual_ip;
};

int tp_is_twin_ip_enable(void);
unsigned int tp_get_twin_ip_wan_ip(void);
unsigned int tp_get_twin_ip_virutal_ip(void);
unsigned char* tp_get_twin_ip_mac(void);

int tp_twin_ip_is_send_redirect(struct sk_buff *skb);
int tp_twin_ip_is_send_arp(unsigned char* sha, unsigned int tip);
void tp_twin_ip_modify_input_packet(struct sk_buff *skb);
void tp_twin_ip_modify_output_packet(struct sk_buff *skb);


#endif
