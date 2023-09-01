/******************************************************************************
*
* Copyright (c) 2010 TP-LINK Technologies CO.,LTD.
* All rights reserved.
*
* FILE NAME  :   wlan_twin_ip.c
* VERSION    :   1.0
* DESCRIPTION:   为韩国Twin IP功能添加对ath0收上来的报文的处理.
*
* AUTHOR     :   huangwenzhong <huangwenzhong@tp-link.net>
* CREATE DATE:   08/24/2012
*
* HISTORY    :
* 01   08/24/2012  huangwenzhong     Create.
*
******************************************************************************/
#include <linux/types.h>
#include <linux/string.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/in.h>
#include <linux/version.h>
#include <linux/proc_fs.h>

#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/if.h>
#include <net/checksum.h>/*  by huangwenzhong, 24Aug12 */
#include <linux/if_ether.h>
#include <linux/udp.h>
#include <linux/tcp.h>
#include <net/ip.h>
#include <asm/byteorder.h>
#include <net/ip_vs.h>
#include <linux/tp_twin_ip.h>

////////////////////////////////////////////////////

/* new */
/* all are host order */
static int tp_twin_ip_enable = 0;
unsigned char tp_twin_ip_mac[TP_MAC_ADDR_SIZE] = {0};
static unsigned int tp_twin_wan_ip = 0;
static unsigned int tp_twin_virtual_ip = 0;/* 192.168.10.9 */

#define IP_FRAG_MASK		htons(0X1FFF)

/* endi */


//int		tp_arp_enable = 0;
//unsigned char		tp_arp_mac[TP_MAC_ADDR_SIZE] = {0};
//static unsigned int tp_arp_wan_ip = 0;

#ifdef CONFIG_PROC_FS
struct proc_dir_entry *tp_twin_ip_dir_entry = NULL;
struct proc_dir_entry *tp_twin_ip_enable_entry = NULL;
struct proc_dir_entry *tp_twin_ip_mac_entry = NULL;
struct proc_dir_entry *tp_twin_ip_wan_ip_entry = NULL;
struct proc_dir_entry *tp_twin_ip_vir_ip_entry = NULL;

int tp_is_twin_ip_enable(void)
{
	return tp_twin_ip_enable;
}

unsigned int tp_get_twin_ip_wan_ip(void)
{
	return tp_twin_wan_ip;
}

unsigned int tp_get_twin_ip_virutal_ip(void)
{
	return tp_twin_virtual_ip;
}

unsigned char* tp_get_twin_ip_mac(void)
{
	return tp_twin_ip_mac;
}

int tp_mac_str_2_eth_addr (char *buffer, char *ether)
{
	int i, c;

	if (NULL == buffer || NULL == ether)
	{
		return -1;
	}
	
	for (i = 0; i < TP_MAC_ADDR_SIZE; i++)
	{	
		if (*buffer == '-' || *buffer == ':')
		{
			buffer++;
		}

		if (*buffer >= '0' && *buffer <= '9')
		{
			c  = (unsigned char) (*buffer++ - '0'); 
		}
		else if (*buffer >= 'a' && *buffer <= 'f')
		{
			c  = (unsigned char) (*buffer++ - 'a') + 10;
		}
		else if (*buffer >= 'A' && *buffer <= 'F')
		{
			c  = (unsigned char) (*buffer++ - 'A') + 10; 
		}
		else
		{
			return -1;
		}

		c <<= 4;

		if (*buffer >= '0' && *buffer <= '9')
		{
			c |= (unsigned char) (*buffer++ - '0');
		}
		else if (*buffer >= 'a' && *buffer <= 'f')
		{
			c |= (unsigned char) (*buffer++ - 'a') + 10;
		}
		else
		{
			c |= (unsigned char) (*buffer++ - 'A') + 10;
		}

		ether[i] = (unsigned char) c;

	}

	return 0;
}


static int tp_twin_ip_enable_read (char *page, char **start, off_t off,
                               int count, int *eof, void *data)
{

	//printk("(%s)%d, tp_arp_enable = %d\n", __FUNCTION__, __LINE__, tp_arp_enable);
    return sprintf (page, "%d\n", tp_twin_ip_enable);
}

static int tp_twin_ip_enable_write (struct file *file, const char *buf,
                                        unsigned long count, void *data)
{
    int val;

	if (sscanf(buf, "%d", &val) != 1)
	{
        return -EINVAL;
	}
	
	if ((val != 0) && (val != 1))
	{
		return -EINVAL;
	}
	
	tp_twin_ip_enable = val;
	//printk("(%s)%d, tp_arp_enable = %d\n", __FUNCTION__, __LINE__, tp_arp_enable);

	return count;
}

static int tp_twin_ip_mac_read (char *page, char **start, off_t off,
                               int count, int *eof, void *data)
{
	unsigned char mac_str[TP_MAC_STR_SIZE] = {0};

	sprintf(mac_str, "%02X-%02X-%02X-%02X-%02X-%02X", 
		tp_twin_ip_mac[0], tp_twin_ip_mac[1], tp_twin_ip_mac[2], 
		tp_twin_ip_mac[3], tp_twin_ip_mac[4], tp_twin_ip_mac[5]);

    return sprintf (page, "%s\n", mac_str);
}

static int tp_twin_ip_mac_write (struct file *file, const char *buf,
                                        unsigned long count, void *data)
{
    unsigned char mac_str[TP_MAC_STR_SIZE] = {0};
	//int i = 0;

	if (sscanf(buf, "%s", mac_str) != 1)
	{
        return -EINVAL;
	}
	//printk("(%s)%d, mac_str = %s\n", __FUNCTION__, __LINE__, mac_str);
	tp_mac_str_2_eth_addr(mac_str, tp_twin_ip_mac);
	/*printk("(%s)%d, tp_twin_ip_mac = ", __FUNCTION__, __LINE__);
	for (i = 0; i < TP_MAC_ADDR_SIZE; i++)
	{
		printk("%02X", tp_twin_ip_mac[i]);
	}
	printk("\n");
	*/
	return count;
}

static int tp_twin_wan_ip_read (char *page, char **start, off_t off,
                               int count, int *eof, void *data)
{

	//printk("(%s)%d, tp_arp_enable = %d\n", __FUNCTION__, __LINE__, tp_arp_enable);
    return sprintf (page, "%u\n", tp_twin_wan_ip);
}

static int tp_twin_wan_ip_write (struct file *file, const char *buf,
                                        unsigned long count, void *data)
{
    unsigned int val;

	if (sscanf(buf, "%u", &val) != 1)
	{
        return -EINVAL;
	}
	
	tp_twin_wan_ip = val;
	//printk("(%s)%d, tp_arp_enable = %d\n", __FUNCTION__, __LINE__, tp_arp_enable);

	return count;
}


static int tp_twin_virtual_ip_read (char *page, char **start, off_t off,
                               int count, int *eof, void *data)
{

	//printk("(%s)%d, tp_arp_enable = %d\n", __FUNCTION__, __LINE__, tp_arp_enable);
    return sprintf (page, "%u\n", tp_twin_virtual_ip);
}

static int tp_twin_virtual_ip_write (struct file *file, const char *buf,
                                        unsigned long count, void *data)
{
    unsigned int val;

	if (sscanf(buf, "%u", &val) != 1)
	{
        return -EINVAL;
	}
	
	tp_twin_virtual_ip = val;
	//printk("(%s)%d, tp_arp_enable = %d\n", __FUNCTION__, __LINE__, tp_arp_enable);

	return count;
}


#endif

static int tp_twin_ip_proc_init(void)
{
#ifdef CONFIG_PROC_FS
	tp_twin_ip_dir_entry = proc_mkdir(TP_TWIN_IP_DIR_NAME, init_net.proc_net);
	if (!tp_twin_ip_dir_entry)
	{
		printk("(%s,%d)cannot create tp arp proc entry", __FUNCTION__, __LINE__);
		return -ENOENT;
	}
	
	tp_twin_ip_enable_entry = create_proc_entry(TP_TWIN_IP_ENABLE_FILE_NAME, 0666, tp_twin_ip_dir_entry);
	if (!tp_twin_ip_enable_entry)
	{
		printk("(%s,%d)cannot create tp arp enable proc entry", __FUNCTION__, __LINE__);
		return -ENOENT;
	}
	tp_twin_ip_enable_entry->write_proc = tp_twin_ip_enable_write;
	tp_twin_ip_enable_entry->read_proc	= tp_twin_ip_enable_read;

	tp_twin_ip_mac_entry = create_proc_entry(TP_TWIN_IP_MAC_FILE_NAME, 0666, tp_twin_ip_dir_entry);
	if (!tp_twin_ip_mac_entry)
	{
		printk("(%s,%d)cannot create tp arp mac proc entry", __FUNCTION__, __LINE__);
		return -ENOENT;
	}
	tp_twin_ip_mac_entry->write_proc = tp_twin_ip_mac_write;
	tp_twin_ip_mac_entry->read_proc	= tp_twin_ip_mac_read;

	tp_twin_ip_wan_ip_entry = create_proc_entry(TP_TWIN_IP_WAN_IP_FILE_NAME, 0666, tp_twin_ip_dir_entry);
	if (!tp_twin_ip_wan_ip_entry)
	{
		printk("(%s,%d)cannot create tp arp wan ip proc entry", __FUNCTION__, __LINE__);
		return -ENOENT;
	}
	tp_twin_ip_wan_ip_entry->write_proc = tp_twin_wan_ip_write;
	tp_twin_ip_wan_ip_entry->read_proc	= tp_twin_wan_ip_read;

	tp_twin_ip_vir_ip_entry = create_proc_entry(TP_TWIN_IP_VIRTUAL_IP_FILE_NAME, 0666, tp_twin_ip_dir_entry);
	if (!tp_twin_ip_vir_ip_entry)
	{
		printk("(%s,%d)cannot create tp arp wan ip proc entry", __FUNCTION__, __LINE__);
		return -ENOENT;
	}
	tp_twin_ip_vir_ip_entry->write_proc = tp_twin_virtual_ip_write;
	tp_twin_ip_vir_ip_entry->read_proc	= tp_twin_virtual_ip_read;
#endif
	return 0;
}


static void tp_twin_ip_proc_exit(void)
{
	if (tp_twin_ip_enable_entry)
	{
		remove_proc_entry(TP_TWIN_IP_ENABLE_FILE_NAME, tp_twin_ip_dir_entry);
	}

	if (tp_twin_ip_mac_entry)
	{
		remove_proc_entry(TP_TWIN_IP_MAC_FILE_NAME, tp_twin_ip_dir_entry);
	}

	if (tp_twin_ip_wan_ip_entry)
	{
		remove_proc_entry(TP_TWIN_IP_WAN_IP_FILE_NAME, tp_twin_ip_dir_entry);
	}

	if (tp_twin_ip_vir_ip_entry)
	{
		remove_proc_entry(TP_TWIN_IP_VIRTUAL_IP_FILE_NAME, tp_twin_ip_dir_entry);
	}

	if (tp_twin_ip_dir_entry)
	{
		remove_proc_entry(TP_TWIN_IP_DIR_NAME, init_net.proc_net);
	}

	

	return;
}


////////////////////////////////////////////////////




//static int wlan_twin_ip_enable = 1;
//static unsigned int wlan_twin_wan_ip = 0XAC1F0001;/* 172.31.0.1 */
//static unsigned int wlan_twin_virtual_ip = 0XC0A80A09;/* 192.168.10.9 */


/*
int wlan_twin_ip_ioctl(struct net_device *dev,struct ifreq *ifr, int cmd)
{
	struct twin_ip_cfg *twincfg = NULL;
	
	if (dev == NULL || ifr == NULL )
	{
		return -EINVAL;
	}

	 twincfg = (struct twin_ip_cfg *)ifr->ifr_data;
	 if (twincfg == NULL  || cmd != SIOCSTWINIPARG)
	 {
	 	return -EINVAL;
	 }

	 tp_twin_ip_enable = twincfg->tp_twin_ip_enable;
	 tp_twin_wan_ip = twincfg->tp_twin_wan_ip;
	 tp_twin_virtual_ip = twincfg->tp_twin_virtual_ip;

	 printk("(%s)%d, tp_twin_ip_enable = %d, tp_wan_ip = %u, tp_virtual_ip = %u\n",
	 	__FUNCTION__, __LINE__, tp_twin_ip_enable, tp_twin_wan_ip, tp_twin_virtual_ip);

	 return 0;
}
*/

int tp_twin_ip_is_send_redirect(struct sk_buff *skb)
{
    struct iphdr *iph = NULL;
	
	iph = ip_hdr(skb);
    if ((NULL != iph) && (0 != htonl(tp_twin_virtual_ip)) &&
		( iph->daddr == htonl(tp_twin_virtual_ip) || iph->saddr == htonl(tp_twin_virtual_ip)))
    {
        return 0;
    }
	
    return 1;
}

/* 
根据MAC地址和IP地址决定是否需要发送ARP REPLY报文
*/
int tp_twin_ip_is_send_arp(unsigned char* sha, unsigned int tip)
{
	if ((tp_twin_ip_enable) && (0 == memcmp(sha, tp_twin_ip_mac, TP_MAC_ADDR_SIZE)))
	{
		if ((tp_twin_wan_ip != 0) && (htonl(tp_twin_wan_ip) != tip))/* 当询问的地址不是它自己的地址时才应答 */
		{
			return 1;
		}
		return 2;
	}

	return 0;
}

/*  by huangwenzhong, 27Sep12 */
/* 重新计算校验和 */
static inline void
tp_udp_fast_csum_update(struct udphdr *uhdr,
		     __be32 oldip, __be32 newip,
		     __be16 oldport, __be16 newport)
{

	uhdr->check = csum_fold(ip_vs_check_diff4(oldip, newip,
				 			ip_vs_check_diff2(oldport, newport,
							~csum_unfold(uhdr->check))));
	if (!uhdr->check)
	{
		uhdr->check = CSUM_MANGLED_0;
	}
	return;
}

static inline void
tp_tcp_fast_csum_update(struct tcphdr *tcphdr,
		     __be32 oldip, __be32 newip,
		     __be16 oldport, __be16 newport)
{

	tcphdr->check = csum_fold(ip_vs_check_diff4(oldip, newip,
				 			ip_vs_check_diff2(oldport, newport,
							~csum_unfold(tcphdr->check))));
	if (!tcphdr->check)
	{
		tcphdr->check = CSUM_MANGLED_0;
	}
	return;
}


void tp_twin_ip_modify_input_packet(struct sk_buff *skb)
{
	struct iphdr *iph = NULL;
	struct udphdr *udph = NULL;
	struct tcphdr *tcph = NULL;
	struct ethhdr *ethh = NULL;

	unsigned short l3_proto = 0;
	unsigned char l4_proto = 0;

	unsigned char* ip_playload = NULL;

	__be32	saddr = 0;

	if (!tp_twin_ip_enable || skb == NULL)
	{
		return;
	}

	ethh = eth_hdr(skb);/* data point to ip header */

	l3_proto = ntohs(ethh->h_proto);

	if (l3_proto != ETH_P_IP)
	{
		return;
	}

	/*  not from the twin pc */
	if (memcmp(tp_twin_ip_mac, ethh->h_source, TP_MAC_ADDR_SIZE) != 0)
	{
		return;
	}
	
	iph = (struct iphdr*)(ethh + 1);

	if (iph->ihl < 5 || iph->version != 4)/* not ipv4 */
	{
		return;
	}

	saddr = iph->saddr;

	if (saddr != htonl(tp_twin_wan_ip))/* source ip is twin ip PC */
	{
		return;
	}

	l4_proto = iph->protocol;

	iph->saddr = htonl(tp_twin_virtual_ip);
	/* calc csum */
	ip_send_check(iph);

	ip_playload = (unsigned char*)((unsigned char*)iph + iph->ihl * 4);

	/*  by huangwenzhong, 17Aug12 */
	/* 
	这里有一定的风险，目前为止，L4层的协议只知道UDP和TCP协议的检验和会因为IP头部
	的变化而发生变化，而不清楚其它协议是否也会这样
	*/
	if (l4_proto == IPPROTO_UDP)
	{
		udph = (struct udphdr*)ip_playload;
		if ((iph->frag_off & IP_FRAG_MASK) == 0)
		{
			tp_udp_fast_csum_update(udph, saddr, iph->saddr, udph->source, udph->source);
		}
	}
	else if (l4_proto == IPPROTO_TCP)
	{
		tcph = (struct tcphdr*)ip_playload;
		/* only the none frag skb or the first frag skb need compute its checksum */
		if ((iph->frag_off & IP_FRAG_MASK) == 0)/*  by huangwenzhong, 27Sep12 */
		{
			tp_tcp_fast_csum_update(tcph, saddr, iph->saddr, tcph->source, tcph->source);
		}
	}

	return;
}

void tp_twin_ip_modify_output_packet(struct sk_buff *skb)
{
	struct ethhdr *ethh = NULL;
	struct iphdr *iph = NULL;
	struct udphdr *udph = NULL;
	struct tcphdr *tcph = NULL;
	
	__be32 daddr = 0;

	unsigned char *ip_playload = NULL;
	
	if (!tp_twin_ip_enable || skb == NULL)
	{
		return;
	}

    /* not find protocol & iph by skb->data to avoid locate error
       when vlan tag exists, by wkp, 1Aug14. */
	/*ethh = (struct ethhdr*)(skb->data);*/
	if (ntohs(skb->protocol) != ETH_P_IP)
	{
		return;
	}

    iph = ip_hdr(skb);
    if (NULL == iph)
    {
        return;
    }

	/*iph = (struct iphdr*)(ethh + 1);*/
	if (iph->ihl < 5 || iph->version != 4)/* not ipv4 */
	{
		return;
	}
	
	daddr = iph->daddr;
	if (htonl(tp_twin_virtual_ip) != daddr)/* packet not to Twin IP PC  */
	{
		return;
	}	

	iph->daddr = htonl(tp_twin_wan_ip);

	/* calc csum */
	ip_send_check(iph);

	ip_playload = (unsigned char*)((unsigned char*)iph + iph->ihl * 4);

	if (iph->protocol == IPPROTO_UDP)
	{
		udph = (struct udphdr*)ip_playload;
		if ((iph->frag_off & IP_FRAG_MASK) == 0)
		{
			tp_udp_fast_csum_update(udph, daddr, iph->daddr, udph->dest, udph->dest);
		}
	}
	else if (iph->protocol == IPPROTO_TCP)
	{
		tcph = (struct tcphdr*)ip_playload;
		if ((iph->frag_off & IP_FRAG_MASK) == 0)
		{
			tp_tcp_fast_csum_update(tcph, daddr, iph->daddr, tcph->dest, tcph->dest);
		}
	}	
	
	return;
}


static int __init tp_twin_ip_init(void)
{
	
#ifdef CONFIG_PROC_FS
	tp_twin_ip_proc_init();
#endif

	return 0;
}

static void __exit tp_twin_ip_exit(void)
{
	
#ifdef CONFIG_PROC_FS
	tp_twin_ip_proc_exit();
#endif

	return;
}

EXPORT_SYMBOL(tp_twin_ip_is_send_redirect);
EXPORT_SYMBOL(tp_twin_ip_is_send_arp);
EXPORT_SYMBOL(tp_twin_ip_modify_input_packet);
EXPORT_SYMBOL(tp_twin_ip_modify_output_packet);

module_init(tp_twin_ip_init);
module_exit(tp_twin_ip_exit);



MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huang Wenzhong <huangwenzhong@tp-link.net>");
MODULE_DESCRIPTION("The Twin IP Module for Korea.");
MODULE_ALIAS("TP Twin IP");


