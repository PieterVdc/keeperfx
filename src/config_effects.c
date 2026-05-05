/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_effects.c
 *     Effects configuration loading functions.
 * @par Purpose:
 *     Support of configuration files for effects and effect elements.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     25 May 2009 - 11 Mar 2014
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "config_effects.h"
#include "globals.h"

#include "bflib_basics.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"
#include "console_cmd.h"

#include "value_util.h"
#include <toml.h>
#include "config.h"
#include "config_strings.h"
#include "thing_effects.h"
#include "game_legacy.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
static TbBool load_effects_config_file(const char *fname, unsigned short flags);

const struct ConfigFileData keeper_effects_file_data = {
    .filename = "effects.toml",
    .load_func = load_effects_config_file,
    .pre_load_func = NULL,
    .post_load_func = NULL,
};

const struct NamedField effects_effectellements_named_fields[] = {
    {"NAME",                       0, field(game.conf.effects_conf.effectelement_cfgstats[0].code_name),                    0,    INT32_MIN, UINT32_MAX, effect_desc,     value_name,      assign_null},
    {"DrawClass",                  0, field(game.conf.effects_conf.effectelement_cfgstats[0].draw_class),                   0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"MoveType",                   0, field(game.conf.effects_conf.effectelement_cfgstats[0].move_type),                    0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"Unanimated",                 0, field(game.conf.effects_conf.effectelement_cfgstats[0].unanimated),                   0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"Lifespan",                   0, field(game.conf.effects_conf.effectelement_cfgstats[0].lifespan),                     0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"Lifespan",                   1, field(game.conf.effects_conf.effectelement_cfgstats[0].lifespan_random),              0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"AnimationId",                0, field(game.conf.effects_conf.effectelement_cfgstats[0].sprite_idx),                   0,    INT32_MIN, UINT32_MAX, NULL,            value_animid,    assign_animid},
    {"SpriteSize",                 0, field(game.conf.effects_conf.effectelement_cfgstats[0].sprite_size_min),              0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"SpriteSize",                 1, field(game.conf.effects_conf.effectelement_cfgstats[0].sprite_size_max),              0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"RenderFlags",                0, field(game.conf.effects_conf.effectelement_cfgstats[0].animate_once),                 0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"AnimateOnce",                0, field(game.conf.effects_conf.effectelement_cfgstats[0].animate_once),                 0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"SpriteSpeed",                0, field(game.conf.effects_conf.effectelement_cfgstats[0].sprite_speed_min),             0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"SpriteSpeed",                1, field(game.conf.effects_conf.effectelement_cfgstats[0].sprite_speed_max),             0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"AnimateOnFloor",             0, field(game.conf.effects_conf.effectelement_cfgstats[0].animate_on_flo),               0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"Unshaded",                   0, field(game.conf.effects_conf.effectelement_cfgstats[0].unshaded),                     0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"Transparent",                0, field(game.conf.effects_conf.effectelement_cfgstats[0].transparent),                  0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"MovementFlags",              0, field(game.conf.effects_conf.effectelement_cfgstats[0].through_walls),                0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"ThroughWalls",               0, field(game.conf.effects_conf.effectelement_cfgstats[0].through_walls),                0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"SizeChange",                 0, field(game.conf.effects_conf.effectelement_cfgstats[0].size_change),                  0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"FallAcceleration",           0, field(game.conf.effects_conf.effectelement_cfgstats[0].fall_accelerat),               0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"InertiaFloor",               0, field(game.conf.effects_conf.effectelement_cfgstats[0].inertia_floor),                0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"InertiaAir",                 0, field(game.conf.effects_conf.effectelement_cfgstats[0].inertia_air),                  0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"SubeffectModel",             0, field(game.conf.effects_conf.effectelement_cfgstats[0].subeffect_mode),               0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"SubeffectDelay",             0, field(game.conf.effects_conf.effectelement_cfgstats[0].subeffect_dela),               0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"Movable",                    0, field(game.conf.effects_conf.effectelement_cfgstats[0].movable),                      0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"Impacts",                    0, field(game.conf.effects_conf.effectelement_cfgstats[0].impacts),                      0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"SolidGroundEffmodel",        0, field(game.conf.effects_conf.effectelement_cfgstats[0].solidgnd_effmodel),            0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"SolidGroundSoundId",         0, field(game.conf.effects_conf.effectelement_cfgstats[0].solidgnd_snd_smpid),           0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"SolidGroundLoudness",        0, field(game.conf.effects_conf.effectelement_cfgstats[0].solidgnd_loudness),            0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"SolidGroundDestroyOnImpact", 0, field(game.conf.effects_conf.effectelement_cfgstats[0].solidgnd_destroy_on_impact),   0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"WaterEffmodel",              0, field(game.conf.effects_conf.effectelement_cfgstats[0].water_effmodel),               0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"WaterSoundId",               0, field(game.conf.effects_conf.effectelement_cfgstats[0].water_snd_smpid),              0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"WaterLoudness",              0, field(game.conf.effects_conf.effectelement_cfgstats[0].water_loudness),               0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"WaterDestroyOnImpact",       0, field(game.conf.effects_conf.effectelement_cfgstats[0].water_destroy_on_impact),      0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"LavaEffmodel",               0, field(game.conf.effects_conf.effectelement_cfgstats[0].lava_effmodel),                0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"LavaSoundId",                0, field(game.conf.effects_conf.effectelement_cfgstats[0].lava_snd_smpid),               0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"LavaLoudness",               0, field(game.conf.effects_conf.effectelement_cfgstats[0].lava_loudness),                0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"LavaDestroyOnImpact",        0, field(game.conf.effects_conf.effectelement_cfgstats[0].lava_destroy_on_impact),       0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"TransformModel",             0, field(game.conf.effects_conf.effectelement_cfgstats[0].transform_model),              0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"LightRadius",                0, field(game.conf.effects_conf.effectelement_cfgstats[0].light_radius),                 0,    INT32_MIN, UINT32_MAX, NULL,            value_stltocoord,assign_default},
    {"LightIntensity",             0, field(game.conf.effects_conf.effectelement_cfgstats[0].light_intensity),              0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"LightFlags",                 0, field(game.conf.effects_conf.effectelement_cfgstats[0].light_flags),                  0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"AffectedByWind",             0, field(game.conf.effects_conf.effectelement_cfgstats[0].affected_by_wind),             0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {NULL},
};

const struct NamedFieldSet effects_effectemements_named_fields_set = {
    &game.conf.effects_conf.effectelements_cfgstats_count,
    "effectElement",
    effects_effectellements_named_fields,
    effectelem_desc,
    EFFECTSELLEMENTS_TYPES_MAX,
    sizeof(game.conf.effects_conf.effectelements_cfgstats[0]),
    game.conf.effects_conf.effectelement_cfgstats,
};


const struct NamedField effects_effect_named_fields[] = {
    {"NAME",                   0, field(game.conf.effects_conf.effect_cfgstats[0].code_name),            0,    INT32_MIN, UINT32_MAX, effect_desc,     value_name,      assign_null},
    {"GenerationAccelXYRange", 0, field(game.conf.effects_conf.effect_cfgstats[0].accel_xy_min),         0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"GenerationAccelXYRange", 1, field(game.conf.effects_conf.effect_cfgstats[0].accel_xy_max),         0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"GenerationAccelZRange",  0, field(game.conf.effects_conf.effect_cfgstats[0].accel_z_min),          0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"GenerationAccelZRange",  1, field(game.conf.effects_conf.effect_cfgstats[0].accel_z_max),          0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"GenerationKindRange",    0, field(game.conf.effects_conf.effect_cfgstats[0].kind_min),             0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"GenerationKindRange",    1, field(game.conf.effects_conf.effect_cfgstats[0].kind_max),             0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"Health",                 0, field(game.conf.effects_conf.effect_cfgstats[0].start_health),         0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"GenerationType",         0, field(game.conf.effects_conf.effect_cfgstats[0].generation_type),      0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"AreaAffectType",         0, field(game.conf.effects_conf.effect_cfgstats[0].area_affect_type),     0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"AffectedByWind",         0, field(game.conf.effects_conf.effect_cfgstats[0].affected_by_wind),     0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"LightRadius",            0, field(game.conf.effects_conf.effect_cfgstats[0].ilght.radius),         0,    INT32_MIN, UINT32_MAX, NULL,            value_stltocoord,assign_default},
    {"LightIntensity",         0, field(game.conf.effects_conf.effect_cfgstats[0].ilght.intensity),      0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"LightFlags",             0, field(game.conf.effects_conf.effect_cfgstats[0].ilght.flags),          0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"ElementsCount",          0, field(game.conf.effects_conf.effect_cfgstats[0].elements_count),       0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"AlwaysGenerate",         0, field(game.conf.effects_conf.effect_cfgstats[0].always_generate),      0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"HitType",                0, field(game.conf.effects_conf.effect_cfgstats[0].effect_hit_type),      0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"SpellEffect",            0, field(game.conf.effects_conf.effect_cfgstats[0].spell_effect),         0,    INT32_MIN, UINT32_MAX, spell_desc,      value_default,   assign_default},
    {NULL},
};

const struct NamedFieldSet effects_effect_named_fields_set = {
    &game.conf.effects_conf.effectgen_cfgstats_count,
    "effect",
    effects_effect_named_fields,
    effect_desc,
    EFFECTS_TYPES_MAX,
    sizeof(game.conf.effects_conf.effect_cfgstats[0]),
    game.conf.effects_conf.effect_cfgstats,
};


const struct NamedField effects_effectgenerator_named_fields[] = {
    {"NAME",                   0, field(game.conf.effects_conf.effectgen_cfgstats[0].code_name),            0,    INT32_MIN, UINT32_MAX, effectgen_desc,  value_name,      assign_null},
    {"GENERATIONDELAYMIN",     0, field(game.conf.effects_conf.effectgen_cfgstats[0].generation_delay_min), 0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"GENERATIONDELAYMAX",     0, field(game.conf.effects_conf.effectgen_cfgstats[0].generation_delay_max), 0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"GENERATIONAMOUNT",       0, field(game.conf.effects_conf.effectgen_cfgstats[0].generation_amount),    0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"EFFECTMODEL",            0, field(game.conf.effects_conf.effectgen_cfgstats[0].effect_model),         0,    INT32_MIN, UINT32_MAX, NULL,            value_effOrEffEl,assign_default},
    {"IGNORETERRAIN",          0, field(game.conf.effects_conf.effectgen_cfgstats[0].ignore_terrain),       0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"SPAWNHEIGHT",            0, field(game.conf.effects_conf.effectgen_cfgstats[0].spawn_height),         1,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"ACCELERATIONMIN",        0, field(game.conf.effects_conf.effectgen_cfgstats[0].acc_x_min),            0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"ACCELERATIONMIN",        1, field(game.conf.effects_conf.effectgen_cfgstats[0].acc_y_min),            0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"ACCELERATIONMIN",        2, field(game.conf.effects_conf.effectgen_cfgstats[0].acc_z_min),            0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"ACCELERATIONMAX",        0, field(game.conf.effects_conf.effectgen_cfgstats[0].acc_x_max),            0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"ACCELERATIONMAX",        1, field(game.conf.effects_conf.effectgen_cfgstats[0].acc_y_max),            0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"ACCELERATIONMAX",        2, field(game.conf.effects_conf.effectgen_cfgstats[0].acc_z_max),            0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"SOUND",                  0, field(game.conf.effects_conf.effectgen_cfgstats[0].sound_sample_idx),     0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {"SOUND",                  1, field(game.conf.effects_conf.effectgen_cfgstats[0].sound_sample_rng),     0,    INT32_MIN, UINT32_MAX, NULL,            value_default,   assign_default},
    {NULL},
};

const struct NamedFieldSet effects_effectgenerator_named_fields_set = {
    &game.conf.effects_conf.effectgen_cfgstats_count,
    "effectGenerator",
    effects_effectgenerator_named_fields,
    effectgen_desc,
    EFFECTSGEN_TYPES_MAX,
    sizeof(game.conf.effects_conf.effectgen_cfgstats[0]),
    game.conf.effects_conf.effectgen_cfgstats,
};

int32_t const imp_spangle_effects[] = {
    TngEff_ImpSpangleRed, TngEff_ImpSpangleBlue, TngEff_ImpSpangleGreen, TngEff_ImpSpangleYellow, TngEff_ImpSpangleWhite,
    TngEff_None, TngEff_ImpSpanglePurple, TngEff_ImpSpangleBlack, TngEff_ImpSpangleOrange
};

int32_t const ball_puff_effects[] = {
    TngEff_BallPuffRed, TngEff_BallPuffBlue, TngEff_BallPuffGreen, TngEff_BallPuffYellow, TngEff_BallPuffWhite,
    TngEff_BallPuffWhite, TngEff_BallPuffPurple, TngEff_BallPuffBlack, TngEff_BallPuffOrange
};

/******************************************************************************/
struct NamedCommand effect_desc[EFFECTS_TYPES_MAX];
struct NamedCommand effectgen_desc[EFFECTSGEN_TYPES_MAX];
struct NamedCommand effectelem_desc[EFFECTSELLEMENTS_TYPES_MAX];
/******************************************************************************/

static void load_effects(VALUE *value, unsigned short flags)
{
    char key[64] = "";
    VALUE *section;
    int max_effect_id = -1;
    for (int id = 0; id < EFFECTS_TYPES_MAX; id++)
    {
        {
            snprintf(key, sizeof(key), "effect%d", id);
            section = value_dict_get(value, key);
        }
        if (value_type(section) == VALUE_DICT)
        {
            struct EffectConfigStats *effcst = &game.conf.effects_conf.effect_cfgstats[id];

            SET_NAME(section,effect_desc,effcst->code_name);
            max_effect_id = id;

            CONDITIONAL_ASSIGN_ARR2_INT_MINMAX(section,"GenerationAccelXYRange",effcst->accel_xy_min,effcst->accel_xy_max);
            CONDITIONAL_ASSIGN_ARR2_INT_MINMAX(section,"GenerationAccelZRange", effcst->accel_z_min, effcst->accel_z_max);
            CONDITIONAL_ASSIGN_ARR2_INT_MINMAX(section,"GenerationKindRange",   effcst->kind_min,    effcst->kind_max);
            CONDITIONAL_ASSIGN_INT(section,"Health",        effcst->start_health);
            CONDITIONAL_ASSIGN_INT(section,"GenerationType",effcst->generation_type);
            CONDITIONAL_ASSIGN_INT(section,"AreaAffectType",effcst->area_affect_type);
            CONDITIONAL_ASSIGN_INT(section,"Sound"         ,effcst->effect_sound    );
            CONDITIONAL_ASSIGN_INT(section,"AffectedByWind",effcst->affected_by_wind);
            CONDITIONAL_ASSIGN_INT_SCALED(section,"LightRadius"   ,effcst->ilght.radius, COORD_PER_STL);
            CONDITIONAL_ASSIGN_INT(section,"LightIntensity",effcst->ilght.intensity );
            CONDITIONAL_ASSIGN_INT(section,"LightFlags"    ,effcst->ilght.flags   );
            CONDITIONAL_ASSIGN_INT(section,"ElementsCount" ,effcst->elements_count  );
            CONDITIONAL_ASSIGN_INT(section,"AlwaysGenerate",effcst->always_generate );
            CONDITIONAL_ASSIGN_INT(section,"HitType",effcst->effect_hit_type);
            CONDITIONAL_ASSIGN_SPELL(section,"SpellEffect",effcst->spell_effect);
        }
    }
    
    // Set sentinel NULL entry to mark the end of valid entries
    if (max_effect_id >= 0)
    {
        if (max_effect_id + 1 < EFFECTS_TYPES_MAX)
        {
            effect_desc[max_effect_id + 1].name = NULL;
        }
        // Fill any gaps with placeholder entries so get_id() won't terminate early
        for (int id = 0; id <= max_effect_id; id++)
        {
            if (effect_desc[id].name == NULL)
            {
                effect_desc[id].num = id;
                // Use the code_name from the effect config (should be initialized to empty or "NULL")
                effect_desc[id].name = game.conf.effects_conf.effect_cfgstats[id].code_name;
            }
        }
    }
}

static void load_effectsgenerators(VALUE *value, unsigned short flags)
{
    char key[KEY_SIZE];
    VALUE *section;
    int max_effectgen_id = -1;
    for (int id = 0; id < EFFECTSGEN_TYPES_MAX; id++)
    {
        {
            snprintf(key, sizeof(key), "effectGenerator%d", id);
            section = value_dict_get(value, key);
        }
        if (value_type(section) == VALUE_DICT)
        {
            struct EffectGeneratorConfigStats *effgencst = &game.conf.effects_conf.effectgen_cfgstats[id];

            SET_NAME(section,effectgen_desc,effgencst->code_name);
            max_effectgen_id = id;

            CONDITIONAL_ASSIGN_INT(section,"GenerationDelayMin",effgencst->generation_delay_min);
            CONDITIONAL_ASSIGN_INT(section,"GenerationDelayMax",effgencst->generation_delay_max);
            CONDITIONAL_ASSIGN_INT(section,"GenerationAmount"  ,effgencst->generation_amount);

            CONDITIONAL_ASSIGN_EFFECT_OR_EL_MODEL(section,"EffectModel",effgencst->effect_model);
            CONDITIONAL_ASSIGN_INT(section,"IgnoreTerrain",effgencst->ignore_terrain);
            CONDITIONAL_ASSIGN_INT(section,"SpawnHeight",effgencst->spawn_height);

            CONDITIONAL_ASSIGN_ARR3_INT(section,"AccelerationMin",effgencst->acc_x_min,effgencst->acc_y_min,effgencst->acc_z_min);
            CONDITIONAL_ASSIGN_ARR3_INT(section,"AccelerationMax",effgencst->acc_x_max,effgencst->acc_y_max,effgencst->acc_z_max);
            CONDITIONAL_ASSIGN_ARR2_INT(section,"Sound",effgencst->sound_sample_idx,effgencst->sound_sample_rng);
        }
    }
    // Set sentinel NULL entry to mark the end of valid entries
    if (max_effectgen_id >= 0)
    {
        if (max_effectgen_id + 1 < EFFECTSGEN_TYPES_MAX)
        {
            effectgen_desc[max_effectgen_id + 1].name = NULL;
        }
        // Fill any gaps with placeholder entries so get_id() won't terminate early
        for (int id = 0; id <= max_effectgen_id; id++)
        {
            if (effectgen_desc[id].name == NULL)
            {
                effectgen_desc[id].num = id;
                // Use the code_name from the effect generator config (should be initialized to empty or "NULL")
                effectgen_desc[id].name = game.conf.effects_conf.effectgen_cfgstats[id].code_name;
            }
        }
    }
}

static void load_effectelements(VALUE *value, unsigned short flags)
{
    char key[KEY_SIZE];
    VALUE *section;
    int max_effectelement_id = -1;
    for (int id = 0; id < EFFECTSELLEMENTS_TYPES_MAX; id++)
    {
        {
            snprintf(key, sizeof(key), "effectElement%d", id);
            section = value_dict_get(value, key);
        }
        if (value_type(section) == VALUE_DICT)
        {
            struct EffectElementConfigStats *effelcst = &game.conf.effects_conf.effectelement_cfgstats[id];

            SET_NAME(section,effectelem_desc,effelcst->code_name);
            max_effectelement_id = id;

            CONDITIONAL_ASSIGN_INT(section,"DrawClass", effelcst->draw_class);
            CONDITIONAL_ASSIGN_INT(section,"MoveType",  effelcst->move_type);
            CONDITIONAL_ASSIGN_INT(section,"Unanimated",effelcst->unanimated);
            CONDITIONAL_ASSIGN_ARR2_INT(section,"Lifespan",effelcst->lifespan,effelcst->lifespan_random);
            CONDITIONAL_ASSIGN_ANIMID(section,"AnimationId",effelcst->sprite_idx);
            CONDITIONAL_ASSIGN_ARR2_INT_MINMAX(section,"SpriteSize",effelcst->sprite_size_min,effelcst->sprite_size_max);
            CONDITIONAL_ASSIGN_INT(section,"RenderFlags",effelcst->animate_once); //todo Remove after people have had time to handle the rename
            CONDITIONAL_ASSIGN_INT(section, "AnimateOnce", effelcst->animate_once);
            CONDITIONAL_ASSIGN_ARR2_INT_MINMAX(section,"SpriteSpeed",effelcst->sprite_speed_min,effelcst->sprite_speed_max);

            CONDITIONAL_ASSIGN_BOOL(section,"AnimateOnFloor",  effelcst->animate_on_floor);
            CONDITIONAL_ASSIGN_BOOL(section,"Unshaded",        effelcst->unshaded);
            CONDITIONAL_ASSIGN_INT(section,"Transparent",      effelcst->transparent);
            CONDITIONAL_ASSIGN_INT(section,"MovementFlags",    effelcst->through_walls); //todo Remove after people have had time to handle the rename
            CONDITIONAL_ASSIGN_INT(section,"ThroughWalls",     effelcst->through_walls);
            CONDITIONAL_ASSIGN_INT(section,"SizeChange",       effelcst->size_change);
            CONDITIONAL_ASSIGN_INT(section,"FallAcceleration", effelcst->fall_acceleration);
            CONDITIONAL_ASSIGN_INT(section,"InertiaFloor",     effelcst->inertia_floor);
            CONDITIONAL_ASSIGN_INT(section,"InertiaAir",       effelcst->inertia_air);
            CONDITIONAL_ASSIGN_INT(section,"SubeffectModel",   effelcst->subeffect_model);
            CONDITIONAL_ASSIGN_INT(section,"SubeffectDelay",   effelcst->subeffect_delay);
            CONDITIONAL_ASSIGN_BOOL(section,"Movable",         effelcst->movable);
            CONDITIONAL_ASSIGN_BOOL(section,"Impacts",         effelcst->impacts);
            if(effelcst->impacts)
            {
                CONDITIONAL_ASSIGN_INT(section,"SolidGroundEffmodel", effelcst->solidgnd_effmodel);
                CONDITIONAL_ASSIGN_INT(section,"SolidGroundSoundId", effelcst->solidgnd_snd_smpid);
                CONDITIONAL_ASSIGN_INT(section,"SolidGroundLoudness", effelcst->solidgnd_loudness);
                CONDITIONAL_ASSIGN_BOOL(section,"SolidGroundDestroyOnImpact", effelcst->solidgnd_destroy_on_impact);
                CONDITIONAL_ASSIGN_INT(section,"WaterEffmodel", effelcst->water_effmodel);
                CONDITIONAL_ASSIGN_INT(section,"WaterSoundId", effelcst->water_snd_smpid);
                CONDITIONAL_ASSIGN_INT(section,"WaterLoudness", effelcst->water_loudness);
                CONDITIONAL_ASSIGN_BOOL(section,"WaterDestroyOnImpact", effelcst->water_destroy_on_impact);
                CONDITIONAL_ASSIGN_INT(section,"LavaEffmodel", effelcst->lava_effmodel);
                CONDITIONAL_ASSIGN_INT(section,"LavaSoundId", effelcst->lava_snd_smpid);
                CONDITIONAL_ASSIGN_INT(section,"LavaLoudness", effelcst->lava_loudness);
                CONDITIONAL_ASSIGN_BOOL(section,"LavaDestroyOnImpact", effelcst->lava_destroy_on_impact);
            }
            CONDITIONAL_ASSIGN_INT(section,"TransformModel", effelcst->transform_model  );
            CONDITIONAL_ASSIGN_INT_SCALED(section,"LightRadius",    effelcst->light_radius, COORD_PER_STL);
            CONDITIONAL_ASSIGN_INT(section,"LightIntensity", effelcst->light_intensity  );
            CONDITIONAL_ASSIGN_INT(section,"LightFlags",     effelcst->light_flags   );
            CONDITIONAL_ASSIGN_INT(section,"AffectedByWind", effelcst->affected_by_wind );
        }
    }
    // Set sentinel NULL entry to mark the end of valid entries
    if (max_effectelement_id >= 0)
    {
        if (max_effectelement_id + 1 < EFFECTSELLEMENTS_TYPES_MAX)
        {
            effectelem_desc[max_effectelement_id + 1].name = NULL;
        }
        // Fill any gaps with placeholder entries so get_id() won't terminate early
        for (int id = 0; id <= max_effectelement_id; id++)
        {
            if (effectelem_desc[id].name == NULL)
            {
                effectelem_desc[id].num = id;
                // Use the code_name from the effect element config (should be initialized to empty or "NULL")
                effectelem_desc[id].name = game.conf.effects_conf.effectelement_cfgstats[id].code_name;
            }
        }
    }
}

static TbBool load_effects_config_file(const char *fname, unsigned short flags)
{
    VALUE file_root;
    if (!load_toml_file(fname,&file_root,flags))
        return false;
    load_effectelements(&file_root, flags);
    load_effects(&file_root,flags);
    load_effectsgenerators(&file_root,flags);

    value_fini(&file_root);

    return true;
}

/**
 * Returns Code Name (name to use in script file) of given effect element model.
 */
const char* effect_element_code_name(ThingModel tngmodel)
{
    const char* name = get_conf_parameter_text(effectelem_desc, tngmodel);
    if (name[0] != '\0')
        return name;
    return "INVALID";
}

/**
 * Returns Code Name (name to use in script file) of given effect model.
 */
const char *effect_code_name(ThingModel tngmodel)
{
    const char* name = get_conf_parameter_text(effect_desc, tngmodel);
    if (name[0] != '\0')
        return name;
    return "INVALID";
}

const char *effectgenerator_code_name(ThingModel tngmodel)
{
    const char* name = get_conf_parameter_text(effectgen_desc, tngmodel);
    if (name[0] != '\0')
        return name;
    return "INVALID";
}

struct EffectGeneratorConfigStats *get_effectgenerator_model_stats(ThingModel tngmodel)
{
    if (tngmodel >= EFFECTSGEN_TYPES_MAX)
        return &game.conf.effects_conf.effectgen_cfgstats[0];
    return &game.conf.effects_conf.effectgen_cfgstats[tngmodel];
}

struct EffectConfigStats *get_effect_model_stats(ThingModel tngmodel)
{
    if (tngmodel >= EFFECTS_TYPES_MAX)
        return &game.conf.effects_conf.effect_cfgstats[0];
    return &game.conf.effects_conf.effect_cfgstats[tngmodel];
}

short effect_or_effect_element_id(const char *code_name)
{
    if (code_name == NULL)
    {
        return 0;
    }
    if (parameter_is_number(code_name))
    {
        return atoi(code_name);
    }
    short id = get_id(effect_desc, code_name);
    if (id > 0)
    {
        return id;
    }
    id = get_id(effectelem_desc, code_name);
    if (id > 0)
    {
        return -id;
    }
    return 0;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
