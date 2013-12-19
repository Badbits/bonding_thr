#ifndef __BOND_THR_H__
#define __BOND_THR_H__

#include <linux/if_ether.h>

int bond_xmit_throughput(struct sk_buff *skb, struct net_device *bond_dev);
int bond_thr_init_slave(struct bonding *bond, struct slave *slave);
void bond_thr_param_lock(struct bonding *bond);
void bond_thr_param_unlock(struct bonding *bond);

#endif /* __BOND_THR_H__ */

