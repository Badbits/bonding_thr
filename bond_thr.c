#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/pkt_sched.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#include <linux/if_bonding.h>
#include <linux/if_vlan.h>
#include <linux/in.h>
#include <net/ipx.h>
#include <net/arp.h>
#include <net/ipv6.h>
#include <asm/byteorder.h>
#include "bonding.h"
#include "bond_thr.h"

/* default link speed */
#define BOND_THR_LINK_SPEED 1024

int bond_xmit_throughput(struct sk_buff *skb, struct net_device *bond_dev)
{
	struct bonding *bond = netdev_priv(bond_dev);
	struct slave *slave, *start_at;
	int i,  res = 1, min_ratio = INT_MAX;
	struct iphdr *iph = ip_hdr(skb);

	read_lock(&bond->lock);

	if (!BOND_IS_OK(bond))
		goto out;
	/*
	 * Start with the curr_active_slave that joined the bond as the
	 * default for sending IGMP traffic.  For failover purposes one
	 * needs to maintain some consistency for the interface that will
	 * send the join/membership reports.  The curr_active_slave found
	 * will send all of this type of traffic.
	 */
	if ((iph->protocol == IPPROTO_IGMP) &&
	    (skb->protocol == htons(ETH_P_IP))) {

		read_lock(&bond->curr_slave_lock);
		slave = bond->curr_active_slave;
		read_unlock(&bond->curr_slave_lock);

		if (!slave)
			goto out;

		start_at = slave;
		bond_for_each_slave_from(bond, slave, i, start_at) {
			if (IS_UP(slave->dev) &&
			    (slave->link == BOND_LINK_UP) &&
			    (slave->state == BOND_STATE_ACTIVE)) {
				res = bond_dev_queue_xmit(bond, skb, slave->dev);
				break;
			}
		}

	} else {
		start_at = bond->first_slave;

		bond_thr_param_lock(bond);
		bond_for_each_slave(bond, slave, i) {
			if (IS_UP(slave->dev) &&
				(slave->link == BOND_LINK_UP) &&
				(slave->state == BOND_STATE_ACTIVE)) {

				if(slave->tx_hist == 0) {
					start_at = slave;
					break;
				}	

				if (min_ratio > slave->tx_hist/slave->link_speed) {
					min_ratio = slave->tx_hist/slave->link_speed;
					start_at  = slave;
				}
			}
		}

		bond_for_each_slave_from(bond, slave, i, start_at) {
			if (IS_UP(slave->dev) &&
				(slave->link == BOND_LINK_UP) &&
				(slave->state == BOND_STATE_ACTIVE)) {					
					res = bond_dev_queue_xmit(bond, skb, slave->dev);
					/* update counter if frame sent */
					if(!res) {
						slave->tx_hist+= skb->len;
					}
					break;
				}
		}

		/* rescale tx_hist to avoid uint32 counter overflow */
		if (slave->tx_hist > INT_MAX) {
			bond_for_each_slave(bond, slave, i) {
				slave->tx_hist/= 2;
			}
		}
		bond_thr_param_unlock(bond);
	}

out:
	if (res) {
		/* no suitable interface, frame not sent */
		dev_kfree_skb(skb);
	}
	read_unlock(&bond->lock);
	return NETDEV_TX_OK;
}

int bond_thr_init_slave(struct bonding *bond, struct slave *slave) {

	slave->tx_hist    = 0;
	slave->link_speed = BOND_THR_LINK_SPEED;
	spin_lock_init(&(bond->thr_param_lock));

	/* @TODO sysfs initialization */

	return 0;
}

void bond_thr_param_lock(struct bonding *bond) 
{
	spin_lock_bh(&(bond->thr_param_lock));
}

void bond_thr_param_unlock(struct bonding *bond) 
{
	spin_unlock_bh(&(bond->thr_param_lock));
}

