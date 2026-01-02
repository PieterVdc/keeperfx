/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file bflib_network_exchange.cpp
 *     Network data exchange for Dungeon Keeper multiplayer.
 * @par Purpose:
 *     Network data exchange routines for multiplayer games.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     11 Apr 2009 - 13 May 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "bflib_network_exchange.h"
#include "bflib_network.h"
#include "bflib_network_internal.h"
#include "bflib_datetm.h"
#include "bflib_sound.h"
#include "globals.h"
#include "player_data.h"
#include "net_game.h"
#include "front_landview.h"
#include "net_received_packets.h"
#include "net_redundant_packets.h"
#include "game_legacy.h"
#include "packets.h"
#include "keeperfx.hpp"
#include "post_inc.h"

#ifdef __cplusplus
void gameplay_loop_draw();
extern "C" void network_yield_draw();
extern "C" long double last_draw_completed_time;
long double get_time_tick_ns();
#endif

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

#define NETWORK_FPS 60
#define NETWORK_WAIT_TIMEOUT 5000

char* InitMessageBuffer(enum NetMessageType msg_type) {
    char* ptr = netstate.msg_buffer;
    *ptr = msg_type;
    return ptr + 1;
}

void SendMessage(NetUserId dest, const char* end_ptr) {
    netstate.sp->sendmsg_single(dest, netstate.msg_buffer, end_ptr - netstate.msg_buffer);
}

void SendFrameToPeers(NetUserId source_id, const void * send_buf, size_t buf_size, int seq_nbr, enum NetMessageType msg_type) {

}

TbError ProcessMessage(NetUserId source, void* server_buf, size_t frame_size) {
    return Lb_OK;
}

TbError LbNetwork_ExchangeLogin(char *plyr_name) {
    NETMSG("Logging in as %s", plyr_name);
    if (1 + strlen(netstate.password) + 1 + strlen(plyr_name) + 1 >= sizeof(netstate.msg_buffer)) {
        ERRORLOG("Login credentials too long");
        return Lb_FAIL;
    }
    char * ptr = InitMessageBuffer(NETMSG_LOGIN);
    strcpy(ptr, netstate.password);
    ptr += strlen(netstate.password) + 1;
    strcpy(ptr, plyr_name);
    ptr += strlen(plyr_name) + 1;
    SendMessage(SERVER_ID, ptr);
    TbClockMSec start = LbTimerClock();
    while (true) {
        TbClockMSec elapsed = LbTimerClock() - start;
        if (elapsed >= 5000) {
            break;
        }
        unsigned wait_ms = (unsigned)(5000 - elapsed);
        if (!netstate.sp->msgready(SERVER_ID, wait_ms)) {
            break;
        }
        if (ProcessMessage(SERVER_ID, &net_screen_packet, sizeof(struct ScreenPacket)) == Lb_FAIL) {
            break;
        }
        if (netstate.msg_buffer[0] == NETMSG_LOGIN) {
            break;
        }
    }
    if (netstate.msg_buffer[0] != NETMSG_LOGIN) {
        fprintf(stderr, "Network login rejected");
        return Lb_FAIL;
    }
    ProcessMessage(SERVER_ID, &net_screen_packet, sizeof (struct ScreenPacket));
    if (netstate.my_id == INVALID_USER_ID) {
        fprintf(stderr, "Network login unsuccessful");
        return Lb_FAIL;
    }
    return Lb_OK;
}

void LbNetwork_WaitForMissingPackets(void* server_buf, size_t client_frame_size) {
    if (game.skip_initial_input_turns > 0) {
        return;
    }
    GameTurn historical_turn = game.play_gameturn - game.input_lag_turns;
    const struct Packet* received_packets = get_received_packets_for_turn(historical_turn);
    if (received_packets == NULL) {
        MULTIPLAYER_LOG("LbNetwork_WaitForMissingPackets: Missing packets for turn=%lu, waiting...", (unsigned long)historical_turn);
        TbClockMSec start = LbTimerClock();
        while (true) {
            int elapsed = LbTimerClock() - start;
            if (elapsed >= NETWORK_WAIT_TIMEOUT) {
                MULTIPLAYER_LOG("LbNetwork_WaitForMissingPackets: Timeout waiting for turn=%lu packets", (unsigned long)historical_turn);
                break;
            }

            NetUserId id;
            for (id = 0; id < netstate.max_players; id += 1) {
                if (id == netstate.my_id) { continue; }
                if (netstate.users[id].progress == USER_UNUSED) { continue; }
                if (my_player_number != get_host_player_id() && id != SERVER_ID) { continue; }

                int wait_time = NETWORK_WAIT_TIMEOUT - elapsed;
                if (netstate.sp->msgready(id, wait_time)) {
                    while (netstate.sp->msgready(id, 0)) {
                        ProcessMessage(id, server_buf, client_frame_size);
                    }
                }
            }

            received_packets = get_received_packets_for_turn(historical_turn);
            if (received_packets != NULL) {
                MULTIPLAYER_LOG("LbNetwork_WaitForMissingPackets: Successfully received packets for turn=%lu after %dms", (unsigned long)historical_turn, elapsed);
                break;
            }

            network_yield_draw();
        }
    }
}

TbError LbNetwork_Exchange(enum NetMessageType msg_type, void *send_buf, void *server_buf, size_t client_frame_size) {
    netstate.sp->update(OnNewUser);
    if (netstate.my_id < 0 || netstate.my_id >= netstate.max_players) {
        ERRORLOG("Critical error: Invalid my_id %i in LbNetwork_Exchange", netstate.my_id);
        abort();
    }
    memcpy(((char*)server_buf) + netstate.my_id * client_frame_size, send_buf, client_frame_size);
    SendFrameToPeers(netstate.my_id, send_buf, client_frame_size, netstate.seq_nbr, msg_type);
    NetUserId id;
    for (id = 0; id < netstate.max_players; id += 1) {
        if (id == netstate.my_id) { continue; }
        if (netstate.users[id].progress == USER_UNUSED) { continue; }
        if (my_player_number != get_host_player_id() && id != SERVER_ID) { continue; }
        if (msg_type == NETMSG_GAMEPLAY) {
            const long double draw_interval_nanoseconds = 1000000000.0 / NETWORK_FPS;
            const int timeout_max = (1000 / game_num_fps);
            TbClockMSec start = LbTimerClock();
            while (true) {
                int elapsed = LbTimerClock() - start;
                if (elapsed >= timeout_max) {break;}

                long long time_since_draw_nanoseconds = get_time_tick_ns() - last_draw_completed_time;
                int remaining_time_until_draw = (int)((draw_interval_nanoseconds - time_since_draw_nanoseconds) / 1000000.0);
                int wait = min(timeout_max - elapsed, remaining_time_until_draw);
                
                if (netstate.sp->msgready(id, wait)) {
                    TbBool received_gameplay_packet = false;
                    ProcessMessage(id, server_buf, client_frame_size);
                    if (netstate.msg_buffer[0] == NETMSG_GAMEPLAY) {received_gameplay_packet = true;}
                    // Process the rest too, for the case of multiple packets having arrived at once.
                    while (netstate.sp->msgready(id, 0)) {
                        ProcessMessage(id, server_buf, client_frame_size);
                        if (netstate.msg_buffer[0] == NETMSG_GAMEPLAY) {received_gameplay_packet = true;}
                    }
                    if (received_gameplay_packet == true) {break;}
                }

                if (LbTimerClock() - start < timeout_max) {
                    network_yield_draw();
                }
            }
        } else {
            if (netstate.sp->msgready(id, 10000)) {
                ProcessMessage(id, server_buf, client_frame_size);
            }
        }
    }
    netstate.seq_nbr += 1;
    return Lb_OK;
}

void LbNetwork_SendPauseImmediate(TbBool pause_state, unsigned long delay_milliseconds) {
    MULTIPLAYER_LOG("LbNetwork_SendPauseImmediate: sending pause_state=%d, delay_milliseconds=%lu", pause_state, delay_milliseconds);

    char* message_pointer = InitMessageBuffer(NETMSG_PAUSE);
    *message_pointer = pause_state;
    message_pointer += 1;
    *(unsigned long*)message_pointer = delay_milliseconds;
    message_pointer += sizeof(unsigned long);

    int message_size = message_pointer - netstate.msg_buffer;

    for (NetUserId id = 0; id < netstate.max_players; id += 1) {
        if (id == netstate.my_id || !IsUserActive(id)) {
            continue;
        }
        netstate.sp->sendmsg_single(id, netstate.msg_buffer, message_size);
    }
}

void LbNetwork_SendChatMessageImmediate(int player_id, const char *message) {
    char* ptr = InitMessageBuffer(NETMSG_CHATMESSAGE);
    *ptr = player_id;
    ptr += 1;
    strcpy(ptr, message);
    for (NetUserId id = 0; id < netstate.max_players; id += 1) {
        if (id != netstate.my_id && IsUserActive(id)) {
            netstate.sp->sendmsg_single(id, netstate.msg_buffer, 3 + strlen(message));
        }
    }
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
