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
   if ((game.system_flags & GSF_NetworkActive) == 0) {
      return;
   }

   struct {
      uint32_t action_random_seed;
      int input_lag_turns;
   } initial_sync_data;

   initial_sync_data.action_random_seed = game.action_random_seed;
   initial_sync_data.input_lag_turns = game.input_lag_turns;
   if (!LbNetwork_Resync(&initial_sync_data, sizeof(initial_sync_data))) {
      ERRORLOG("Initial sync failed");
      return;
   }
   game.action_random_seed = initial_sync_data.action_random_seed;
   game.ai_random_seed = game.action_random_seed * 9377 + 9391;
   game.player_random_seed = game.action_random_seed * 9473 + 9479;
   game.input_lag_turns = initial_sync_data.input_lag_turns;
   game.skip_initial_input_turns = calculate_skip_input();
   NETLOG("Initial network state synced: action_seed=%u, input_lag=%d", game.action_random_seed, game.input_lag_turns);
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
