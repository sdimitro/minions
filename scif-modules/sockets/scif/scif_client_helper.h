#ifndef __SCIF_CLIENT_HELPER_H
#define __SCIF_CLIENT_HELPER_H

#include <scif.h>

#ifdef HOST
#define PEER_NODE 1
#else
#define PEER_NODE 0
#endif

#define PEER_PORT  2050
#define LOCAL_PORT 2049

#define MAX_TRIES 20

/* SCIF - open -> bind -> connect */
scif_epd_t scif_obc(void);

#endif /* __SCIF_CLIENT_HELPER_H */
