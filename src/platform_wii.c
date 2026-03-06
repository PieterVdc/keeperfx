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
#include "steam_api.hpp"
#include "astronomy.h"
#include "json.h"
#include "value.h"
#include "lua_base.h"
#include "lua_triggers.h"
#include "lua_cfg_funcs.h"
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
//centijson value stubs for toml_api.c
static VALUE wii_stub_value;
static const char wii_stub_empty_string[] = "";
struct TbFileFind { int dummy; };

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

void lua_set_serialised_data(const char *data, size_t len)
{
	(void)data;
	(void)len;
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