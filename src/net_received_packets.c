/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file received_packets.c
 *     Received packets list tracking for network game synchronization.
 * @par Purpose:
 *     Stores and retrieves received packet data for multiplayer desync debugging.
 * @par Comment:
 *     Uses a circular buffer to store the last N turns of received packets.
 * @author   KeeperFX Team
 * @date     24 Oct 2025
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "net_received_packets.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_network.h"
#include "packets.h"
#include "player_data.h"
#include "game_legacy.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/




/******************************************************************************/

void clear_packet_tracking(void) {

}

void initialize_packet_tracking(void) {
}

void store_received_packet(GameTurn turn, PlayerNumber player, const struct Packet* packet) {

}

void store_received_packets(void) {

}

const struct Packet* get_received_packets_for_turn(GameTurn turn) {
    return NULL;
}

const struct Packet* get_received_packet_for_player(GameTurn turn, PlayerNumber player) {
    return NULL;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
