/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_dialog.c
 *     Dialog/messagebox support routines.
 * @par Purpose:
 *     Keep SDL dialog integration isolated from core basics/logging code.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "bflib_basics.h"
#include "globals.h"

#include <stdio.h>
#include <SDL2/SDL.h>

#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

short warning_dialog(const char *codefile,const int ecode,const char *message)
{
    LbWarnLog("In source %s:\n %5d - %s\n",codefile,ecode,message);

    const SDL_MessageBoxButtonData buttons[] = {
        { .flags = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, .buttonid = 1, .text = "Ignore" },
        { .flags = SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, .buttonid = 0, .text = "Abort" },
    };

    const SDL_MessageBoxData messageboxdata = {
        .flags = SDL_MESSAGEBOX_WARNING,
        .window = NULL,
        .title = PROGRAM_FULL_NAME,
        .message = message,
        .numbuttons = SDL_arraysize(buttons),
        .buttons = buttons,
        .colorScheme = NULL
    };

    int button = 0;
    SDL_ShowMessageBox(&messageboxdata, &button);
    return button;
}

short error_dialog(const char *codefile,const int ecode,const char *message)
{
    LbErrorLog("In source %s:\n %5d - %s\n",codefile,ecode,message);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, PROGRAM_FULL_NAME, message, NULL);
    return 0;
}

short error_dialog_fatal(const char *codefile,const int ecode,const char *message)
{
    LbErrorLog("In source %s:\n %5d - %s\n",codefile,ecode,message);
    char msg_text[2048];
    snprintf(msg_text, sizeof(msg_text), "%s This error in '%s' makes the program unable to continue. See '%s' for details.", message, codefile, log_file_name);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, PROGRAM_FULL_NAME, msg_text, NULL);
    return 0;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
