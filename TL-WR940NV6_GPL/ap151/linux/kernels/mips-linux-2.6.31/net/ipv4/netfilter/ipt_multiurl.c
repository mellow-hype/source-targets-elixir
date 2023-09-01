/******************************************************************************
 *
 * Copyright (c) 2008-2008 TP-Link Technologies CO.,LTD.
 * All rights reserved.
 *
 * 文件名称:	ipt_multiurl.c
 * 版    本:	1.0
 * 摘    要:	netfilter kernel module
 *
 * 作    者:	ZJin <zhongjin@tp-link.net>
 * 创建日期:	10/23/2008
 *
 * 修改历史:
----------------------------
 *
 ******************************************************************************/

/* for example */
/*
	iptables -A FORWARD -m multiurl --urls www.baidu.com,sina,163 -p tcp --dport 80 -j ACCEPT

	netfilter will find the urls in every http pkt, if found, means matched!
*/

#if defined(MODVERSIONS)
#include <linux/modversions.h>
#endif
#include <linux/module.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/skbuff.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ipt_multiurl.h>

#include <linux/ip.h>
#include <linux/tcp.h>

#define	HOST_STR	"\r\nHost: "
#ifdef CONFIG_NF_HTTPS_URL_FILTER
#define DNS_PORT 53
#define HTTP_PORT 80
#define HTTPS_PORT 443

#define HANDSHAKE 22 /*ssl: content type.SSL_TYPE*/
#define CLIENT_HELLO 1 /*handshake: content type.HANDSHAKE_TYPE*/
#define SERVER_NAME 0 /*extension type in client hello(can only appear once in client hello).EXTENSION_TYPE*/
#define HOST_NAME 0 /*content type in SNI(in server_name extension).SERVER_NAME_TYPE*/


typedef struct _PROTOCOL_VERSION
{
	uint8_t majorVersion;
	uint8_t minorVersion;
}PROTOCOL_VERSION;

typedef struct _SSL_MSG{
	uint8_t type; /*len:1 byte*/
	PROTOCOL_VERSION version; /*len:2 bytes*/
	uint16_t length; /* The length (in bytes) of the following TLSPlaintext.fragment.*/
	uint8_t *pContent; /*  The application data,type is specified by the type field.*/
}SSL_MSG;

typedef uint32_t uint24_t;

typedef struct{
	uint16_t length;
	uint8_t *pData;
}CIPHER_SUITE,CH_EXTENSIONS;

typedef struct{
	uint8_t length;
	uint8_t *pData;
}SESSION_ID,COMPRESSION_METHOD;

typedef struct _TLS_EXTENSION{
	uint16_t type;
	uint16_t length;
	uint8_t *pData;
}TLS_EXTENSION;/*TLS(client hello) extension*/

typedef struct _HANDSHAKE_CLIENT_HELLO{
	uint8_t type; /*len:1 byte*/
	uint24_t length;
	PROTOCOL_VERSION clientVersion;
    uint8_t *random;/*the length is 32,but we don't need this field.So only give pointer to start position*/
    SESSION_ID sessionID;
    CIPHER_SUITE cipherSuites;
    COMPRESSION_METHOD compression_methods;
    uint8_t *pExtensions /*pointer to extensions length field*/;
}HANDSHAKE_CLIENT_HELLO;


/* 
 * fn		static bool extractHandshakeFromSSL(const uint8_t *pSSLBuff, uint8_t **ppHandshake) 
 * brief	extract the handshake From SSL packet.
 * details	only get address of the pointer to handshake.
 *
 * param[in]	pSSL - pointer to the start of SSL packet in skb_buff.
 * param[out]	ppHandshake - address of pointer to the start of handshake message wrapped with SSLv3/TLS.
 *
 * return	BOOL
 * retval	true  succeed to extract handshake 
 *		false fail to extract handshake  
 * note		
 */
static bool extractHandshakeFromSSL(const uint8_t *pSSL, uint8_t **ppHandshake)
{
	SSL_MSG ssl;
	
	if ((ssl.type = *pSSL++) != HANDSHAKE)
	{
		return false;
	}
	/*
	ssl.version.majorVersion = *pSSL++;
	ssl.version.minorVersion = *pSSL++;
	*/
	pSSL += 2;
	
	ssl.length = ntohs(*((uint16_t *)pSSL));
	pSSL += 2;
	
	if(0 == ssl.length)
	{
		return false;
	}
	/*ssl.pContent = pSSL;*/
	*ppHandshake = (uint8_t *)pSSL;

	
	return true;
}
/* 
 * fn		static bool extractSNIFromExtensions(const uint8_t *pExtensions, uint8_t *ppSNIExt) 
 * brief	extract SNI extension form extensions.
 * details	get pointer to start position of SNI extension that exists in server name extension.
 *
 * param[in]	pExtensions - pointer to start of extensionList.
 * param[out]	ppSNIExt      - address of pointer to SNI extension.
 *
 * return	bool
 * retval	true - extract SNI extension successfully.
 *          false - extract SNI extension unsuccessfully.
 * note		
 */
static bool extractSNIFromExtensions(const uint8_t *pExtensions, uint8_t **ppSNIExt)
{
	int extensionsLen; /*length of all extensions.*/
	int handledExtLen;/*length of handled extensions.*/
	TLS_EXTENSION ext;

	extensionsLen = ntohs(*((uint16_t *)pExtensions));
	pExtensions += 2;
	
	for (handledExtLen = 0; handledExtLen < extensionsLen; )
	{
		ext.type = ntohs(*((uint16_t *)pExtensions));
		pExtensions += 2;
		ext.length = ntohs(*((uint16_t *)pExtensions));
		pExtensions += 2;
		ext.pData = (ext.length ? (uint8_t *)pExtensions : NULL);
		if (SERVER_NAME == ext.type)
		{
			*ppSNIExt = ext.pData;
			if (ext.length)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		pExtensions += ext.length;
		handledExtLen += (2 + 2 + ext.length);
	}

	return false;
}
/* 
 * fn		static  bool extractSNIFromClientHello(const uint8_t *pClientHello, uint8_t **ppSNIExt) 
 * brief	extract SNI extension(Server_name)represents host_name from client_hello.
 * details	get pointer to start position of SNI extension from client_hello message.
 *
 * param[in]	pClientHello - pointer to start position of client_hello message.
 * param[out]	ppSNIExt - address of pointer to the start position of SNI extension in client_hello.
 *
 * return	bool
 * retval	true -get the SNI represents host_name.
 *			false - doesn't get the right SNI.
 * note		
 */
static bool extractSNIFromClientHello(const uint8_t *pClientHello, uint8_t **ppSNIExt)
{
	HANDSHAKE_CLIENT_HELLO clientHello;
	/*
	clientHello.type = *pClientHello++;
	clientHello.length = NET_3BYTES_TO_HOST_UINT32(pClientHello);
	pClientHello += 3;
	Ignore type and length of client_hello.
	*/
	pClientHello += 4;
	
	clientHello.clientVersion.majorVersion = *pClientHello++;
	clientHello.clientVersion.minorVersion = *pClientHello++;
	/*SNI extension is not supported until TLS 1.0(version 0x0301)*/
	if (clientHello.clientVersion.majorVersion < 3
	 || (3 == clientHello.clientVersion.majorVersion && 0 == clientHello.clientVersion.minorVersion))
	{
		return false;
	}
	/*clientHello.random = pClientHello;*/
	pClientHello += 32;/*length of random is fixed.*/
	clientHello.sessionID.length = *pClientHello++;
	/*clientHello.sessionID.pData = pClientHello;*/
	pClientHello += clientHello.sessionID.length;
	clientHello.cipherSuites.length = ntohs(*((uint16_t *)pClientHello));
	pClientHello += 2;
	/*clientHello.cipherSuites.pData = pClientHello;*/
	pClientHello += clientHello.cipherSuites.length;
	clientHello.compression_methods.length = *pClientHello++;
	/*clientHello.compression_methods.pData = pClientHello;*/
	
	pClientHello += clientHello.compression_methods.length;
	clientHello.pExtensions = (uint8_t *)pClientHello;

	return extractSNIFromExtensions(clientHello.pExtensions, ppSNIExt);
}
#endif

/*
start: the search area start mem addr;
end:	the search area end mem addr;

strCharSet:
Null-terminated string to search for
*/
u_int8_t *url_strstr(const u_int8_t* start, const u_int8_t* end, const u_int8_t* strCharSet)
{
	const u_int8_t *s_temp = start;        /*the s_temp point to the s*/

	int l1, l2;

	l2 = strlen(strCharSet);
	if (!l2)
		return (u_int8_t *)start;

	l1 = end - s_temp + 1;

	while (l1 >= l2)
	{
		l1--;
		if (!memcmp(s_temp, strCharSet, l2))
			return (u_int8_t *)s_temp;
		s_temp++;
	}

	return NULL;
}


static bool match(const struct sk_buff *skb, 
      const struct xt_match_param *param)
{	
	const struct xt_multiurl_info *info = param->matchinfo;		

	const struct iphdr *iph = ip_hdr(skb);

    struct tcphdr *tcph = (void *)iph + iph->ihl*4;	/* Might be TCP, UDP */

#ifdef CONFIG_NF_HTTPS_URL_FILTER
    if(HTTP_PORT == ntohs(tcph->dest))
    {
#endif	
	u_int8_t*	http_payload_start = NULL;
	u_int8_t*	http_payload_end = NULL;

	
	u_int8_t*	host_str_start = NULL;
	u_int8_t*	url_start = NULL;
	u_int8_t*	url_end = NULL;
	u_int8_t*	crlf_pos = NULL;
	
	/* int found_crlf = 0; */

	int i;

	/* ZJin: we should check HOST first, if the pkt has no HOST, 
		such as ack/syn/fin or continuous POST pkt, 
		we should process it by "returnX flag" */
#if 0
	/* return ack/syn/fin pkts ZJin 090409 */
	if (iph->tot_len == iph->ihl*4 + tcph->doff*4)
	{
		//printk("1st-if: ack/syn/fin pkts!\n");
		/* return 1 or 0 based the last "FLAG" string. ZJin090417 */
		if (!memcmp(info->urls[info->url_count - 1], "return1", 7))
			return 1;
		else
			return 0;
	}
#endif
	
	http_payload_start = (u_int8_t *)tcph + tcph->doff*4;
	http_payload_end = http_payload_start + (iph->tot_len - iph->ihl*4 - tcph->doff*4) - 1;

#if 0
	/* usually it's a ack/syn pkt */
	if (http_payload_start == http_payload_end)
	{
		//printk("1st-if: not a get pkt!\n");
		return 1;
	}
#endif

	/* check "\r\nHOST:" in payload, pass other pkts, by zhanjinguang, 11Aug10 */
	if (http_payload_start < http_payload_end)
	{
		host_str_start = url_strstr(http_payload_start, http_payload_end, HOST_STR);
	}

	if (host_str_start != NULL)
	{
		url_start = host_str_start + 8;

		crlf_pos = url_strstr(url_start, http_payload_end, "\r\n");

		if (crlf_pos == NULL)
			return 0;	/* Host without \r\n... */

		/* the end of "www.xxx.com" */
		url_end = crlf_pos - 1;
		#if 0
		crlf_pos = url_start;

		while (0 == found_crlf)
		{
			if ((*crlf_pos == '\r') && (*(crlf_pos + 1) == '\n'))
			{
				found_crlf = 1;
			}
			crlf_pos++;
		}
		/* the end of "www.xxx.com" */
		url_end = crlf_pos - 2;
		#endif
		/* ZJin: 090417 the last "URL" just is a FLAG */
		for (i = 0; i < info->url_count - 1; ++i)
		{
			if (url_strstr(url_start, url_end, info->urls[i]))
			{
				//printk("==== matched %s ====\n", info->urls[i]);
				
				return 1;
			}
		}

		/* strict match, should return 0, by zhanjinguang 11Aug10 */
		return 0;
	}
	else
	{
		/* return value according to "returnx", by zhanjinguang, 11Aug10 */
		if (!memcmp(info->urls[info->url_count - 1], "return1", 7))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
#ifdef CONFIG_NF_HTTPS_URL_FILTER
    }
    else if (HTTPS_PORT == ntohs(tcph->dest))
    {
        unsigned char *sslStart;
        unsigned char *sslEnd;
        uint8_t *pHandshake;
	uint8_t * pSNIExt;
	int defaultret = (!memcmp(info->urls[info->url_count - 1], "return1", 7))?1:0;
        int i;
        TLS_EXTENSION SNIExt;/*format is similar with server name extension*/
        int SNIListLen;
        int handledSNILen; 
        sslStart = (unsigned char *)tcph + tcph->doff * 4;
	sslEnd = sslStart + (ntohs(iph->tot_len) - iph->ihl * 4 - tcph->doff * 4);
        if (sslStart >= sslEnd)
        {
            /*UNIDENTIFY*/
            return defaultret;
        }
        if ((!extractHandshakeFromSSL(sslStart, &pHandshake))
	|| (*pHandshake != CLIENT_HELLO)
	|| (!extractSNIFromClientHello(pHandshake, &pSNIExt)))
        {
            /*UNIDENTIFY*/
            return defaultret;
		}
        SNIListLen = ntohs(*((uint16_t *)pSNIExt));
        pSNIExt += 2;

        for (handledSNILen = 0; handledSNILen < SNIListLen; )
        {
            SNIExt.type = *pSNIExt++;
            SNIExt.length = ntohs(*((uint16_t *)pSNIExt));
            pSNIExt += 2;
            SNIExt.pData = (uint8_t *)pSNIExt;
            pSNIExt += SNIExt.length;
            /*Does CLENT HELLO  fragment have impact on SNI?*/
            if (pSNIExt > sslEnd)
            {
                /*UNIDENTIFY*/
                return defaultret;
            }
            handledSNILen += (1 + 2 + SNIExt.length);
            if (HOST_NAME == SNIExt.type)
            {
                for (i = 0; i < info->url_count-1; ++i)
                {
                    if(url_strstr(SNIExt.pData,pSNIExt,info->urls[i]))
                    {    
                        return 1;
                    }
                }
            }
        }
        return 0;	
    }
    return (!memcmp(info->urls[info->url_count - 1], "return1", 7))?1:0;
#endif	
}


/* Called when user tries to insert an entry of this type. */

static bool checkentry(const struct xt_mtchk_param *param)
{
	if (param->match->matchsize != XT_ALIGN(sizeof(struct xt_multiurl_info)))		
		return 0;

	return 1;
}


static struct xt_match multiurl_match = { 
    .name           = "multiurl",
    .family			= NFPROTO_IPV4,
    .match          = &match,
    .checkentry     = &checkentry,
    .matchsize		= XT_ALIGN(sizeof(struct xt_multiurl_info)),
    .me             = THIS_MODULE,
};

static int __init init(void)
{
	return xt_register_match(&multiurl_match);
}

static void __exit fini(void)
{
	xt_unregister_match(&multiurl_match);
}

module_init(init);
module_exit(fini);

MODULE_AUTHOR("ZJin");
MODULE_DESCRIPTION("A module to match MultiURL in host section of http get!.");
MODULE_LICENSE("GPL");

