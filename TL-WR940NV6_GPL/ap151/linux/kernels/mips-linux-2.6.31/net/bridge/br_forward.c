/** Copyright (c) 2013 Qualcomm Atheros, Inc. */

/*
 *	Forwarding decision
 *	Linux ethernet bridge
 *
 *	Authors:
 *	Lennert Buytenhek		<buytenh@gnu.org>
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/if_vlan.h>
#include <linux/netfilter_bridge.h>
#include "br_private.h"
#if CONFIG_MODULE_VLAN_FOR_SPECIAL_ISP
#include "../8021q/vlan.h"
#endif

/*start added by huanglifu 2012/07/02. for Vietnam viettel isp demand that IPTV VLAN IGMP Snooping*/
#include <linux/tp_mroute.h>
/*end added by huanglifu 2012/07/02. for Vietnam viettel isp demand that IPTV VLAN IGMP Snooping*/

/* Don't forward packets to originating port or forwarding diasabled */
static inline int should_deliver(const struct net_bridge_port *p,
				 const struct sk_buff *skb)
{

	if ((skb->dev == p->dev) ||  (p->state != BR_STATE_FORWARDING)) 		
	{
		return 0;
	}

    /*���û�����߿ͻ��˼���ಥ�飬��Ӧ��ת��������*/
    if((!strcmp(p->dev, "ath0")) && (0x10000000 == (skb->mark & 0xFFFF0000)))
    {
	    return 0;
    }

	return 1;
}

static inline unsigned packet_length(const struct sk_buff *skb)
{
	return skb->len - (skb->protocol == htons(ETH_P_8021Q) ? VLAN_HLEN : 0);
}

int br_dev_queue_push_xmit(struct sk_buff *skb)
{
	/* drop mtu oversized packets except gso */
	if (packet_length(skb) > skb->dev->mtu && !skb_is_gso(skb))
		kfree_skb(skb);
	else {
		/* ip_refrag calls ip_fragment, doesn't copy the MAC header. */
		if (nf_bridge_maybe_copy_header(skb))
			kfree_skb(skb);
		else {
			skb_push(skb, ETH_HLEN);

			dev_queue_xmit(skb);
		}
	}

	return 0;
}

int br_forward_finish(struct sk_buff *skb)
{
	return NF_HOOK(PF_BRIDGE, NF_BR_POST_ROUTING, skb, NULL, skb->dev,
		       br_dev_queue_push_xmit);

}

static void __br_deliver(const struct net_bridge_port *to, struct sk_buff *skb)
{
	skb->dev = to->dev;
	NF_HOOK(PF_BRIDGE, NF_BR_LOCAL_OUT, skb, NULL, skb->dev,
			br_forward_finish);
}
#if CONFIG_MODULE_VLAN_FOR_SPECIAL_ISP
extern int iptv_eth0_index;
extern int lan_bridge_mask;
#endif
static void __br_forward(const struct net_bridge_port *to, struct sk_buff *skb)
{
	struct net_device *indev;
#if CONFIG_MODULE_VLAN_FOR_SPECIAL_ISP
	struct net_bridge_port *from = NULL;
	struct net_device *compare1, *compare2;
#endif

	if (skb_warn_if_lro(skb)) {
		kfree_skb(skb);
		return;
	}

	indev = skb->dev;
	skb->dev = to->dev;
	skb_forward_csum(skb);

#if CONFIG_MODULE_VLAN_FOR_SPECIAL_ISP
	if (lan_bridge_mask > 0)
	{
		compare1 = vlan_dev_info(indev)->real_dev;
		compare2 = vlan_dev_info(skb->dev)->real_dev;

		if (compare1 != NULL && compare2 != NULL && compare1 == compare2)
		{
			kfree_skb(skb);
			return;
		}
	}
#endif
	/*start added by huanglifu 2012/07/02. for Vietnam viettel isp demand that IPTV VLAN IGMP Snooping*/
#ifdef VLAN_IPTV_IGMP_SNOOPING
#if 0
	from = rcu_dereference(indev->br_port);
	if (tp_mr_classify_iptv(skb, from))
	{
		//printk("[%s]drop skb  skb->dev->ifindex =0x%x  iptv_eth0_index=0x%x dev:%s!!!!!!!!!!!!!!!!!!\n", __func__, skb->dev->ifindex, iptv_eth0_index, skb->dev->name);
		return;
	}
#else
	if (tp_mr_classify_iptv(skb))
	{
		return;
	}
#endif
#endif
	/*end added by huanglifu 2012/07/02. for Vietnam viettel isp demand that IPTV VLAN IGMP Snooping*/
	

	NF_HOOK(PF_BRIDGE, NF_BR_FORWARD, skb, indev, skb->dev,
			br_forward_finish);
}

/* called with rcu_read_lock */
void br_deliver(const struct net_bridge_port *to, struct sk_buff *skb)
{
	if (should_deliver(to, skb)) {
		__br_deliver(to, skb);
		return;
	}

	kfree_skb(skb);
}

/* called with rcu_read_lock */
void br_forward(const struct net_bridge_port *to, struct sk_buff *skb)
{
#ifdef CONFIG_ATH_HOTSPOT
	/* the src port and dest port are the same and this port has l2tif
	 * enabled, then forward the frame to the configured wan port
	 */
	if (to->l2tif && skb->dev == to->dev) {
		struct net_bridge *br = to->br;
		struct net_bridge_port *p;
		list_for_each_entry_rcu(p, &br->port_list, list) {
			if (p->iswan) {
				to = p;
				break;
			}
		}
	}
#endif
	if (should_deliver(to, skb)) {
		__br_forward(to, skb);
		return;
	}

	kfree_skb(skb);
}

/* called under bridge lock */
static void br_flood(struct net_bridge *br, struct sk_buff *skb,
	void (*__packet_hook)(const struct net_bridge_port *p,
			      struct sk_buff *skb))
{
	struct net_bridge_port *p;
	struct net_bridge_port *prev;

	prev = NULL;

	/* backup multicast address. by HouXB, 07Dec10 */
#ifdef CONFIG_TP_MULTICAST			
#define IS_MULTICAST_ADDR(ptr)  ((ptr[0] == 0x01) && (ptr[1] == 0x00) && (ptr[2] == 0x5e) ? 1 : 0)
	
		mac_addr multi_mac_addr;
		unsigned char *pmac = multi_mac_addr.addr;
		memset(pmac, 0, 6/*ETH_ALEN*/);
		
		if(IS_MULTICAST_ADDR(skb_mac_header(skb)))
		{
			//backup multicast address
			memcpy(pmac, skb_mac_header(skb), 6/*ETH_ALEN*/);
		}
#endif

	list_for_each_entry_rcu(p, &br->port_list, list) {
		if (should_deliver(p, skb)) {
#ifdef CONFIG_ATH_WRAP
			if(PTYPE_IS_WRAP(p->type))
				skb->mark |= WRAP_BR_MARK_FLOOD;
#endif
			if (prev != NULL) {
				struct sk_buff *skb2;

				if ((skb2 = skb_clone(skb, GFP_ATOMIC)) == NULL) {
					br->dev->stats.tx_dropped++;
					kfree_skb(skb);
					return;
				}

#ifdef CONFIG_TP_MULTICAST				
				if(IS_MULTICAST_ADDR(pmac))
				{
					//restore multicast address
					memcpy(skb_mac_header(skb), pmac, 6/*ETH_ALEN*/);
				}
#endif

				__packet_hook(prev, skb2);
			}

			prev = p;
		}
	}

	if (prev != NULL) {
#ifdef CONFIG_TP_MULTICAST				
		if(IS_MULTICAST_ADDR(pmac))
		{
			//restore multicast address
			memcpy(skb_mac_header(skb), pmac, 6/*ETH_ALEN*/);
		}
#endif

		__packet_hook(prev, skb);
		return;
	}

	kfree_skb(skb);
}


/* called with rcu_read_lock */
void br_flood_deliver(struct net_bridge *br, struct sk_buff *skb)
{
	br_flood(br, skb, __br_deliver);
}

/* called under bridge lock */
void br_flood_forward(struct net_bridge *br, struct sk_buff *skb)
{
	br_flood(br, skb, __br_forward);
}
