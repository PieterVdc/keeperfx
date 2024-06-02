#include "pre_inc.h"

#include "../deps/luajit/src/lua.h"
#include "../deps/luajit/src/lauxlib.h"
#include "../deps/luajit/src/lualib.h"

#include "bflib_basics.h"
#include "globals.h"
#include "thing_data.h"
#include "creature_states.h"
#include "gui_msgs.h"
#include "thing_navigate.h"
#include "map_data.h"
#include "game_legacy.h"
#include "player_utils.h"
#include "lvl_script_lib.h"
#include "room_library.h"
#include "keeperfx.hpp"
#include "music_player.h"
#include "lua_base.h"


#include "post_inc.h"

/**********************************************/
static int thing_set_field(lua_State *L);
static int thing_get_field(lua_State *L);

static const struct luaL_Reg thing_methods[];

struct PlayerRange
{
    PlayerNumber start_idx;
    PlayerNumber end_idx;
};

/***************************************************************************************************/
/************    Inputs   **************************************************************************/
/***************************************************************************************************/



static long luaL_checkNamedCommand(lua_State *L, int index,const struct NamedCommand * commanddesc)
{
    if (lua_isnumber(L, index))
    {
        return lua_tointeger(L, index);
    }
    else if(lua_isstring(L, index))
    {
        const char* text = lua_tostring(L, index);
        long id = get_rid(commanddesc, text);

        luaL_argcheck(L,id != -1,index,"invalid namedcommandoption");

        return id;
    }
    luaL_error(L,"invalid namedcommandoption");
    return 0;

}
static struct Thing *luaL_checkThing(lua_State *L, int index)
{
    if (!lua_istable(L, index)) {
        luaL_error(L, "Expected a table");
        return NULL;
    }

    // Get idx field
    lua_getfield(L, index, "ThingIndex");
    if (!lua_isnumber(L, -1)) {
        luaL_error(L, "Expected 'index' to be an integer");
        return NULL;
    }
    int idx = lua_tointeger(L, -1);
    lua_pop(L, 1);  // Pop the idx value off the stack

    // Get creation_turn field
    lua_getfield(L, index, "creation_turn");
    if (!lua_isnumber(L, -1)) {
        luaL_error(L, "Expected 'creation_turn' to be an integer");
        return NULL;
    }
    int creation_turn = lua_tointeger(L, -1);
    lua_pop(L, 1);  // Pop the creation_turn value off the stack

    struct Thing* thing = thing_get(idx);
    if (thing_is_invalid(thing) || thing->creation_turn != creation_turn) {
        luaL_error(L, "Failed to resolve thing");
        return NULL;
    }
    return thing;
}

static TbMapLocation luaL_checkLocation(lua_State *L, int index)
{
    if (lua_istable(L, index)) {
        lua_getfield(L, index, "stl_x");
        int stl_x = lua_tointeger(L, -1);
        lua_getfield(L, index, "stl_y");
        int stl_y = lua_tointeger(L, -1);

        return get_coord_encoded_location(stl_x,stl_y);
    }

    const char* locname = lua_tostring(L, index);
    TbMapLocation location;
    if(!get_map_location_id(locname, &location))
    {
        luaL_error (L,"Invalid location, '%s'", locname);
    }
    return location;
}

static TbMapLocation luaL_optLocation(lua_State *L, int index)
{
    if (lua_isnone(L,index))
        return 0;
    else
        return luaL_checkLocation(L,index);
}

static TbMapLocation luaL_checkHeadingLocation(lua_State *L, int index)
{
    const char* locname = lua_tostring(L, index);
    long target = luaL_checkNamedCommand(L, index + 1,head_for_desc);


    TbMapLocation location;
    if(!get_map_heading_id(locname, target, &location))
    {
        luaL_error (L,"Invalid location, '%s'", locname);
    }
    return location;
}


static struct PlayerRange luaL_checkPlayerRange(lua_State *L, int index)
{
    struct PlayerRange playerRange = {0,0};
    if (lua_istable(L, index))
    {
        lua_getfield(L, index, "playerId");
        if (lua_isnumber(L, -1)) {
            int i = lua_tointeger(L, -1);
            playerRange.start_idx = i;
            playerRange.end_idx   = i;
            return playerRange;
        }
        luaL_error(L, "Expected table to be of class Player");
        return playerRange;
    }

    const char* plrname = lua_tostring(L, index);

    long plr_range_id = get_id(player_desc, plrname);
    if (plr_range_id == ALL_PLAYERS)
    {
        playerRange.start_idx = 0;
        playerRange.end_idx = PLAYERS_COUNT;
    }
    else
    {
        playerRange.start_idx = plr_range_id;
        playerRange.end_idx   = plr_range_id;
    }

    return playerRange;
}

static PlayerNumber luaL_checkPlayerSingle(lua_State *L, int index)
{
    

    struct PlayerRange playerRange = luaL_checkPlayerRange(L,index);
    if(playerRange.start_idx != playerRange.end_idx)
    {
        luaL_error (L,"player range not supported for this command");
    }
    return playerRange.start_idx;
}

static MapSubtlCoord luaL_checkstl_x(lua_State *L, int index)
{
    MapSubtlCoord stl_x = luaL_checkint(L,index);
    luaL_argcheck(L, 0 <= stl_x && stl_x <= gameadd.map_subtiles_x, index,
                       "x subtile coord out of range");
    return stl_x;
}

static MapSubtlCoord luaL_checkstl_y(lua_State *L, int index)
{
    MapSubtlCoord stl_y = luaL_checkint(L,index);
    luaL_argcheck(L, 0 <= stl_y && stl_y <= gameadd.map_subtiles_y, index,
                       "y subtile coord out of range");
    return stl_y;
}

static ActionPointId luaL_checkActionPoint(lua_State *L, int index)
{
    int apt_num = luaL_checkint(L,index);
    ActionPointId apt_idx = action_point_number_to_index(apt_num);
    if (!action_point_exists_idx(apt_idx))
    {
        luaL_error(L,"Non-existing Action Point, no %d", apt_num);
        return 0;
    }
    return apt_idx;
}




/***************************************************************************************************/
/************    Outputs   *************************************************************************/
/***************************************************************************************************/

void lua_pushThing(lua_State *L, struct Thing* thing) {
    if (thing_is_invalid(thing)) {
        luaL_error(L,"Attempt to push invalid thing to Lua");
        lua_pushnil(L);
        return;
    }

    lua_createtable(L, 0, 2);

    lua_pushinteger(L, thing->index);
    lua_setfield(L, -2, "ThingIndex");

    lua_pushinteger(L, thing->creation_turn);
    lua_setfield(L, -2, "creation_turn");

    luaL_getmetatable(L, "Thing");
    lua_setmetatable(L, -2);
}

void lua_pushPlayer(lua_State *L, PlayerNumber plr_idx) {


    lua_createtable(L, 0, 2);

    lua_pushinteger(L, plr_idx);
    lua_setfield(L, -2, "playerId");

    luaL_getmetatable(L, "Player");
    lua_setmetatable(L, -2);
}

void lua_pushPos(lua_State *L, struct Coord3d* pos) {


    lua_createtable(L, 0, 2);

    lua_pushinteger(L, pos->x.stl.num);
    lua_setfield(L, -2, "stl_x");

    lua_pushinteger(L,  pos->y.stl.num);
    lua_setfield(L, -2, "stl_y");

    //luaL_getmetatable(L, "Thing");
    //lua_setmetatable(L, -2);
}

/***************************************************************************************************/
/************    Api Functions    ******************************************************************/
/***************************************************************************************************/

static int lua_CREATE_PARTY(lua_State *L)
{
    const char* party_name = luaL_checklstring(L, 1,NULL);
    create_party(party_name);
    return 0;
}
static int lua_ADD_TO_PARTY(lua_State *L)
{
    const char* party_name = luaL_checkstring(L,  1);
    long crtr_id           = luaL_checkNamedCommand(L,2,creature_desc);
    long experience        = luaL_checklong(L, 3);
    long gold              = luaL_checklong(L, 4);
    long objective_id      = luaL_checkNamedCommand(L, 5,hero_objective_desc);
    long countdown         = luaL_checklong (L, 6);

    int party_id = get_party_index_of_name(party_name);
    if (party_id < 0)
    {
        SCRPTERRLOG("Invalid Party:%s",party_name);
        return 0;
    }
    if ((experience < 1) || (experience > CREATURE_MAX_LEVEL))
    {
      SCRPTERRLOG("Invalid Creature Level parameter; %ld not in range (%d,%d)",experience,1,CREATURE_MAX_LEVEL);
      return 0;
    }

    add_member_to_party(party_id, crtr_id, experience, gold, objective_id, countdown);
    return 0;
}
static int lua_DELETE_FROM_PARTY(lua_State *L)
{
    const char* party_name = lua_tostring(L,  1);
    const char* creature   = lua_tostring(L,  2);
    long experience  = lua_tointeger(L, 3);

    int party_id = get_party_index_of_name(party_name);
    if (party_id < 0)
    {
        SCRPTERRLOG("Invalid Party:%s",party_name);
        return 0;
    }
    long creature_id = get_rid(creature_desc, creature);
    if (creature_id == -1)
    {
      SCRPTERRLOG("Unknown creature, '%s'", creature);
      return 0;
    }

    delete_member_from_party(party_id, creature_id, experience);
    return 0;
}
static int lua_ADD_PARTY_TO_LEVEL(lua_State *L)
{
    long plr_range_id      = luaL_checkinteger(L, 1);
    const char *prtname    = luaL_checkstring(L,  2);
    TbMapLocation location = luaL_checkLocation(L,  3);
    long ncopies           = luaL_checkinteger(L, 4);

    if (ncopies < 1)
    {
        SCRPTERRLOG("Invalid NUMBER parameter");
        return 0;
    }
    // Verify player
    long plr_id = get_players_range_single(plr_range_id);
    if (plr_id < 0) {
        SCRPTERRLOG("Given owning player is not supported in this command");
        return 0;
    }
    // Recognize place where party is created
    if (location == 0)
        return 0;
    // Recognize party name
    long prty_id = get_party_index_of_name(prtname);
    if (prty_id < 0)
    {
        SCRPTERRLOG("Party of requested name, '%s', is not defined",prtname);
        return 0;
    }
    struct Party* party = &gameadd.script.creature_partys[prty_id];
    script_process_new_party(party, plr_id, location, ncopies);
        return 0;
}

static int lua_ADD_CREATURE_TO_LEVEL(lua_State *L)
{
    PlayerNumber plr_idx   = luaL_checkPlayerSingle(L, 1);
    long crtr_id           = luaL_checkNamedCommand(L,2,creature_desc);
    TbMapLocation location = luaL_checkLocation(L,  3);
    long ncopies           = luaL_checkinteger(L, 4);
    long crtr_level        = luaL_checkinteger(L, 5);
    long carried_gold      = luaL_checkinteger(L, 6);

    if ((crtr_level < 1) || (crtr_level > CREATURE_MAX_LEVEL))
    {
        SCRPTERRLOG("Invalid CREATURE LEVEL parameter");
        return 0;
    }
    if ((ncopies <= 0) || (ncopies >= CREATURES_COUNT))
    {
        SCRPTERRLOG("Invalid number of creatures to add");
        return 0;
    }

    // Recognize place where party is created
    if (location == 0)
        return 0;

    for (long i = 0; i < ncopies; i++)
    {
        lua_pushThing(L,script_create_new_creature(plr_idx, crtr_id, location, carried_gold, crtr_level-1));
    }
    return ncopies;
}

static int lua_ADD_OBJECT_TO_LEVEL(lua_State *L)
{
    long obj_id            = luaL_checkNamedCommand(L,1,object_desc);
    TbMapLocation location = luaL_checkLocation(L,  2);
    long arg               = lua_tointeger(L,3);
    PlayerNumber plr_idx   = luaL_checkPlayerSingle(L, 4);

    lua_pushThing(L,script_process_new_object(obj_id, location, arg, plr_idx));
    return 1;
}

static int lua_SET_GENERATE_SPEED(lua_State *L)
{
    GameTurnDelta interval   = luaL_checkinteger(L,1);

    game.generate_speed = saturate_set_unsigned(interval, 16);
    update_dungeon_generation_speeds();
    return 0;
}

static int lua_START_MONEY(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    GoldAmount gold_val = luaL_checkinteger(L, 2);

    for (PlayerNumber i = player_range.start_idx; i <= player_range.end_idx; i++)
    {
        if (gold_val > SENSIBLE_GOLD)
        {
            gold_val = SENSIBLE_GOLD;
            SCRPTWRNLOG("Gold added to player reduced to %d", SENSIBLE_GOLD);
        }
        player_add_offmap_gold(i, gold_val);
    }
    return 0;

}

static int lua_ROOM_AVAILABLE(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    long rkind                      = luaL_checkNamedCommand(L,2,room_desc);
    TbBool can_be_available         = lua_toboolean(L, 3);
    TbBool is_available             = lua_toboolean(L, 4);

    for (PlayerNumber i = player_range.start_idx; i <= player_range.end_idx; i++)
    {
        set_room_available(i,rkind,can_be_available,is_available);
    }
    return 0;
}
static int lua_CREATURE_AVAILABLE(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    long cr_kind                    = luaL_checkNamedCommand(L,2,creature_desc);
    TbBool can_be_attracted         = lua_toboolean(L, 3);
    long amount_forced              = luaL_checkinteger(L, 4);

    for (PlayerNumber i = player_range.start_idx; i <= player_range.end_idx; i++)
    {
        if (!set_creature_available(i,cr_kind,can_be_attracted,amount_forced))
            WARNLOG("Setting creature %s availability for player %d failed.",creature_code_name(cr_kind),(int)i);
    }
    return 0;
}
static int lua_MAGIC_AVAILABLE(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    long power                      = luaL_checkNamedCommand(L,2,power_desc);
    TbBool can_be_available         = lua_toboolean(L, 3);
    TbBool is_available             = lua_toboolean(L, 4);

    JUSTLOG("lua_MAGIC_AVAILABLE %d;%d0",player_range.start_idx, player_range.end_idx); 

    for (PlayerNumber i = player_range.start_idx; i <= player_range.end_idx; i++)
    {
        JUSTLOG("%d,%d;%d;%d",i,power,can_be_available,is_available);
        set_power_available(i,power,can_be_available,is_available);
    }
    return 0;
}
static int lua_TRAP_AVAILABLE(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    long trap_type                  = luaL_checkNamedCommand(L,2,trap_desc);
    TbBool can_be_available         = lua_toboolean(L, 3);
    long number_available           = luaL_checkinteger(L, 4);

    for (PlayerNumber i = player_range.start_idx; i <= player_range.end_idx; i++)
    {
        set_trap_buildable_and_add_to_amount(i, trap_type, can_be_available, number_available);
    }
    return 0;
}
static int lua_RESEARCH(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    long research_type              = luaL_checkNamedCommand(L,2,research_desc);
    int room_or_spell;
    switch (research_type)
    {
        case 1:
            room_or_spell = luaL_checkNamedCommand(L,3,power_desc);
            break;
        case 2:
            room_or_spell = luaL_checkNamedCommand(L,3,room_desc);
            break;
        default:
            luaL_error (L,"invalid research_type %d",research_type);
            return 0;
    }
    long research_value         = luaL_checkint(L, 4);

    for (PlayerNumber i = player_range.start_idx; i <= player_range.end_idx; i++)
    {
        update_or_add_players_research_amount(i, research_type, room_or_spell, research_value);
    }
    return 0;
}
static int lua_RESEARCH_ORDER(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    long research_type              = luaL_checkNamedCommand(L,2,research_desc);
    int room_or_spell;
    switch (research_type)
    {
        case 1:
            room_or_spell = luaL_checkNamedCommand(L,3,power_desc);
            break;
        case 2:
            room_or_spell = luaL_checkNamedCommand(L,3,room_desc);
            break;
        default:
            luaL_error (L,"invalid research_type %d",research_type);
            return 0;
    }
    long research_value         = luaL_checkint(L, 4);

    for (PlayerNumber i = player_range.start_idx; i <= player_range.end_idx; i++)
    {
        if (!research_overriden_for_player(i))
            remove_all_research_from_player(i);
        add_research_to_player(i, research_type, room_or_spell, research_value);
    }
    return 0;
}

static int lua_COMPUTER_PLAYER(lua_State *L)
{
    PlayerNumber player = luaL_checkPlayerSingle(L,1);
    long attitude       = luaL_checkint(L,2);

    script_support_setup_player_as_computer_keeper(player, attitude);
    return 0;
}

static int lua_SET_TIMER(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    long timr_id              = luaL_checkNamedCommand(L,2,timer_desc);

    for (PlayerNumber i = player_range.start_idx; i <= player_range.end_idx; i++)
    {
        restart_script_timer(i, timr_id);
    }
    return 0;
}

static int lua_ADD_TUNNELLER_TO_LEVEL(lua_State *L)
{
    PlayerNumber plr_id          = luaL_checkPlayerSingle(L,1);
    TbMapLocation spawn_location = luaL_checkLocation(L,2);
    TbMapLocation head_for       = luaL_checkHeadingLocation(L,3);
    long level                   = luaL_checkinteger(L,5);
    long gold_held               = luaL_checkinteger(L,6);

    struct Thing* thing = script_process_new_tunneler(plr_id, spawn_location, head_for, level-1, gold_held);

    lua_pushThing(L, thing);
    return 1;
}

static int lua_WIN_GAME(lua_State *L)
{
    struct PlayerRange player_range;
    if(lua_isnone(L,1))
    {
        player_range.start_idx = 0;
        player_range.end_idx = PLAYERS_COUNT;
    }
    else
        player_range = luaL_checkPlayerRange(L, 1);


    for (PlayerNumber i = player_range.start_idx; i <= player_range.end_idx; i++)
    {
        struct PlayerInfo *player = get_player(i);
        set_player_as_won_level(player);
    }
    return 0;
}
static int lua_LOSE_GAME(lua_State *L)
{
    struct PlayerRange player_range;
    if(lua_isnone(L,1))
    {
        player_range.start_idx = 0;
        player_range.end_idx = PLAYERS_COUNT;
    }
    else
        player_range = luaL_checkPlayerRange(L, 1);

    for (PlayerNumber i = player_range.start_idx; i <= player_range.end_idx; i++)
    {
        struct PlayerInfo *player = get_player(i);
        set_player_as_lost_level(player);
    }
    return 0;
}

static int lua_MAX_CREATURES(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    long max_amount                 = luaL_checkinteger(L, 2);

    for (PlayerNumber i = player_range.start_idx; i <= player_range.end_idx; i++)
    {
        SYNCDBG(4,"Setting player %d max attracted creatures to %d.",(int)i,(int)max_amount);
        struct Dungeon* dungeon = get_dungeon(i);
        if (dungeon_invalid(dungeon))
            continue;
        dungeon->max_creatures_attracted = max_amount;
    }
    return 0;
}

static int lua_DOOR_AVAILABLE(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    long door_type                  = luaL_checkNamedCommand(L,2,door_desc);
    TbBool can_be_available         = lua_toboolean(L, 3);
    long number_available           = luaL_checkinteger(L, 4);

    for (PlayerNumber i = player_range.start_idx; i <= player_range.end_idx; i++)
    {
        set_door_buildable_and_add_to_amount(i, door_type, can_be_available, number_available);
    }
    return 0;
}

static int lua_DISPLAY_OBJECTIVE(lua_State *L)
{
    long msg_id    = luaL_checkinteger(L, 1);
    TbMapLocation zoom_location = luaL_optLocation(L,2);

    set_general_objective(msg_id,zoom_location,0,0);
    return 0;
}

static int lua_DISPLAY_OBJECTIVE_WITH_POS(lua_State *L)
{
    long msg_id   = luaL_checkinteger(L, 1);
    long stl_x    = luaL_checkstl_x(L, 2);
    long stl_y    = luaL_checkstl_y(L, 3);

    set_general_objective(msg_id,0,stl_x,stl_y);
    return 0;
}

static int lua_DISPLAY_INFORMATION(lua_State *L)
{
    long msg_id    = luaL_checkinteger(L, 1);
    TbMapLocation zoom_location = luaL_optLocation(L,2);

    set_general_information(msg_id,zoom_location,0,0);
    return 0;
}

static int lua_DISPLAY_INFORMATION_WITH_POS(lua_State *L)
{
    long msg_id    = luaL_checkinteger(L, 1);
    long stl_x    = luaL_checkstl_x(L, 2);
    long stl_y    = luaL_checkstl_y(L, 3);

    set_general_objective(msg_id,0,stl_x,stl_y);
    return 0;
}

static int lua_ADD_TUNNELLER_PARTY_TO_LEVEL(lua_State *L)
{
    PlayerNumber owner           = luaL_checkPlayerSingle(L, 1);
    const char *party_name       = luaL_checkstring(L,  2);
    TbMapLocation spawn_location = luaL_checkLocation(L,  3);
    TbMapLocation head_for       = luaL_checkHeadingLocation(L,4);
    long target                  = luaL_checkinteger(L, 5);
    long crtr_level              = luaL_checkinteger(L, 6);
    GoldAmount carried_gold      = luaL_checkinteger(L, 7);

    if ((crtr_level < 1) || (crtr_level > CREATURE_MAX_LEVEL))
    {
        SCRPTERRLOG("Invalid CREATURE LEVEL parameter");
        return 0;
    }
    // Recognize party name
    long prty_id = get_party_index_of_name(party_name);
    if (prty_id < 0)
    {
        SCRPTERRLOG("Party of requested name, '%s', is not defined", party_name);
        return 0;
    }
    struct Party* party = &gameadd.script.creature_partys[prty_id];
    if (party->members_num >= GROUP_MEMBERS_COUNT-1)
    {
        SCRPTERRLOG("Party too big for ADD_TUNNELLER (Max %d members)", GROUP_MEMBERS_COUNT-1);
        return 0;
    }

    script_process_new_tunneller_party(owner, prty_id, spawn_location, head_for, crtr_level-1, carried_gold);
    return 0;
}

static int lua_ADD_CREATURE_TO_POOL(lua_State *L)
{
    long crtr_model = luaL_checkNamedCommand(L,1,creature_desc);
    long amount     = luaL_checkinteger(L, 2);

    add_creature_to_pool(crtr_model, amount, false);
    return 0;
}

static int lua_RESET_ACTION_POINT(lua_State *L)
{
    ActionPointId apt_idx     = luaL_checkActionPoint(L, 1);
    PlayerNumber player_range_id = luaL_checkNamedCommand(L,2,player_desc);;

    action_point_reset_idx(apt_idx,player_range_id);
    return 0;
}

static int lua_SET_CREATURE_MAX_LEVEL(lua_State *L)
{

    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    long crtr_model                 = luaL_checkNamedCommand(L,2,creature_desc);
    long max_level                  = luaL_checkinteger(L, 3);

    for (PlayerNumber i = player_range.start_idx; i <= player_range.end_idx; i++)
    {
        struct Dungeon* dungeon = get_dungeon(i);
        if (dungeon_invalid(dungeon))
            continue;
        if (max_level == -1)
            max_level = CREATURE_MAX_LEVEL + 1;
        dungeon->creature_max_level[crtr_model%game.conf.crtr_conf.model_count] = max_level;
    }
    return 0;
}

static int lua_SET_MUSIC(lua_State *L)
{
    long track_number                  = luaL_checkinteger(L, 1);

    if (track_number >= FIRST_TRACK && track_number <= max_track)
    {
        game.audiotrack = track_number;
    }
    return 0;

}

static int lua_TUTORIAL_FLASH_BUTTON(lua_State *L)
{
    long          button    = luaL_checkinteger(L, 1);
    GameTurnDelta gameturns = luaL_checkinteger(L, 2);

    gui_set_button_flashing(button,gameturns);
    return 0;
}
/*

static int lua_SET_CREATURE_PROPERTY(lua_State *L)
{

}
static int lua_SET_COMPUTER_GLOBALS(lua_State *L)
static int lua_SET_COMPUTER_CHECKS(lua_State *L)
static int lua_SET_COMPUTER_EVENT(lua_State *L)
static int lua_SET_COMPUTER_PROCESS(lua_State *L)

/*
static int lua_ALLY_PLAYERS(lua_State *L)
static int lua_DEAD_CREATURES_RETURN_TO_POOL(lua_State *L)
static int lua_BONUS_LEVEL_TIME(lua_State *L)
static int lua_QUICK_OBJECTIVE(lua_State *L)
static int lua_QUICK_INFORMATION(lua_State *L)
static int lua_QUICK_OBJECTIVE_WITH_POS(lua_State *L)
static int lua_QUICK_INFORMATION_WITH_POS(lua_State *L)
static int lua_SWAP_CREATURE(lua_State *L)
static int lua_PRINT(lua_State *L)
static int lua_MESSAGE(lua_State *L)
static int lua_PLAY_MESSAGE(lua_State *L)
static int lua_ADD_GOLD_TO_PLAYER(lua_State *L)
static int lua_SET_CREATURE_TENDENCIES(lua_State *L)
static int lua_REVEAL_MAP_RECT(lua_State *L)
static int lua_CONCEAL_MAP_RECT(lua_State *L)
static int lua_REVEAL_MAP_LOCATION(lua_State *L)
static int lua_LEVEL_VERSION(lua_State *L)
static int lua_KILL_CREATURE(lua_State *L)
static int lua_COMPUTER_DIG_TO_LOCATION(lua_State *L)
static int lua_USE_POWER_ON_CREATURE(lua_State *L)
static int lua_USE_POWER_AT_POS(lua_State *L)
static int lua_USE_POWER_AT_SUBTILE(lua_State *L)
static int lua_USE_POWER_AT_LOCATION(lua_State *L)
static int lua_USE_POWER(lua_State *L)
static int lua_USE_SPECIAL_INCREASE_LEVEL(lua_State *L)
static int lua_USE_SPECIAL_MULTIPLY_CREATURES",
static int lua_USE_SPECIAL_MAKE_SAFE(lua_State *L)
static int lua_USE_SPECIAL_LOCATE_HIDDEN_WORLD"
static int lua_USE_SPECIAL_TRANSFER_CREATURE(lua_State *L)
static int lua_TRANSFER_CREATURE(lua_State *L)
static int lua_CHANGE_CREATURES_ANNOYANCE(lua_State *L)
static int lua_RUN_AFTER_VICTORY(lua_State *L)
static int lua_LEVEL_UP_CREATURE(lua_State *L)
static int lua_CHANGE_CREATURE_OWNER(lua_State *L)
static int lua_SET_GAME_RULE(lua_State *L)
static int lua_SET_TRAP_CONFIGURATION(lua_State *L)
static int lua_SET_DOOR_CONFIGURATION(lua_State *L)
static int lua_SET_OBJECT_CONFIGURATION(lua_State *L)
static int lua_SET_CREATURE_CONFIGURATION(lua_State *L)
static int lua_SET_SACRIFICE_RECIPE(lua_State *L)
static int lua_REMOVE_SACRIFICE_RECIPE(lua_State *L)
static int lua_SET_BOX_TOOLTIP(lua_State *L)
static int lua_SET_BOX_TOOLTIP_ID(lua_State *L)
static int lua_CHANGE_SLAB_OWNER(lua_State *L)
static int lua_CHANGE_SLAB_TYPE(lua_State *L)
static int lua_CREATE_EFFECTS_LINE(lua_State *L)
static int lua_QUICK_MESSAGE(lua_State *L)
static int lua_DISPLAY_MESSAGE(lua_State *L)
static int lua_USE_SPELL_ON_CREATURE(lua_State *L)
static int lua_SET_HEART_HEALTH(lua_State *L)
static int lua_ADD_HEART_HEALTH(lua_State *L)
static int lua_CREATURE_ENTRANCE_LEVEL(lua_State *L)
static int lua_RANDOMISE_FLAG(lua_State *L)
static int lua_DISPLAY_TIMER(lua_State *L)
static int lua_ADD_TO_TIMER(lua_State *L)
static int lua_ADD_BONUS_TIME(lua_State *L)
static int lua_DISPLAY_VARIABLE(lua_State *L)
static int lua_DISPLAY_COUNTDOWN(lua_State *L)
static int lua_HIDE_TIMER(lua_State *L)
static int lua_HIDE_VARIABLE(lua_State *L)
static int lua_CREATE_EFFECT(lua_State *L)
static int lua_CREATE_EFFECT_AT_POS(lua_State *L)
static int lua_HEART_LOST_QUICK_OBJECTIVE(lua_State *L)
static int lua_HEART_LOST_OBJECTIVE(lua_State *L)
static int lua_SET_DOOR(lua_State *L)
static int lua_SET_CREATURE_INSTANCE(lua_State *L)
static int lua_SET_HAND_RULE(lua_State *L)
static int lua_MOVE_CREATURE(lua_State *L)
static int lua_COUNT_CREATURES_AT_ACTION_POINT(lua_State *L)
static int lua_SET_TEXTURE(lua_State *L)
static int lua_HIDE_HERO_GATE(lua_State *L)

*/














/**********************************************/
// game
/**********************************************/


static int lua_get_creature_near(lua_State *L)
{
    // the arguments lua passes to the C code
    int stl_x = lua_tointeger(L, 1); // the last number is the position of the argument, just increment these
    int stl_y = lua_tointeger(L, 2);

    struct Thing *thing = get_creature_near(stl_x * COORD_PER_STL, stl_y * COORD_PER_STL);

    // arguments you push back to lua
    lua_pushThing(L, thing);
    return 1; // return value is the amount of args you push back
}

static int lua_get_thing_by_idx(lua_State *L)
{
    // the arguments lua passes to the C code
    ThingIndex tng_idx = lua_tointeger(L, 1);

    struct Thing *thing = thing_get(tng_idx);

    // arguments you push back to lua
    lua_pushThing(L, thing);
    return 1; // return value is the amount of args you push back
}

static int send_chat_message(lua_State *L)
{
    int plyr_idx = lua_tointeger(L, 1);
    const char *msg = lua_tostring(L, 2);

    char type = MsgType_Player;

    message_add(type,plyr_idx, msg);

    return 0;
}

static int lua_print(lua_State *L)
{
    const char* msg = lua_tostring(L, 1); // the last number is the position of the argument, just increment these

    JUSTLOG("%s",msg);
    return 0; // return value is the amount of args you push back
}

static int lua_get_things_of_class(lua_State *L)
{
    ThingClass class_id = luaL_checkNamedCommand(L,1,class_commands);

    const struct StructureList* slist = get_list_for_thing_class(class_id);
    ThingIndex i = slist->index;
    long k = 0;

    lua_newtable(L);

    while (i != 0)
    {
        struct Thing *thing;
        thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_of_class;
        // Per-thing code

        lua_pushThing(L, thing);  // Push the value onto the stack
        lua_rawseti(L, -2, k + 1);      // Set table[-2][i + 1] = value
        
        // Per-thing code ends
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
    return 1; // return value is the amount of args you push back
}



static const luaL_Reg global_methods[] = {
//Setup Commands
   {"SET_GENERATE_SPEED"            ,lua_SET_GENERATE_SPEED           },
   {"COMPUTER_PLAYER"               ,lua_COMPUTER_PLAYER              },
//   {"ALLY_PLAYERS"                  ,lua_ALLY_PLAYERS                 },
   {"START_MONEY"                   ,lua_START_MONEY                  },
   {"MAX_CREATURES"                 ,lua_MAX_CREATURES                },
   {"ADD_CREATURE_TO_POOL"          ,lua_ADD_CREATURE_TO_POOL         },
   {"CREATURE_AVAILABLE"            ,lua_CREATURE_AVAILABLE           },
//   {"DEAD_CREATURES_RETURN_TO_POOL" ,lua_DEAD_CREATURES_RETURN_TO_POOL},
   {"ROOM_AVAILABLE"                ,lua_ROOM_AVAILABLE               },
   {"MAGIC_AVAILABLE"               ,lua_MAGIC_AVAILABLE              },
   {"DOOR_AVAILABLE"                ,lua_DOOR_AVAILABLE               },
   {"TRAP_AVAILABLE"                ,lua_TRAP_AVAILABLE               },

//Script flow control
   {"WIN_GAME"                             ,lua_WIN_GAME                        },
   {"LOSE_GAME"                            ,lua_LOSE_GAME                       },
/*
   {"COUNT_CREATURES_AT_ACTION_POINT"      ,lua_COUNT_CREATURES_AT_ACTION_POINT },
   {"SET_TIMER"                            ,lua_SET_TIMER                       },
   {"ADD_TO_TIMER"                         ,lua_ADD_TO_TIMER                    },
   {"DISPLAY_TIMER"                        ,lua_DISPLAY_TIMER                   },
   {"HIDE_TIMER"                           ,lua_HIDE_TIMER                      },
   {"BONUS_LEVEL_TIME"                     ,lua_BONUS_LEVEL_TIME                },
   {"ADD_BONUS_TIME"                       ,lua_ADD_BONUS_TIME                  },
*/
//Adding New Creatures and Parties to the Level
   {"ADD_CREATURE_TO_LEVEL"                ,lua_ADD_CREATURE_TO_LEVEL           },
   {"ADD_TUNNELLER_TO_LEVEL"               ,lua_ADD_TUNNELLER_TO_LEVEL          },
   {"CREATE_PARTY"                         ,lua_CREATE_PARTY                    },
   {"ADD_TO_PARTY"                         ,lua_ADD_TO_PARTY                    },
   {"DELETE_FROM_PARTY"                    ,lua_DELETE_FROM_PARTY               },
   {"ADD_TUNNELLER_PARTY_TO_LEVEL"         ,lua_ADD_TUNNELLER_PARTY_TO_LEVEL    },
   {"ADD_PARTY_TO_LEVEL"                   ,lua_ADD_PARTY_TO_LEVEL              },

//Displaying information and affecting interface
   {"DISPLAY_OBJECTIVE"                    ,lua_DISPLAY_OBJECTIVE               },
   {"DISPLAY_OBJECTIVE_WITH_POS"           ,lua_DISPLAY_OBJECTIVE_WITH_POS      },
   {"DISPLAY_INFORMATION"                  ,lua_DISPLAY_INFORMATION             },
   {"DISPLAY_INFORMATION_WITH_POS"         ,lua_DISPLAY_INFORMATION_WITH_POS    },
/*
   {"QUICK_OBJECTIVE"                      ,lua_QUICK_OBJECTIVE                 },
   {"QUICK_OBJECTIVE_WITH_POS"             ,lua_QUICK_OBJECTIVE_WITH_POS        },
   {"QUICK_INFORMATION"                    ,lua_QUICK_INFORMATION               },
   {"QUICK_INFORMATION_WITH_POS"           ,lua_QUICK_INFORMATION_WITH_POS      },
   {"DISPLAY_MESSAGE"                      ,lua_DISPLAY_MESSAGE                 },
   {"QUICK_MESSAGE"                        ,lua_QUICK_MESSAGE                   },
   {"HEART_LOST_OBJECTIVE"                 ,lua_HEART_LOST_OBJECTIVE            },
   {"HEART_LOST_QUICK_OBJECTIVE"           ,lua_HEART_LOST_QUICK_OBJECTIVE      },
   {"PLAY_MESSAGE"                         ,lua_PLAY_MESSAGE                    },
   {"TUTORIAL_FLASH_BUTTON"                ,lua_TUTORIAL_FLASH_BUTTON           },
   {"DISPLAY_COUNTDOWN"                    ,lua_DISPLAY_COUNTDOWN               },
   {"DISPLAY_VARIABLE"                     ,lua_DISPLAY_VARIABLE                },
   {"HIDE_VARIABLE"                        ,lua_HIDE_VARIABLE                   },

//Manipulating Map
   {"REVEAL_MAP_LOCATION"                  ,lua_REVEAL_MAP_LOCATION             },
   {"REVEAL_MAP_RECT"                      ,lua_REVEAL_MAP_RECT                 },
   {"CONCEAL_MAP_RECT"                     ,lua_CONCEAL_MAP_RECT                },
   {"SET_TEXTURE"                          ,lua_SET_TEXTURE                     },
   {"SET_DOOR"                             ,lua_SET_DOOR                        },
   */
   {"ADD_OBJECT_TO_LEVEL"                  ,lua_ADD_OBJECT_TO_LEVEL             },
   /*
   {"ADD_EFFECT_GENERATOR_TO_LEVEL"        ,lua_ADD_EFFECT_GENERATOR_TO_LEVEL   },

//Manipulating Configs
   {"SET_GAME_RULE"                        ,lua_SET_GAME_RULE                   },
   {"SET_HAND_RULE"                        ,lua_SET_HAND_RULE                   },
   {"SET_DOOR_CONFIGURATION"               ,lua_SET_DOOR_CONFIGURATION          },
   {"NEW_OBJECT_TYPE"                      ,lua_NEW_OBJECT_TYPE                 },
   {"SET_OBJECT_CONFIGURATION"             ,lua_SET_OBJECT_CONFIGURATION        },
   {"NEW_TRAP_TYPE"                        ,lua_NEW_TRAP_TYPE                   },
   {"SET_TRAP_CONFIGURATION"               ,lua_SET_TRAP_CONFIGURATION          },
   {"NEW_CREATURE_TYPE"                    ,lua_NEW_CREATURE_TYPE               },
   {"SET_CREATURE_CONFIGURATION"           ,lua_SET_CREATURE_CONFIGURATION      },
   {"SET_EFFECT_GENERATOR_CONFIGURATION"   ,lua_SET_EFFECT_GENERATOR_CONFIGURATI},
   {"SET_POWER_CONFIGURATION"              ,lua_SET_POWER_CONFIGURATION         },
   {"SWAP_CREATURE"                        ,lua_SWAP_CREATURE                   },
   {"NEW_ROOM_TYPE"                        ,lua_NEW_ROOM_TYPE                   },
   {"SET_ROOM_CONFIGURATION"               ,lua_SET_ROOM_CONFIGURATION          },
   {"SET_SACRIFICE_RECIPE"                 ,lua_SET_SACRIFICE_RECIPE            },
   {"REMOVE_SACRIFICE_RECIPE"              ,lua_REMOVE_SACRIFICE_RECIPE         },
   {"SET_MUSIC"                            ,lua_SET_MUSIC                       },

//Manipulating Creature stats
   {"SET_CREATURE_INSTANCE"                ,lua_SET_CREATURE_INSTANCE           },
   {"SET_CREATURE_MAX_LEVEL"               ,lua_SET_CREATURE_MAX_LEVEL          },
   {"SET_CREATURE_PROPERTY"                ,lua_SET_CREATURE_PROPERTY           },
   {"SET_CREATURE_TENDENCIES"              ,lua_SET_CREATURE_TENDENCIES         },
   {"CREATURE_ENTRANCE_LEVEL"              ,lua_CREATURE_ENTRANCE_LEVEL         },

//Man{"ipulating individual Creatures
   {"TRANSFER_CREATURE"                    ,lua_TRANSFER_CREATURE               },
   {"KILL_CREATURE"                        ,lua_KILL_CREATURE                   },
   {"LEVEL_UP_CREATURE"                    ,lua_LEVEL_UP_CREATURE               },
   {"MOVE_CREATURE"                        ,lua_MOVE_CREATURE                   },
   {"CHANGE_CREATURE_OWNER"                ,lua_CHANGE_CREATURE_OWNER           },
   {"CHANGE_CREATURES_ANNOYANCE"           ,lua_CHANGE_CREATURES_ANNOYANCE      },
   {"LEVEL_UP_PLAYERS_CREATURES"           ,lua_LEVEL_UP_PLAYERS_CREATURES      },
*/
//Manipulating Research
   {"RESEARCH"                             ,lua_RESEARCH                        },
   {"RESEARCH_ORDER"                       ,lua_RESEARCH_ORDER                  },
/*
//Tweaking players
   {"ADD_GOLD_TO_PLAYER"                   ,lua_ADD_GOLD_TO_PLAYER              },
   {"SET_PLAYER_COLOR"                     ,lua_SET_PLAYER_COLOR                },
   {"SET_PLAYER_MODIFIER"                  ,lua_SET_PLAYER_MODIFIER             },
   {"ADD_TO_PLAYER_MODIFIER"               ,lua_ADD_TO_PLAYER_MODIFIER          },

//Tweaking computer players
   {"COMPUTER_DIG_TO_LOCATION"             ,lua_COMPUTER_DIG_TO_LOCATION        },
   {"SET_COMPUTER_PROCESS"                 ,lua_SET_COMPUTER_PROCESS            },
   {"SET_COMPUTER_CHECKS"                  ,lua_SET_COMPUTER_CHECKS             },
   {"SET_COMPUTER_GLOBALS"                 ,lua_SET_COMPUTER_GLOBALS            },
   {"SET_COMPUTER_EVENT"                   ,lua_SET_COMPUTER_EVENT              },

//Specials
   {"USE_SPECIAL_INCREASE_LEVEL"           ,lua_USE_SPECIAL_INCREASE_LEVEL      },
   {"USE_SPECIAL_MULTIPLY_CREATURES"       ,lua_USE_SPECIAL_MULTIPLY_CREATURES  },
   {"USE_SPECIAL_TRANSFER_CREATURE"        ,lua_USE_SPECIAL_TRANSFER_CREATURE   },
   {"SET_BOX_TOOLTIP"                      ,lua_SET_BOX_TOOLTIP                 },
   {"SET_BOX_TOOLTIP_ID"                   ,lua_SET_BOX_TOOLTIP_ID              },

//Effects
   {"CREATE_EFFECT"                        ,lua_CREATE_EFFECT                   },
   {"CREATE_EFFECT_AT_POS"                 ,lua_CREATE_EFFECT_AT_POS            },
   {"CREATE_EFFECTS_LINE"                  ,lua_CREATE_EFFECTS_LINE             },

//Other
   {"USE_POWER"                            ,lua_USE_POWER                       },
   {"USE_POWER_AT_LOCATION"                ,lua_USE_POWER_AT_LOCATION           },
   {"USE_POWER_AT_POS"                     ,lua_USE_POWER_AT_POS                },
   {"USE_POWER_ON_CREATURE"                ,lua_USE_POWER_ON_CREATURE           },
   {"USE_SPELL_ON_CREATURE"                ,lua_USE_SPELL_ON_CREATURE           },
   {"USE_SPELL_ON_PLAYERS_CREATURES"       ,lua_USE_SPELL_ON_PLAYERS_CREATURES  },
   {"USE_POWER_ON_PLAYERS_CREATURES"       ,lua_USE_POWER_ON_PLAYERS_CREATURES  },
   {"LOCATE_HIDDEN_WORLD"                  ,lua_LOCATE_HIDDEN_WORLD             },
   {"MAKE_SAFE"                            ,lua_MAKE_SAFE                       },
   {"MAKE_UNSAFE"                          ,lua_MAKE_UNSAFE                     },
   {"SET_HAND_GRAPHIC"                     ,lua_SET_HAND_GRAPHIC                },
   {"SET_INCREASE_ON_EXPERIENCE"           ,lua_SET_INCREASE_ON_EXPERIENCE      },
*/

//debug stuff
   {"print"           ,lua_print      },

    {"GetCreatureNear", lua_get_creature_near},
    {"SendChatMessage", send_chat_message},
    {"getThingByIdx", lua_get_thing_by_idx},
    {"get_things_of_class", lua_get_things_of_class},
};

static const luaL_Reg game_meta[] = {
    {NULL, NULL}
};

static void global_register(lua_State *L)
{
    //luaL_newlib(L, global_methods);
    for (size_t i = 0; i < (sizeof(global_methods)/sizeof(global_methods[0])); i++)
    {
        lua_register(L, global_methods[i].name, global_methods[i].func);
    }
    lua_setglobal(L, "thing");
}


/**********************************************/
// things
/**********************************************/

static int make_thing_zombie (lua_State *L)
{
    struct Thing *thing = luaL_checkThing(L, 1);

    //internal_set_thing_state(thing, CrSt_Disabled);
    //thing->active_state = CrSt_Disabled;
    //thing->continue_state = CrSt_Disabled;

    thing->alloc_flags |= TAlF_IsControlled;


    return 0;
}

static int lua_delete_thing(lua_State *L)
{
    struct Thing *thing = luaL_checkThing(L, 1);
    delete_thing_structure(thing,0);
    return 0;
}

static int move_thing_to(lua_State *L)
{
    struct Thing *thing = luaL_checkThing(L, 1);
    int stl_x = lua_tointeger(L, 2);
    int stl_y = lua_tointeger(L, 3);

    if (!setup_person_move_to_position(thing, stl_x, stl_y, NavRtF_Default))
        WARNLOG("Move %s order failed", thing_model_name(thing));
    thing->continue_state = CrSt_ManualControl;

    return 0;
}

static int lua_kill_creature(lua_State *L)
{
    struct Thing* thing = luaL_checkThing(L, 1);
    kill_creature(thing, INVALID_THING, -1, CrDed_NoUnconscious);

    return 0;
}

static int thing_tostring(lua_State *L)
{
    char buff[64];
    struct Thing* thing = luaL_checkThing(L, 1);
    snprintf(buff, sizeof(buff), "id: %d turn: %ld %s", thing->index, thing->creation_turn, thing_class_and_model_name(thing->class_id,thing->model));

    lua_pushfstring(L, "Thing (%s)", buff);
    return 1;
}

// Function to set field values
static int thing_set_field(lua_State *L) {

    struct Thing* thing = luaL_checkThing(L, 1);
    const char* key = luaL_checkstring(L, 2);
    int value = luaL_checkinteger(L, 3);

    //char* read_only_arr[] =  ["index","creation_turn"];

    if (strcmp(key, "orientation") == 0) {
        thing->move_angle_xy = value;
    } else {
        //luaL_error(L, "Unknown field: %s", key);
    }

    return 0;
}



// Function to get field values
static int thing_get_field(lua_State *L) {

    const char* key = luaL_checkstring(L, 2);

    // Check if the key exists in thing_methods
    for (int i = 0; thing_methods[i].name != NULL; i++) {
        if (strcmp(key, thing_methods[i].name) == 0) {
            // If the key exists in thing_methods, call the corresponding function
            lua_pushcfunction(L, thing_methods[i].func);
            return 1;
        }
    }
    
    struct Thing* thing = luaL_checkThing(L, 1);

    if (strcmp(key, "ThingIndex") == 0) {
        lua_pushinteger(L, thing->index);
    } else if (strcmp(key, "creation_turn") == 0) {
        lua_pushinteger(L, thing->creation_turn);
    } else if (strcmp(key, "model") == 0) {
        lua_pushstring(L,thing_model_only_name(thing->class_id,thing->model));
    } else if (strcmp(key, "owner") == 0) {
        lua_pushPlayer(L, thing->owner);
    } else if (strcmp(key, "pos") == 0) {
        lua_pushPos(L, &thing->mappos);
    } else {
        lua_pushnil(L);
    }

    return 1;

}

static int thing_eq(lua_State *L) {

    if (!lua_istable(L, 1) || !lua_istable(L, 2)) {
        luaL_error(L, "Expected a table");
        return 1;
    }

    // Get idx field
    lua_getfield(L, 1, "ThingIndex");
    if (!lua_isnumber(L, -1)) {
        luaL_error(L, "Expected 'index' to be an integer");
        return 1;
    }
    int idx1 = lua_tointeger(L, -1);
    lua_pop(L, 1);  // Pop the idx value off the stack

    // Get idx field
    lua_getfield(L, 2, "ThingIndex");
    if (!lua_isnumber(L, -1)) {
        luaL_error(L, "Expected 'index' to be an integer");
        return 1;
    }
    int idx2 = lua_tointeger(L, -1);
    lua_pop(L, 1);  // Pop the idx value off the stack

    if(idx1 != idx2)
    {
        lua_pushboolean(L, false);
        return 1;
    }


    // Get creation_turn field
    lua_getfield(L, 1, "creation_turn");
    if (!lua_isnumber(L, -1)) {
        luaL_error(L, "Expected 'creation_turn' to be an integer");
        return 1;
    }
    int creation_turn1 = lua_tointeger(L, -1);
    lua_pop(L, 1);  // Pop the creation_turn value off the stack

    lua_getfield(L,2, "creation_turn");
    if (!lua_isnumber(L, -1)) {
        luaL_error(L, "Expected 'creation_turn' to be an integer");
        return 1;
    }
    int creation_turn2 = lua_tointeger(L, -1);
    lua_pop(L, 1);  // Pop the creation_turn value off the stack

    lua_pushboolean(L, creation_turn1 == creation_turn2);
    return 1;
}


static const struct luaL_Reg thing_methods[] = {
    {"MakeThingZombie", make_thing_zombie},
    {"MoveThingTo",     move_thing_to},
    {"KillCreature",    lua_kill_creature},
    {"DeleteThing",     lua_delete_thing},
    {NULL, NULL}
};

static const struct luaL_Reg thing_meta[] = {
    {"__tostring", thing_tostring},
    {"__index",    thing_get_field},
    {"__newindex", thing_set_field},
    {"__eq",       thing_eq},
    {NULL, NULL}
};


static int Thing_register(lua_State *L)
{
    // Create a metatable for thing and add it to the registry
    luaL_newmetatable(L, "Thing");

    // Set the __index and __newindex metamethods
    luaL_setfuncs(L, thing_meta, 0);

    // Create a methods table
    luaL_newlib(L, thing_methods);

    for (int i = 0; thing_methods[i].name != NULL; i++) {
        const char *name = thing_methods[i].name;
        lua_pushcfunction(L, thing_methods[i].func);
        lua_setfield(L, -2, name);
    }


    // Hide the metatable by setting the __metatable field to nil
    lua_pushliteral(L, "__metatable");
    lua_pushnil(L);
    lua_rawset(L, -3);

    // Pop the metatable from the stack
    lua_pop(L, 1);

    return 1; // Return the methods table
}




/**********************************************/
//Player
/**********************************************/

static int player_tostring(lua_State *L)
{
    PlayerNumber player_idx = luaL_checkPlayerSingle(L, 1);
    
    lua_pushstring(L,get_conf_parameter_text(player_desc,player_idx));
    return 1;

}

// Function to set field values
static int player_set_field(lua_State *L) {

    PlayerNumber player_idx = luaL_checkPlayerSingle(L, 1);
    const char* key = luaL_checkstring(L, 2);
    int value = luaL_checkinteger(L, 3);

    long variable_type;
    long variable_id;

    if (parse_get_varib(key, &variable_id, &variable_type))
    {
        set_variable(player_idx,variable_type,variable_id,value);
        return 0;
    }

    /*
    JUSTLOG("set key %s",key);

    //char* read_only_arr[] =  ["index","creation_turn"];

    if (strcmp(key, "Orientation") == 0) {
        thing->move_angle_xy = value;
    } else {
        //luaL_error(L, "Unknown field: %s", key);
    }
*/
    return 0;
}

// Function to get field values
static int player_get_field(lua_State *L) {

    PlayerNumber player_idx = luaL_checkPlayerSingle(L, 1);
    const char* key = luaL_checkstring(L, 2);

    
    long variable_type;
    long variable_id;

    if (parse_get_varib(key, &variable_id, &variable_type))
    {
        lua_pushinteger(L, get_condition_value(player_idx, variable_type, variable_id));
        return 1;
    }

/*
    if (strcmp(key, "index") == 0) {
        lua_pushinteger(L, thing->index);
    } else if (strcmp(key, "creation_turn") == 0) {
        lua_pushinteger(L, thing->creation_turn);
    } else if (strcmp(key, "Owner") == 0) {
        lua_pushPlayer(L, thing->owner);


    } else {
        lua_pushnil(L);
    }
*/
    return 1;

}

static int player_eq(lua_State *L) {

    if (!lua_istable(L, 1) || !lua_istable(L, 2)) {
        luaL_error(L, "Expected a table");
        return 1;
    }

    // Get idx field
    lua_getfield(L, 1, "playerId");
    if (!lua_isnumber(L, -1)) {
        luaL_error(L, "Expected 'playerId' to be an integer");
        return 1;
    }
    int idx1 = lua_tointeger(L, -1);
    lua_pop(L, 1);  // Pop the idx value off the stack

    // Get idx field
    lua_getfield(L, 2, "playerId");
    if (!lua_isnumber(L, -1)) {
        luaL_error(L, "Expected 'playerId' to be an integer");
        return 1;
    }
    int idx2 = lua_tointeger(L, -1);
    lua_pop(L, 1);  // Pop the idx value off the stack


    lua_pushboolean(L, idx1 == idx2);
    return 1;
}

static const struct luaL_Reg player_methods[] = {
    {NULL, NULL}
};

static const struct luaL_Reg player_meta[] = {
    {"__tostring", player_tostring},
    {"__index",    player_get_field},
    {"__newindex", player_set_field},
    {"__eq",       player_eq},
    {NULL, NULL}
};

static void Player_register(lua_State *L) {
    // Create a metatable for thing and add it to the registry
    luaL_newmetatable(L, "Player");

    // Set the __index and __newindex metamethods
    luaL_setfuncs(L, player_meta, 0);

    // Create a methods table
    luaL_newlib(L, player_methods);

    // Hide the metatable by setting the __metatable field to nil
    lua_pushliteral(L, "__metatable");
    lua_pushnil(L);
    lua_rawset(L, -3);

    // Pop the metatable from the stack
    lua_pop(L, 1);


    for (PlayerNumber i = 0; i < PLAYERS_COUNT; i++)
    {
        lua_pushPlayer(L,i);
        lua_setglobal(L, get_conf_parameter_text(player_desc,i));
    }
    

}


/**********************************************/
//...
/**********************************************/


void reg_host_functions(lua_State *L)
{
    Player_register(L);
    global_register(L);
    Thing_register(L);
}