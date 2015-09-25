bonding_thr
===========

Additional mode for the Linux Kernel bonding driver allowing slaves with different connection speeds to maximize throughput over ethernet based connections.

You'll need at least kernel headers to build, and you'll need to modify include/uapi/linux/if_bonding.h to include:

"#define BOND_MODE_THROUGHPUT    7"

You can load slaves with different weights, so net0:1; net1:2 would be the same as net0:1024; net1:2048.

