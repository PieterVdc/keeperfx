

#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include "discord_game_sdk.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <string.h>
#endif


#define DISCORD_REQUIRE(x) assert(x == DiscordResult_Ok)



struct Application {
    struct IDiscordCore* core;
    struct IDiscordUserManager* users;
    struct IDiscordAchievementManager* achievements;
    struct IDiscordActivityManager* activities;
    struct IDiscordRelationshipManager* relationships;
    struct IDiscordApplicationManager* application;
    struct IDiscordLobbyManager* lobbies;
    DiscordUserId user_id;
};

struct Application app;

void UpdateActivityCallback(void* data, enum EDiscordResult result)
{
    DISCORD_REQUIRE(result);
}

void OnOAuth2Token(void* data, enum EDiscordResult result, struct DiscordOAuth2Token* token)
{
    if (result == DiscordResult_Ok) {
       // ERRORLOG("OAuth2 token: %s\n", token->access_token);
    }
    else {
       // ERRORLOG("GetOAuth2Token failed with %d\n", (int)result);
    }
}

void OnUserUpdated(void* data)
{
    struct Application* app = (struct Application*)data;
    struct DiscordUser user;
    app->users->get_current_user(app->users, &user);
    app->user_id = user.id;
}

void initialise_discord()
{
    
    memset(&app, 0, sizeof(app));

    struct IDiscordUserEvents users_events;
    memset(&users_events, 0, sizeof(users_events));
    users_events.on_current_user_update = OnUserUpdated;

    struct IDiscordActivityEvents activities_events;
    memset(&activities_events, 0, sizeof(activities_events));
/*
    struct IDiscordRelationshipEvents relationships_events;
    memset(&relationships_events, 0, sizeof(relationships_events));
    relationships_events.on_refresh = OnRelationshipsRefresh;
*/
    struct DiscordCreateParams params;
    DiscordCreateParamsSetDefault(&params);
    params.client_id = 945341607178289162;
    params.flags = DiscordCreateFlags_Default;
    params.event_data = &app;
    params.activity_events = &activities_events;
    /*
    params.relationship_events = &relationships_events;
    */
    params.user_events = &users_events;
    DISCORD_REQUIRE(DiscordCreate(DISCORD_VERSION, &params, &app.core));

    app.users = app.core->get_user_manager(app.core);
  
  //  app.achievements = app.core->get_achievement_manager(app.core);
    app.activities = app.core->get_activity_manager(app.core);
      /*

    app.lobbies = app.core->get_lobby_manager(app.core);

    app.lobbies->connect_lobby_with_activity_secret(
      app.lobbies, "invalid_secret", &app, OnLobbyConnect);
*/
    app.application = app.core->get_application_manager(app.core);
    app.application->get_oauth2_token(app.application, &app, OnOAuth2Token);

    DiscordBranch branch;
    app.application->get_current_branch(app.application, &branch);
    printf("Current branch %s\n", branch);

    DiscordCreate(DISCORD_VERSION, &params, &app.core);

            
    
    for (;;) {
        DISCORD_REQUIRE(app.core->run_callbacks(app.core));

        #ifdef _WIN32
                Sleep(16);
        #else
                usleep(16 * 1000);
        #endif
    }

    

}

void discord_callback() 
{

DISCORD_REQUIRE(app.core->run_callbacks(app.core));
    
}

