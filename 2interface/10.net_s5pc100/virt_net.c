
/*
 * 参考 drivers\net\cs89x0.c
 */

#include <linux/module.h>
#include <linux/errno.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/in.h>
#include <linux/skbuff.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/ip.h>
#include <linux/if_ether.h>

#include <asm/system.h>
#include <asm/io.h>
#include <asm/irq.h>

MODULE_LICENSE("GPL");
static struct net_device *vnet_dev;

void virt_tx(struct sk_buff *skb, struct net_device *dev)
{

	unsigned char *type;
	struct iphdr *ih;
	__be32 *saddr, *daddr, tmp;
	unsigned char	tmp_dev_addr[ETH_ALEN];
	struct ethhdr *ethhdr;
	
	struct sk_buff *rx_skb;
		
	/* 对调"源/目的"的mac地址 */
	ethhdr = (struct ethhdr *)skb->data;
	memcpy(tmp_dev_addr, ethhdr->h_dest, ETH_ALEN);
	memcpy(ethhdr->h_dest, ethhdr->h_source, ETH_ALEN);
	memcpy(ethhdr->h_source, tmp_dev_addr, ETH_ALEN);

	/* 对调"源/目的"的ip地址 */    
	ih = (struct iphdr *)(skb->data + sizeof(struct ethhdr));
	saddr = &ih->saddr;
	daddr = &ih->daddr;

	tmp = *saddr;
	*saddr = *daddr;
	*daddr = tmp;
	
	type = skb->data + sizeof(struct ethhdr) + sizeof(struct iphdr);
	// 修改类型, 原来0x8表示ping
	*type = 0; /* 0表示reply */
	
	ih->check = 0;		   /* and rebuild the checksum (ip needs it) */
	ih->check = ip_fast_csum((unsigned char *)ih,ih->ihl);
	
	rx_skb = dev_alloc_skb(skb->len + 2);
	skb_reserve(rx_skb, 2); /* align IP on 16B boundary */	
	memcpy(skb_put(rx_skb, skb->len), skb->data, skb->len);

	/* Write metadata, and then pass to the receive level */
	rx_skb->dev = dev;
	rx_skb->protocol = eth_type_trans(rx_skb, dev);
	rx_skb->ip_summed = CHECKSUM_UNNECESSARY; /* don't check it */
	dev->stats.rx_packets++;
	dev->stats.rx_bytes += skb->len;
	

	netif_rx(rx_skb);
}

static int virt_net_send_packet(struct sk_buff *skb, struct net_device *dev)
{

	netif_stop_queue(dev); /* 停止该网卡的队列 */

	virt_tx(skb, dev);

	dev_kfree_skb (skb);   /* 释放skb */
	netif_wake_queue(dev); /* 数据全部发送出去后,唤醒网卡的队列 */

	dev->stats.tx_packets++;
	dev->stats.tx_bytes += skb->len;
	
	return 0;
}


struct net_device_ops  netops ={

		.ndo_init = NULL,
		.ndo_start_xmit=virt_net_send_packet,
};
static int virt_net_init(void)
{	int err =0 ;
	vnet_dev = alloc_etherdev(0);  /* alloc_etherdev */
	
	vnet_dev->netdev_ops = &netops;
	
	vnet_dev->dev_addr[0]=0x11;
	vnet_dev->dev_addr[1]=0x22;
	vnet_dev->dev_addr[2]=0x33;
	vnet_dev->dev_addr[3]=0x44;
	vnet_dev->dev_addr[4]=0x55;
	vnet_dev->dev_addr[5]=0x66;

	//ping respones
	vnet_dev->flags           |= IFF_NOARP;
	//vnet_dev->flags           |= IFF_BROADCAST;

	err = register_netdev(vnet_dev);
	if(err)
	{
		printk(KERN_ERR" net dev register failed\n");
	}
	printk(KERN_ERR" %s \n", vnet_dev->name);
	printk(KERN_ERR"NET REGISTERED\n");	
	return 0;
}
static void virt_net_exit(void)
{
	unregister_netdev(vnet_dev);
	free_netdev(vnet_dev);
	printk(KERN_ERR"net dev unregister\n");
}

module_init(virt_net_init);
module_exit(virt_net_exit);


