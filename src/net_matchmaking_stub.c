/******************************************************************************/
// Stub implementation for ARM builds without WebSocket support
/******************************************************************************/
#include "net_matchmaking.h"
#include <string.h>

struct TbNetworkSessionNameEntry matchmaking_sessions[MATCHMAKING_SESSIONS_MAX];
int matchmaking_session_count = 0;
char join_lobby_id[MATCHMAKING_ID_MAX] = "";

void matchmaking_connect_async(void) {
    // Stub: matchmaking disabled on this platform
}

int matchmaking_connect(void) {
    return -1;  // Failure
}

int matchmaking_request_list(void) {
    matchmaking_session_count = 0;
    return -1;
}

void matchmaking_disconnect(void) {
    // Stub
}

void matchmaking_refresh_sessions(void) {
    matchmaking_session_count = 0;
}

int matchmaking_create(const char *name, int udp_ipv4_port, int udp_ipv6_port) {
    (void)name;
    (void)udp_ipv4_port;
    (void)udp_ipv6_port;
    return -1;
}

int matchmaking_punch(const char *lobby_id, int udp_ipv4_port, int udp_ipv6_port, PunchAddresses *output) {
    (void)lobby_id;
    (void)udp_ipv4_port;
    (void)udp_ipv6_port;
    (void)output;
    return -1;
}

int matchmaking_poll_punch(PunchAddresses *output) {
    (void)output;
    return -1;
}
