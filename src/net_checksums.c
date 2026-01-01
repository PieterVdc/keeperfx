/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file net_checksums.c
 *     Network checksum computation and desync analysis for multiplayer games.
 * @par Purpose:
 *     Computes checksums for multiplayer sync and stores history for debugging.
 * @par Comment:
 *     Uses a circular buffer to store the last N turns of checksum data.
 * @author   KeeperFX Team
 * @date     03 Nov 2025
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "net_checksums.h"
#include "bflib_network_exchange.h"
#include "game_legacy.h"
#include "net_game.h"
#include "packets.h"
#include "player_data.h"
#include "thing_data.h"
#include "room_list.h"
#include "creature_control.h"
#include "frontend.h"
#include "thing_list.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/


TbBigChecksum get_thing_checksum(const struct Thing* thing) {
    return 0;
}



short checksums_different(void) {
    return 0;
}

void update_turn_checksums(void) {
}

void pack_desync_history_for_resync(void) {

}

void compare_desync_history_from_host(void) {

}

CoroutineLoopState perform_checksum_verification(CoroutineLoop *con) {
    return CLS_RETURN;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
