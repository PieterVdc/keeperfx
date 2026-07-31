/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file scrcapt.c
 *     Screen capturing functions.
 * @par Purpose:
 *     Functions to read display buffer and store it in various formats.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     05 Jan 2009 - 12 Jan 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "globals.h"
#include "bflib_basics.h"
#include "bflib_fileio.h"
#include "bflib_sndlib.h"
#include "net_portforward.h"
#include "platform.h"
#include "engine_render.h"
#include "creature_graphics.h"
#include "steam_api.hpp"
#include "astronomy.h"
#include "json.h"
#include "value.h"
#include "lua_base.h"
#include "lua_triggers.h"
#include "lua_cfg_funcs.h"
#include "frontmenu_net.h"
#include "net_lobby.h"
#include "net_matchmaking.h"
#include "net_checksums.h"
#include "net_game.h"
#include "net_exchange_common.h"
#include "net_exchange_gameplay.h"
#include "front_network.h"
#include "front_landview.h"
#include "bflib_inputctrl.h"
#include "bflib_joyst.h"
#include "bflib_guibtns.h"
#include "post_inc.h"
/******************************************************************************/


//scrcapt.c functions
TbBool take_screenshot(char *fname){ return false; }
TbBool cumulative_screen_shot(void){ return false; }
TbBool movie_record_start(void){ return false; }
TbBool movie_record_stop(void){ return false; }
TbBool movie_record_frame(void){ return false; }
TbBool perform_any_screen_capturing(void){ return false; }

/******************************************************************************/
//bflib_fmvids.cpp functions
TbBool play_smk(const char *filename, int flags)
{
	(void)filename;
	(void)flags;
	return false;
}

short anim_stop(void)
{
	return 1;
}

short anim_record(void)
{
	return 0;
}

TbBool anim_record_frame(unsigned char *screenbuf, unsigned char *palette)
{
	(void)screenbuf;
	(void)palette;
	return false;
}

/******************************************************************************/
//cdrom.cpp functions
void SetRedbookVolume(SoundVolume value)
{
	(void)value;
}

TbBool PlayRedbookTrack(int track)
{
	(void)track;
	return false;
}

void PauseRedbookTrack(void)
{
}

void ResumeRedbookTrack(void)
{
}

void StopRedbookTrack(void)
{
}

/******************************************************************************/
//net_portforward.cpp functions
int port_forward_add_mapping(uint16_t port)
{
	(void)port;
	return 0;
}

void port_forward_remove_mapping(void)
{
}

/******************************************************************************/
// frontmenu_net.c / net_lobby.c stubs
TbBool frontnet_start_input(void)
{
	return false;
}

TbError LbNetwork_Stop(void)
{
	return Lb_OK;
}

void LbNetwork_InitSessionsFromCmdLine(const char *str)
{
	(void)str;
}

void LbNetwork_SetServerPort(int port)
{
	(void)port;
}

/******************************************************************************/
// custom sound stubs used by sound_manager.cpp
int custom_sound_bank_size(void)
{
	return 0;
}

TbBool custom_sound_load_wav(const char *filepath, int sample_id)
{
	(void)filepath;
	(void)sample_id;
	return false;
}

TbBool custom_sound_load_wav_mem(const unsigned char *data, size_t size, const char *logical_name, int sample_id)
{
	(void)data;
	(void)size;
	(void)logical_name;
	(void)sample_id;
	return false;
}

void custom_sound_bank_clear(void)
{
}

SoundSmplTblID get_custom_offset(void)
{
	return 0;
}

/******************************************************************************/
//centijson value stubs for toml_api.c
static VALUE wii_stub_value;
static const char wii_stub_empty_string[] = "";
struct TbFileFind { int dummy; };

struct TbSpriteSheet * gui_panel_sprites = NULL;
short td_to_fp_sprite_add[KEEPERSPRITE_ADD_NUM] = {0};
short fp_to_td_sprite_add[KEEPERSPRITE_ADD_NUM] = {0};
short iso_td_add[KEEPERSPRITE_ADD_NUM] = {0};
short td_iso_add[KEEPERSPRITE_ADD_NUM] = {0};
TbSpriteData keepersprite_add[KEEPERSPRITE_ADD_NUM] = {0};
struct KeeperSprite creature_table_add[KEEPERSPRITE_ADD_NUM] = {0};
TbBigChecksum required_sprite_zip_checksums[9] = {0};

int total_sprite_zip_count = 0;
short bad_icon_id = -1;

void show_ignored_fxdata_zip_messages(void)
{
}

void init_custom_sprites(LevelNumber level_no)
{
	(void)level_no;
}

short get_anim_id(const char *name, struct ObjectConfigStats* objst)
{
	(void)name;
	(void)objst;
	return -1;
}

short get_anim_id_(const char* name)
{
	(void)name;
	return -1;
}

short get_icon_id(const char *name)
{
	(void)name;
	return -1;
}

const struct TbSprite *get_button_sprite_for_player(short sprite_idx, PlayerNumber plyr_idx)
{
	(void)sprite_idx;
	(void)plyr_idx;
	return NULL;
}

const struct TbSprite *get_button_sprite(short sprite_idx)
{
	(void)sprite_idx;
	return NULL;
}

const struct TbSprite *get_frontend_sprite(short sprite_idx)
{
	(void)sprite_idx;
	return NULL;
}

const struct TbSprite *get_new_icon_sprite(short sprite_idx)
{
	(void)sprite_idx;
	return NULL;
}

const struct TbSprite *get_panel_sprite(short sprite_idx)
{
	(void)sprite_idx;
	return NULL;
}

int is_custom_icon(short icon_idx)
{
	(void)icon_idx;
	return 0;
}

const struct LensOverlayData* get_lens_overlay_data(const char *name)
{
	(void)name;
	return NULL;
}

const struct LensMistData* get_lens_mist_data(const char *name)
{
	(void)name;
	return NULL;
}

VALUE* value_array_get(const VALUE* v, size_t index)
{
	(void)v;
	(void)index;
	return &wii_stub_value;
}

VALUE* value_array_append(VALUE* v)
{
	(void)v;
	return &wii_stub_value;
}

VALUE* value_array_insert(VALUE* v, size_t index)
{
	(void)v;
	(void)index;
	return &wii_stub_value;
}

size_t value_array_size(const VALUE* v)
{
	(void)v;
	return 0;
}

VALUE* value_dict_get(const VALUE* v, const char* key)
{
	(void)v;
	(void)key;
	return &wii_stub_value;
}

VALUE* value_dict_add(VALUE* v, const char* key)
{
	(void)v;
	(void)key;
	return &wii_stub_value;
}

VALUE* value_dict_add_(VALUE* v, const char* key, size_t key_len)
{
	(void)v;
	(void)key;
	(void)key_len;
	return &wii_stub_value;
}

VALUE* value_dict_get_or_add(VALUE* v, const char* key)
{
	(void)v;
	(void)key;
	return &wii_stub_value;
}

int value_bool(const VALUE* v)
{
	(void)v;
	return 0;
}

int32_t value_int32(const VALUE* v)
{
	(void)v;
	return 0;
}

uint32_t value_uint32(const VALUE* v)
{
	(void)v;
	return 0;
}

int64_t value_int64(const VALUE* v)
{
	(void)v;
	return 0;
}

uint64_t value_uint64(const VALUE* v)
{
	(void)v;
	return 0;
}

float value_float(const VALUE* v)
{
	(void)v;
	return 0.0f;
}

double value_double(const VALUE* v)
{
	(void)v;
	return 0.0;
}

VALUE_TYPE value_type(const VALUE* v)
{
	(void)v;
	return VALUE_NULL;
}

const char* value_string(const VALUE* v)
{
	(void)v;
	return wii_stub_empty_string;
}

void value_fini(VALUE* v)
{
	(void)v;
}

int value_init_array(VALUE* v)
{
	(void)v;
	return 0;
}

int value_init_bool(VALUE* v, int b)
{
	(void)v;
	(void)b;
	return 0;
}

int value_init_dict(VALUE* v)
{
	(void)v;
	return 0;
}

int value_init_double(VALUE* v, double d)
{
	(void)v;
	(void)d;
	return 0;
}

int value_init_float(VALUE* v, float f)
{
	(void)v;
	(void)f;
	return 0;
}

int value_init_int32(VALUE* v, int32_t i32)
{
	(void)v;
	(void)i32;
	return 0;
}

int value_init_int64(VALUE* v, int64_t i64)
{
	(void)v;
	(void)i64;
	return 0;
}

void value_init_null(VALUE* v)
{
	(void)v;
}

int value_init_string_(VALUE* v, const char* str, size_t len)
{
	(void)v;
	(void)str;
	(void)len;
	return 0;
}

int value_init_uint32(VALUE* v, uint32_t u32)
{
	(void)v;
	(void)u32;
	return 0;
}

int value_init_uint64(VALUE* v, uint64_t u64)
{
	(void)v;
	(void)u64;
	return 0;
}

void json_analyze_number(const char* num, size_t num_size,
	int* p_is_int32_compatible,
	int* p_is_uint32_compatible,
	int* p_is_int64_compatible,
	int* p_is_uint64_compatible)
{
	(void)num;
	(void)num_size;
	if (p_is_int32_compatible) *p_is_int32_compatible = 0;
	if (p_is_uint32_compatible) *p_is_uint32_compatible = 0;
	if (p_is_int64_compatible) *p_is_int64_compatible = 0;
	if (p_is_uint64_compatible) *p_is_uint64_compatible = 0;
}

int32_t json_number_to_int32(const char* num, size_t num_size)
{
	(void)num;
	(void)num_size;
	return 0;
}

uint32_t json_number_to_uint32(const char* num, size_t num_size)
{
	(void)num;
	(void)num_size;
	return 0;
}

int64_t json_number_to_int64(const char* num, size_t num_size)
{
	(void)num;
	(void)num_size;
	return 0;
}

uint64_t json_number_to_uint64(const char* num, size_t num_size)
{
	(void)num;
	(void)num_size;
	return 0;
}

int json_number_to_double(const char* num, size_t num_size, double* p_result)
{
	(void)num;
	(void)num_size;
	if (p_result) {
		*p_result = 0.0;
	}
	return 0;
}

int json_dom_parse(char* in, size_t in_size, void* memctx, int flags, VALUE* out, JSON_INPUT_POS* pos)
{
	(void)in;
	(void)in_size;
	(void)memctx;
	(void)flags;
	if (out != NULL) {
		value_init_null(out);
	}
	if (pos != NULL) {
		pos->line_number = 0;
		pos->column_number = 0;
	}
	return -1;
}

int json_dom_dump(const VALUE* root, JSON_DUMP_CALLBACK writer, void* userdata, int indent, int flags)
{
	(void)root;
	(void)indent;
	(void)flags;
	if (writer != NULL) {
		return writer("{}", 2, userdata);
	}
	return 0;
}

/******************************************************************************/
//platform.cpp functions
const char * get_os_version(void)
{
	return "Wii";
}

const void * get_image_base(void)
{
	return NULL;
}

const char * get_wine_version(void)
{
	return NULL;
}

const char * get_wine_host(void)
{
	return NULL;
}

/******************************************************************************/
//steam_api.cpp functions
int steam_api_init()
{
	return 0;
}

void steam_api_shutdown()
{
}

/******************************************************************************/
//bflib_fileio platform functions
struct TbFileFind * LbFileFindFirst(const char * filespec, struct TbFileEntry * fentry)
{
	(void)filespec;
	(void)fentry;
	return NULL;
}

int LbFileFindNext(struct TbFileFind * ffind, struct TbFileEntry * fentry)
{
	(void)ffind;
	(void)fentry;
	return -1;
}

void LbFileFindEnd(struct TbFileFind * ffind)
{
	(void)ffind;
}

/******************************************************************************/
//astronomy functions
astro_time_t Astronomy_CurrentTime(void)
{
	astro_time_t result = {0};
	return result;
}

astro_angle_result_t Astronomy_MoonPhase(astro_time_t time)
{
	(void)time;
	astro_angle_result_t result = {0};
	return result;
}

/******************************************************************************/
//api.c functions
void api_check_var_update(){}
void api_event(const char *event_name){}
void api_update_server(){}
void api_close_server(){}
int api_init_server(){ return 0; }

/******************************************************************************/
//net_resync.cpp functions
TbBool detailed_multiplayer_logging = false;

void resync_game(void) {}
void LbNetwork_TimesyncBarrier(void) {}

/******************************************************************************/

//net_checksums.c functions
short checksums_different(void) { return 0; }
void update_turn_checksums(void) {}
void pack_desync_history_for_resync(void) {}
void compare_desync_history_from_host(void) {}

/******************************************************************************/
//bflib_dialog.c functions
short warning_dialog(const char *codefile,const int ecode,const char *message)
{
	LbWarnLog("In source %s:\n %5d - %s\n", codefile, ecode, message);
	return 1;
}

short error_dialog(const char *codefile,const int ecode,const char *message)
{
	LbErrorLog("In source %s:\n %5d - %s\n", codefile, ecode, message);
	return 0;
}

short error_dialog_fatal(const char *codefile,const int ecode,const char *message)
{
	LbErrorLog("In source %s:\n %5d - %s\n", codefile, ecode, message);
	return 0;
}

/******************************************************************************/
//lua_* functions
struct lua_State *Lvl_script = NULL;

TbBool CheckLua(lua_State *L, int result,const char* func)
{
	(void)L;
	(void)result;
	(void)func;
	return true;
}

TbBool open_lua_script(LevelNumber lvnum)
{
	(void)lvnum;
	return false;
}

void close_lua_script(void)
{
}

const char* get_lua_serialized_data(size_t *len)
{
	if (len != NULL) {
		*len = 0;
	}
	return NULL;
}

void set_lua_serialized_data(const char* data, size_t len)
{
	(void)data;
	(void)len;
}

TbBool execute_lua_code_from_console(const char* code)
{
	(void)code;
	return false;
}

TbBool execute_lua_code_from_script(const char* code)
{
	(void)code;
	return false;
}

const char* lua_get_serialised_data(size_t *len)
{
	if (len != NULL) {
		*len = 0;
	}
	return NULL;
}

TbBool lua_set_serialised_data(const char *data, size_t len)
{
	(void)data;
	(void)len;
	return false;
}

void cleanup_serialized_data(void)
{
}

void lua_set_random_seed(unsigned int seed)
{
	(void)seed;
}

void generate_lua_types_file(void)
{
}

void lua_on_chatmsg(PlayerNumber plyr_idx, char *msg)
{
	(void)plyr_idx;
	(void)msg;
}

void lua_on_game_start(void)
{
}

void lua_on_game_tick(void)
{
}

void lua_on_power_cast(PlayerNumber plyr_idx, PowerKind pwkind,unsigned short splevel, MapSubtlCoord stl_x, MapSubtlCoord stl_y, struct Thing *thing)
{
	(void)plyr_idx;
	(void)pwkind;
	(void)splevel;
	(void)stl_x;
	(void)stl_y;
	(void)thing;
}

void lua_on_special_box_activate(PlayerNumber plyr_idx, struct Thing *cratetng)
{
	(void)plyr_idx;
	(void)cratetng;
}

void lua_on_dungeon_destroyed(PlayerNumber plyr_idx)
{
	(void)plyr_idx;
}

void lua_on_creature_death(struct Thing *crtng)
{
	(void)crtng;
}

void lua_on_creature_rebirth(struct Thing* crtng)
{
	(void)crtng;
}

void lua_on_trap_placed(struct Thing *traptng)
{
	(void)traptng;
}

void lua_on_apply_damage_to_thing(struct Thing *thing, HitPoints dmg, PlayerNumber dealing_plyr_idx)
{
	(void)thing;
	(void)dmg;
	(void)dealing_plyr_idx;
}

void lua_on_level_up(struct Thing *thing)
{
	(void)thing;
}

void lua_on_slab_kind_change(MapSlabCoord slb_x, MapSlabCoord slb_y, SlabKind old_slab)
{
	(void)slb_x;
	(void)slb_y;
	(void)old_slab;
}

void lua_on_slab_owner_change(MapSlabCoord slb_x, MapSlabCoord slb_y, PlayerNumber old_owner)
{
	(void)slb_x;
	(void)slb_y;
	(void)old_owner;
}

void lua_on_room_owner_change(struct Room *room, PlayerNumber old_owner)
{
	(void)room;
	(void)old_owner;
}

FuncIdx get_function_idx(const char *func_name,const struct NamedCommand * Cfuncs)
{
	(void)func_name;
	(void)Cfuncs;
	return 0;
}

TbResult luafunc_magic_use_power(FuncIdx func_idx, PlayerNumber plyr_idx, PowerKind pwkind,
	unsigned short splevel, MapSubtlCoord stl_x, MapSubtlCoord stl_y, struct Thing *thing, unsigned long allow_flags)
{
	(void)func_idx;
	(void)plyr_idx;
	(void)pwkind;
	(void)splevel;
	(void)stl_x;
	(void)stl_y;
	(void)thing;
	(void)allow_flags;
	return Lb_SUCCESS;
}

short luafunc_crstate_func(FuncIdx func_idx,struct Thing *thing)
{
	(void)func_idx;
	(void)thing;
	return 1;
}

short luafunc_thing_update_func(FuncIdx func_idx,struct Thing *thing)
{
	(void)func_idx;
	(void)thing;
	return 1;
}

void *iconv_open(const char *tocode, const char *fromcode)
{
	(void)tocode;
	(void)fromcode;
	return (void *)1;
}

size_t iconv(void *cd, const char **inbuf, size_t *inbytesleft, char **outbuf, size_t *outbytesleft)
{
	(void)cd;
	if (inbuf == NULL || inbytesleft == NULL || outbuf == NULL || outbytesleft == NULL) {
		return (size_t)-1;
	}
	while (*inbytesleft > 0 && *outbytesleft > 0) {
		**outbuf = **inbuf;
		(*outbuf)++;
		(*inbuf)++;
		(*inbytesleft)--;
		(*outbytesleft)--;
	}
	return 0;
}

int iconv_close(void *cd)
{
	(void)cd;
	return 0;
}

/******************************************************************************/
// Additional Wii link stubs
struct GuiMenu frontend_net_service_menu = {0};
struct GuiMenu frontend_net_session_menu = {0};
struct GuiMenu frontend_net_start_menu = {0};
struct GuiMenu frontend_add_session_box = {0};

void frontnet_session_create(struct GuiButton *gbtn)
{
	(void)gbtn;
}

void frontnet_session_join(struct GuiButton *gbtn)
{
	(void)gbtn;
}

TbError LbNetwork_ExchangeFrontend(void *send_buf, void *server_buf, size_t frame_size)
{
	(void)send_buf;
	(void)server_buf;
	(void)frame_size;
	return Lb_OK;
}

TbError LbNetwork_EnableNewPlayers(TbBool allow)
{
	(void)allow;
	return Lb_OK;
}

TbError LbNetwork_EnumeratePlayers(struct TbNetworkSessionNameEntry *sesn, TbNetworkCallbackFunc callback, void *user_data)
{
	(void)sesn;
	(void)callback;
	(void)user_data;
	return Lb_OK;
}

TbError LbNetwork_EnumerateSessions(TbNetworkCallbackFunc callback, void *ptr)
{
	(void)callback;
	(void)ptr;
	return Lb_OK;
}


SoundSmplTblID get_speech_offset(void)
{
	return 0;
}

void sound_register_id_redirect(SoundSmplTblID from_id, SoundSmplTblID to_id)
{
	(void)from_id;
	(void)to_id;
}

void sound_register_stack_policy(SoundSmplTblID smptbl_id, unsigned char mode, short max_instances)
{
	(void)smptbl_id;
	(void)mode;
	(void)max_instances;
}

void sound_save_id_redirect_snapshot(void)
{
}

void sound_restore_id_redirect_snapshot(void)
{
}

TbBool play_music_fgroup(short fgroup, const char *fname)
{
	(void)fgroup;
	(void)fname;
	return false;
}

void intentional_desync(void)
{
}

void lua_on_object_destroyed(struct Thing *objtng)
{
	(void)objtng;
}

void lua_on_pick_up(struct Thing *thing, PlayerNumber plyr_idx)
{
	(void)thing;
	(void)plyr_idx;
}

void lua_on_slap(struct Thing *thing, PlayerNumber plyr_idx)
{
	(void)thing;
	(void)plyr_idx;
}

void lua_on_shot_hit(struct Thing *shot, struct Thing *shooter, struct Thing *target, MapSubtlCoord next_stl_x, MapSubtlCoord next_stl_y, bool rebound_hit)
{
	(void)shot;
	(void)shooter;
	(void)target;
	(void)next_stl_x;
	(void)next_stl_y;
	(void)rebound_hit;
}

short luafunc_trap_activation_func(FuncIdx func_idx, struct Thing *trap, struct Thing *creature)
{
	(void)func_idx;
	(void)trap;
	(void)creature;
	return 1;
}

short luafunc_shot_hit_thing_func(FuncIdx func_idx, struct Thing *shot, struct Thing *shooter, struct Thing *target, MapSubtlCoord next_stl_x, MapSubtlCoord next_stl_y)
{
	(void)func_idx;
	(void)shot;
	(void)shooter;
	(void)target;
	(void)next_stl_x;
	(void)next_stl_y;
	return 1;
}

// Network fully disabled on Wii build
struct NetState netstate = {0};

int net_service_index_selected = FrontendNetSvc_Skirmish;
struct TbNetworkSessionNameEntry *net_session[SESSION_ENTRIES_COUNT] = {0};
long net_number_of_sessions = 0;
long net_session_index_active = -1;
struct TbNetworkPlayerName net_player[MAX_NET_USERS] = {0};
struct ConfigInfo net_config_info = {0};
char net_service[16][NET_SERVICE_LEN] = {{0}};
char net_player_name[20] = {0};
char tmp_net_player_name[24] = {0};
long fe_net_level_selected = SINGLEPLAYER_NOTSTARTED;

TbBool frontnet_service_selected(enum FrontendNetService service)
{
	(void)service;
	return false;
}

void enum_sessions_callback(struct TbNetworkCallbackData *netcdat, void *ptr)
{
	(void)netcdat;
	(void)ptr;
}

void setup_alliances(void) {}
void frontnet_service_setup(void) {}
void frontnet_session_setup(void) {}
void frontnet_start_setup(void) {}
void frontnet_service_update(void) {}
void frontnet_session_update(void) {}
void frontnet_start_update(void) {}

void frontnet_send_campaign_change_message(const char *campaign_fname)
{
	(void)campaign_fname;
}

void frontnetmap_unload(void) {}
TbBool frontnetmap_load(void) { return false; }
void frontnetmap_input(void) {}
void frontnetmap_draw(void) {}
TbBool frontnetmap_update(void) { return false; }

short setup_network_service(enum FrontendNetService service)
{
	(void)service;
	return false;
}

int setup_old_network_service(void)
{
	return false;
}

TbBool init_players_network_game(void)
{
	return false;
}

void setup_count_players(void) {}
void are_disconnect_victories_allowed(void) {}
TbBool network_human_contenders_remain(void) { return false; }
void process_player_leave_game_packet(struct PlayerInfo *player) { (void)player; }
void process_disconnected_network_players(void) {}
void sync_initial_network_seed(void) {}
unsigned long get_host_player_id(void) { return my_player_number; }

void initialize_packet_history(void) {}
void store_packet_history(PlayerNumber player, const struct Packet *packet) { (void)player; (void)packet; }
const struct Packet *get_history_packet(PlayerNumber player, GameTurn turn) { (void)player; (void)turn; return NULL; }
const struct Packet *get_latest_history_packet(PlayerNumber player) { (void)player; return NULL; }
void network_update(void *server_buf, size_t frame_size) { (void)server_buf; (void)frame_size; }
TbError LbNetwork_ExchangeGameplay(void *send_buf, void *server_buf, size_t frame_size)
{
	(void)send_buf;
	(void)server_buf;
	(void)frame_size;
	return Lb_OK;
}
void LbNetwork_BroadcastUnpause(void) {}
void process_gameplay_chat_message(int player_id, const char *message) { (void)player_id; (void)message; }
void send_network_chat_message(int player_id, const char *message) { (void)player_id; (void)message; }
void wait_for_all_players(void) {}