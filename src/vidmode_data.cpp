/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file vidmode_data.cpp
 *     Video mode switching/setting function.
 * @par Purpose:
 *     Structures which store video-related parameters.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     05 Jan 2009 - 12 Jan 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "vidmode.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_fmvids.h"
#include "bflib_video.h"
#include "bflib_mouse.h"
#include "bflib_sprite.h"
#include "bflib_dernc.h"
#include "bflib_sprfnt.h"
#include "bflib_filelst.h"

#include "front_simple.h"
#include "front_landview.h"
#include "frontend.h"
#include "game_heap.h"
#include "gui_draw.h"
#include "gui_parchment.h"
#include "engine_redraw.h"
#include "engine_textures.h"
#include "config.h"
#include "lens_api.h"
#include "config_settings.h"
#include "game_legacy.h"
#include "creature_graphics.h"
#include "keeperfx.hpp"
#include "sprites.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

// Declarations for font testing screen
struct TbSetupSprite setup_testfont[] = {
  {&testfont[0],          &testfont_end[0],       &testfont_data[0]},
  {&testfont[1],          &testfont_end[1],       &testfont_data[1]},
  {&testfont[2],          &testfont_end[2],       &testfont_data[2]},
  {&testfont[3],          &testfont_end[3],       &testfont_data[3]},
  {&testfont[4],          &testfont_end[4],       &testfont_data[4]},
  {&testfont[5],          &testfont_end[5],       &testfont_data[5]},
  {&testfont[6],          &testfont_end[6],       &testfont_data[6]},
  {&testfont[7],          &testfont_end[7],       &testfont_data[7]},
  {&testfont[8],          &testfont_end[8],       &testfont_data[8]},
  {&testfont[9],          &testfont_end[9],       &testfont_data[9]},
  {&testfont[10],         &testfont_end[10],      &testfont_data[10]},
  {NULL,                  NULL,                   NULL},
};

struct TbLoadFiles testfont_load_files[] = {
  {"data/frontend.pal",  (unsigned char **)&testfont_palette[0],NULL,                                           0, 0, 0},
  {"data/palette.dat",   (unsigned char **)&testfont_palette[1],NULL,                                           0, 0, 0},
  {"",                    NULL,                                 NULL,                                           0, 0, 0},
};

struct TbLoadFiles gui_load_files_320[] = {
  {"data/slab0-0.dat",   (unsigned char **)&gui_slab,           NULL,                                           0, 0, 0},
  {"",                    NULL,                                 NULL,                                           0, 0, 0},
};

struct TbLoadFiles gui_load_files_640[] = {
  {"data/slab0-1.dat",   (unsigned char **)&gui_slab,           NULL,                                           0, 0, 0},
  {"*B_SCREEN",          (unsigned char **)&hires_parchment,    NULL,                                     640*480, 0, 0},
  {"",                   NULL,                                  NULL,                                           0, 0, 0},
};

#ifdef SPRITE_FORMAT_V2
struct TbLoadFiles front_load_files_minimal_640[] = {
  {"*FE_BACKUP_PAL",       (unsigned char **)&frontend_backup_palette,NULL,                            PALETTE_SIZE, 0, 0},
  {"",                     NULL,                                  NULL,                                           0, 0, 0},
};

#else
struct TbLoadFiles front_load_files_minimal_640[] = {
  {"*FE_BACKUP_PAL",     (unsigned char **)&frontend_backup_palette,NULL,                            PALETTE_SIZE, 0, 0},
  {"",                   NULL,                                  NULL,                                           0, 0, 0},
};

#endif

struct TbLoadFiles legal_load_files[] = {
    {"*PALETTE",         &engine_palette,                        NULL,                               PALETTE_SIZE, 0, 0},
    {"*SCRATCH",         &scratch,                               NULL,                                    0x10000, 1, 0},
    {"",                 NULL,                                   NULL,                                          0, 0, 0},
};

struct TbLoadFilesV2 game_load_files[] = {
    {"*SCRATCH",         &scratch,                                                                   0x10000, nullptr, nullptr},
    {"*TEXTURE_PAGE",    (unsigned char **)&block_mem,  max(sizeof(block_mem), size_t(960*720)), nullptr, nullptr},// Store whole texture image or land view image
#ifdef SPRITE_FORMAT_V2
    {"data/thingspr-32.tab",(unsigned char**)&creature_table,                                              0, nullptr, nullptr},
#else
    {"data/creature.tab",(unsigned char**)&creature_table,                                               0, &creature_table_load_get_size, &creature_table_load_unpack},
#endif
    {"data/palette.dat", &engine_palette,                                                                  0, nullptr, nullptr},
    {"data/bluepal.dat", &blue_palette,                                                                    0, nullptr, nullptr},
    {"data/redpall.dat", &red_palette,                                                                     0, nullptr, nullptr},
    {"data/lightng.pal", &lightning_palette,                                                               0, nullptr, nullptr},
    {"data/dogpal.pal",  &dog_palette,                                                                     0, nullptr, nullptr},
    {"data/vampal.pal",  &vampire_palette,                                                                 0, nullptr, nullptr},
    {"",                 NULL,                                                                             0, nullptr, nullptr},
};

/******************************************************************************/
#ifdef __cplusplus
}
#endif
