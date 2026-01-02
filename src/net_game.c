/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file net_game.c
 *     Network game support for Dungeon Keeper.
 * @par Purpose:
 *     Functions to exchange packets through network.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     11 Mar 2010 - 09 Oct 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "net_game.h"

#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
struct TbNetworkPlayerInfo net_player_info[NET_PLAYERS_COUNT];
struct TbNetworkSessionNameEntry *net_session[32];
long net_number_of_sessions;
long net_session_index_active;
struct TbNetworkPlayerName net_player[NET_PLAYERS_COUNT];
struct ConfigInfo net_config_info;
char net_service[16][NET_SERVICE_LEN];
char net_player_name[20];
/******************************************************************************/
short setup_network_service(int srvidx)
{
  return 0;
}

unsigned long get_host_player_id(void) {
    return 0;
}

int setup_old_network_service(void)
{
  return 0;
}



void setup_count_players(void)
{
}

void init_players_network_game(CoroutineLoop *context)
{
}

TbBool network_player_active(int plyr_idx)
{
  return false;
}

const char *network_player_name(int plyr_idx)
{
  return "";
}

long network_session_join(void)
{
    return 0;
}

void sync_various_data()
{
 
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
