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
//api.c functions
void api_check_var_update(){}
void api_event(const char *event_name){}
void api_update_server(){}
void api_close_server(){}
int api_init_server(){
    return 0;
}

/******************************************************************************/
//net_resync.cpp functions
TbBool detailed_multiplayer_logging = false;

void resync_game(void) {}
void LbNetwork_TimesyncBarrier(void) {}

/******************************************************************************/
