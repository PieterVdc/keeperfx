/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file redundant_packets.c
 *     Redundant packet bundling for network game packet loss prevention.
 * @par Purpose:
 *     Bundles current packet with previous 2 packets to prevent packet loss.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     11 Nov 2025
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "net_redundant_packets.h"
#include "packets.h"
#include "net_received_packets.h"
#include "bflib_network.h"
#include <string.h>
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/


/******************************************************************************/

void initialize_redundant_packets(void) {
}

void clear_redundant_packets(void) {
}

void store_sent_packet(PlayerNumber player, const struct Packet* packet) {

}



/******************************************************************************/
#ifdef __cplusplus
}
#endif
