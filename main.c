#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "raylib.h"
#include "raymath.h"

#include "bounce.h"
#ifndef DYNAMIC_LOADING
#include "bounce.c"
#endif /* DYNAMIC_LOADING */

#ifndef SHARED_LIB
int main(void)
{
    if (reload_game() == 0)
        return 1;

    int factor        = 60;
    int screen_width  = factor*24; /* 96 px -> 24 px */
    int screen_height = factor*17; /* 68 px -> 17 px */

    SetConfigFlags(FLAG_VSYNC_HINT);
    /* SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_ALWAYS_RUN); */

    /* SetWindowMinSize(screen_width/4.0f, screen_height/4.0f); */
    /* SetWindowMaxSize(screen_width*4.0f, screen_height*4.0f); */
    InitWindow(screen_width, screen_height, "Bounce");

    {
        Image game_icon = LoadImage("./assets/logo.png");
        SetWindowIcon(game_icon);
        UnloadImage(game_icon);
    }

    /* SetExitKey(KEY_NULL); */
    SetTargetFPS(60);

    game_init();

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_R)) {
            void *state = game_pre_reload();
            reload_game();
            game_post_reload(state);
            TraceLog(LOG_INFO, "GAME HAS BEEN RELOADED");
        }

        if (IsKeyPressed(KEY_Q))
            game_reset();

        game_update();
        game_draw();
    }

    game_unload();

    CloseWindow();

    return 0;
}

#ifdef DYNAMIC_LOADING
int reload_game()
{
    const char *game_path = "./libbounce.so";
    void *game = NULL;
    if (game != NULL)
        dlclose(game);

    game = dlopen(game_path, RTLD_NOW);
    if (game == NULL) {
        fprintf(stderr, "ERROR: %s\n", dlerror());
        return 0;
    }

    #define X(name, ...)                                \
    name = dlsym(game, #name);                          \
    if (name == NULL) {                                 \
        fprintf(stderr, "ERROR: %s\n", dlerror());      \
        return 0;                                       \
    }
    SHARED_FUNCTIONS
    #undef X

    return 1;
}
#else
int reload_game() { return 1; }
#endif /* DYNAMIC_LOADING */
#endif /* SHARED_LIB */
