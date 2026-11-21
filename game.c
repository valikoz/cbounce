#define SHARED_FUNCTIONS             \
    X(game_pre_reload, void*, void)  \
    X(game_post_reload, void, void*) \
    X(game_init, void, void)         \
    X(game_reset, void, void)        \
    X(game_update, void, void)       \
    X(game_draw, void, void)         \
    X(game_unload, void, void)

#if defined(GAME_SHARED) || defined(STATIC_BUILD)

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <raymath.h>

#define DEFAULT_CAMERA_ZOOM   3.5f
#define DEFAULT_CAMERA_OFFSET 4.0f

#define TARGET_FPS 60

/* Defined map size */
#define TILE_MAP_WIDTH  113
#define TILE_MAP_HEIGHT 8

const int TILE_SIZE  = 8;
const int TILE_ROUND = 7; /* Used in bitwise operation | TILE_SIZE - 1 */

static char map[] = {
    ""
    " BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
    " BB   BBBBBB          BBB         BB         BBB                  BBB      BB  L   BB               BB         BB"
    " BB   BBBBBB          BBB         R          BB                    BB      BB      BB               BB         BB"
    " BB   BBBBBB   BB     BBB         +          BB         C          BB  BB  BB  BB  BB  BB           BB         BB"
    " BB   BBBBBB   BB     BBB  BBBB  BBBB  BBBB  BB                    BB  BB  BB  BB  BB  BB       BBE-BBE-BB     BB"
    " BB       R    BB          BB     BB     BB                            BB  R   BB  R   BB    BBTBB  C   BBTBB  G#"
    " BB       +    BB T        BB  T     T   BB        T  T   T   T        BB  +   BB  +   BB    BBBBB      BBBBB  ##"
    " BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
    ""
};

typedef enum {
    EMPTY,
    BLOCK,
    INVALID,
    MARKED,
    RING_V,
    RING_H,
    RING_V_COLLECTED,
    RING_H_COLLECTED,
    RING_HOLE_V,
    RING_HOLE_H,
    CHECKPOINT,
    CHECKPOINT_ACTIVATED,
    COIN,
    SPIKE,
    PORTAL
} BlockType;

typedef enum {
    TX_BLOCK = 0,
    TX_CHECKPOINT_ACTIVATED,
    TX_CHECKPOINT,
    TX_SMALL_BALL,
    TX_RING_COLLECTED,
    TX_RING,
    TX_SPIKE,
    TX_COIN,
    TX_STATUSLINE_RING,
    TX_STATUSLINE_COIN,
    TX_PORTAL,
    COUNT_TEXTURES,
} Texture_Type;

static_assert(COUNT_TEXTURES == 11, "The amount of textures has changed.");
static Texture2D textures[COUNT_TEXTURES];
static const char *image_paths[COUNT_TEXTURES] = {
    [TX_BLOCK]                = "./assets/block.png",
    [TX_RING]                 = "./assets/ring-not-collected.png",
    [TX_RING_COLLECTED]       = "./assets/ring-collected.png",
    [TX_CHECKPOINT]           = "./assets/checkpoint-not-activated.png",
    [TX_CHECKPOINT_ACTIVATED] = "./assets/checkpoint-activated.png",
    [TX_SPIKE]                = "./assets/spike.png",
    [TX_SMALL_BALL]           = "./assets/small-ball.png",
    [TX_COIN]                 = "./assets/coin.png",
    [TX_STATUSLINE_RING]      = "./assets/statusline-ring.png",
    [TX_STATUSLINE_COIN]      = "./assets/statusline-coin.png",
    [TX_PORTAL]               = "./assets/portal.png"
};

/* static int marginal_block_px[] = { */
/*     1,1,1,1,1,1,1,1, */
/*     1,0,1,0,1,0,1,1, */
/*     1,1,1,1,1,1,1,1, */
/*     1,1,1,1,1,1,1,1, */
/*     1,1,1,1,1,1,1,1, */
/*     1,1,1,1,1,1,1,1 */
/* }; */

/* Object storing inputs for Player */
typedef struct {
    float right;
    float left;
    float up;
    float down;
    bool jump;
    float zoom;
} Input;

typedef struct {
    int width;
    int height;

    Vector2 position;
    Vector2 checkpoint;
    int lifes;
    float direction;
    float maxSpd;
    float acc;
    float dcc;
    float gravity;
    float jumpImpulse;
    float jumpRelease;
    Vector2 velocity;
    /* Carry stored subpixel values */
    float hsp;
    float vsp;

    bool isGrounded;
    bool isJumping;
    bool isFalling;

    bool should_reset;
    bool was_deflated;
    float freeze_time;

    /* Flags for detecting collision */
    bool hitOnFloor;
    bool hitOnCeiling;
    bool hitOnWall;

    Input *control;
} Player;

typedef struct {
    size_t size;
    Font font;
    Player player;
    Camera2D camera;
    int count_rings;
    int count_coins;
    int count_checkpoints;
    bool win;
    int score;
    int tiles[TILE_MAP_WIDTH*TILE_MAP_HEIGHT];
    Input input;
} State;

typedef struct {
    Vector2 position;
    bool visible;
} Coin;

/* Functions */
bool IsKeyPressedOrDown(int key);

#define X(name, ret, ...) ret name(__VA_ARGS__);
SHARED_FUNCTIONS
#undef X

void map_init(void);
void map_update(void);
void map_draw(void);
void input_update(Input *input);
void player_init(Player *player);
void player_reset(Player *player);
void player_reset_move(Player *player);
void player_update(Player *player);
void player_draw(Player *player);
void camera_init(Camera2D *camera);
void camera_update(Camera2D *camera, Player *player);
void margins_draw(void);
void statusline_draw(void);

void Jump(Player *instance);
void PlayerMoveUpdate(Player *intance);
void GetDirection(Player *instance);
void GroundCheck(Player *instance);
void MoveCalc(Player *instance);
void GravityCalc(Player *instance);
void CollisionCheck(Player *instance);
void CollisionHorizontalBlocks(Player *instance);
void CollisionVerticalBlocks(Player *instance);

int MapGetTileWorld(int *entities,int x, int y);
int TileHeight(int x, int y, int tile);

int abs(int x);
float sqrtf(float x);
int ttc_sign(int x);
int ttc_signf(float x);
float ttc_absf(float x);
float ttc_clampf(float value, float min, float max);

/* END OF HEADER */

/* Global Variables Declaration */
static float delta;
static State *s = NULL;

static void load_assets(void)
{
    for (size_t i = 0; i < COUNT_TEXTURES; ++i) {
        Image image = LoadImage(image_paths[i]);

        if (IsImageValid(image)) {
            textures[i] = LoadTextureFromImage(image);
            UnloadImage(image);
            if (!IsTextureValid(textures[i])) assert(true && "Texture not valid.");
        }
    }

    s->font = LoadFont("./assets/nokiafc22.ttf");
    if (!IsFontValid(s->font)) assert(true && "Font is not valid.");
}

static void unload_assets(void)
{
    for (size_t i = 0; i < COUNT_TEXTURES; --i) UnloadTexture(textures[i]);
    UnloadFont(s->font);
}

void *game_pre_reload(void)
{
    unload_assets();
    return s;
}

void game_post_reload(void *state)
{
    s = state;
    if (s->size < sizeof(*s)) {
        TraceLog(LOG_INFO, "Migrating game state schema %zu bytes -> %zu bytes", s->size, sizeof(*s));
        s = realloc(s, sizeof(*s));
        s->size = sizeof(*s);
    }

    load_assets();
}

/* Initialize game variables */
void game_init(void)
{
    s = malloc(sizeof(*s));
    assert(s != NULL);
    memset(s, 0, sizeof(*s));
    s->size = sizeof(*s);

    load_assets();

    memset(&s->input, 0, sizeof(bool)*sizeof(s->input));
    s->input.zoom = 0.0f;

    s->win = false;
    s->count_rings = 0;
    s->count_checkpoints = 0;
    s->score = 0;

    player_init(&s->player);
    camera_init(&s->camera);
    map_init();

    TraceLog(LOG_INFO, "Initialized the game.");
}

void game_reset(void)
{
    memset(&s->input, 0, sizeof(bool)*sizeof(s->input));
    s->input.zoom = 0.0f;

    s->win = false;
    s->count_rings = 0;
    s->count_checkpoints = 0;
    s->score = 0;

    player_reset(&s->player);
    camera_init(&s->camera);
    map_init();

    TraceLog(LOG_INFO, "Reset the game.");
}

void game_unload(void)
{
    unload_assets();
}

/* Update game (one frame) */
void game_update(void)
{
    /* Get time since last frame */
    delta = GetFrameTime();
    delta = delta > 2.0f/TARGET_FPS ? 1.0f/TARGET_FPS : delta;

    map_update();
    input_update(&s->input);
    player_update(&s->player);
    camera_update(&s->camera, &s->player);

    /* If all coins are collected */
    if (s->win) {
        if (IsKeyPressed(KEY_ENTER)) {
            game_reset();
        }
    }
}

void game_draw(void)
{
  BeginDrawing();
  ClearBackground(WHITE);

  BeginMode2D(s->camera);
      map_draw();
      player_draw(&s->player);
      statusline_draw();
      margins_draw();
  EndMode2D();

  /* DrawText(TextFormat("SCORE: %i", s->score), GetScreenWidth()/2 - MeasureText(TextFormat("SCORE: %i", s->score), 40)/2, 50, 40, BLACK); */

  EndDrawing();
}


void map_init(void)
{
    for (int y = 0; y < TILE_MAP_HEIGHT; y++) {
        for (int x = 0; x < TILE_MAP_WIDTH; x++) {
            switch (map[x+y*TILE_MAP_WIDTH]) {
                case 'B': s->tiles[x+y*TILE_MAP_WIDTH] = BLOCK; break;
                case 'R':
                    s->tiles[x+y*TILE_MAP_WIDTH] = RING_V;
                    s->count_rings++;
                    /* score += 50; */
                    break;
                case 'E':
                    s->tiles[x+y*TILE_MAP_WIDTH] = RING_H;
                    s->count_rings++;
                    /* score += 50; */
                    break;
                case '+': s->tiles[x+y*TILE_MAP_WIDTH] = RING_HOLE_V; break;
                case '-': s->tiles[x+y*TILE_MAP_WIDTH] = RING_HOLE_H; break;
                case 'L':
                    s->tiles[x+y*TILE_MAP_WIDTH] = COIN;
                    /* score += 100 */
                    break;
                case 'C':
                    s->tiles[x+y*TILE_MAP_WIDTH] = CHECKPOINT;
                    s->count_checkpoints++;
                    /* score += 20; */
                    break;
                case 'T': s->tiles[x+y*TILE_MAP_WIDTH] = SPIKE;  break;
                case 'G': s->tiles[x+y*TILE_MAP_WIDTH] = PORTAL; break;
                default:  s->tiles[x+y*TILE_MAP_WIDTH] = EMPTY;  break;
            }
        }
    }
}

void player_init(Player *p)
{
    p->maxSpd = 1.5625f*60;
    p->acc = 0.118164f*60*60;
    p->dcc = 0.113281f*60*60;

    float max_height = 6*TILE_SIZE;
    /* p->gravity = 0.363281f*60*60; */
    /* p->jumpImpulse = -5.5625f*60; */
    /* p->jumpRelease = p->jumpImpulse*0.6f; */
    p->gravity = 1024.0f;
    p->jumpImpulse = -sqrtf(2.0f*p->gravity*max_height);
    p->jumpRelease = p->jumpImpulse*0.6f;

    p->width = textures[TX_SMALL_BALL].width;
    p->height = textures[TX_SMALL_BALL].height;

    p->control = &s->input;

    player_reset(p);
}

void player_reset(Player *p)
{
    p->lifes = 3;
    p->checkpoint.x = TILE_SIZE*4-p->width*0.5;
    p->checkpoint.y = TILE_SIZE*2-1;
    player_reset_move(p);
}

void player_reset_move(Player *p)
{
    p->was_deflated = false;
    p->freeze_time = 0.0f;
    p->direction = 1.0;

    p->hsp = 0;
    p->vsp = 0;

    p->position = p->checkpoint;

    p->velocity = (Vector2){ 0.0f, 0.0f };

    p->isGrounded = false;
    p->isJumping = false;
    p->isFalling = false;
}

void camera_init(Camera2D *c) {
    c->rotation = 0.0f;
    c->zoom = DEFAULT_CAMERA_ZOOM;
    c->offset = (Vector2){DEFAULT_CAMERA_OFFSET*c->zoom, DEFAULT_CAMERA_OFFSET*c->zoom};
}

void map_update(void)
{
    return;
}

void input_update(Input *input)
{
    /* Update player controlled Input instance */
    input->left =  (float)(IsKeyDown('H') || IsKeyDown('A') || IsKeyDown(KEY_LEFT));
    input->right = (float)(IsKeyDown('L') || IsKeyDown('D') || IsKeyDown(KEY_RIGHT));

    if (IsKeyPressedOrDown(KEY_W) || IsKeyPressedOrDown(KEY_SPACE) || IsKeyPressedOrDown(KEY_K) || IsKeyPressedOrDown(KEY_UP)) input->jump = true;
    else if (IsKeyPressedOrDown(KEY_W) || IsKeyReleased(KEY_SPACE) || IsKeyReleased(KEY_K) || IsKeyReleased(KEY_UP)) input->jump = false;

    if (IsKeyPressed(KEY_MINUS)) {
        input->zoom -= 0.5f;
    } else if (IsKeyPressed(KEY_EQUAL)) {
        input->zoom += 0.5f;
    } else if (IsKeyPressed(KEY_ZERO)) {
        input->zoom = 0.0f;
    }

    if (MOUSE_CURSOR_RESIZE_NWSE || MOUSE_CURSOR_RESIZE_NESW) {
        /*  */
    }
}

void player_update(Player *p)
{
    if (p->was_deflated) {
        if (p->freeze_time > 0.5f) {
            p->lifes--;
            if (p->lifes == 0) {
                game_reset();
            } else {
                player_reset_move(p);
            }
            textures[TX_SMALL_BALL].height = p->height;
            textures[TX_SMALL_BALL].width  = p->width;
        } else {
            p->freeze_time += delta;
            return;
        }

    }

    GroundCheck(p);
    MoveCalc(p); /* Horizontal acceleration */
    GravityCalc(p); /* Vertical acceleration */
    CollisionHorizontalBlocks(p);
    CollisionVerticalBlocks(p);
}

void camera_update(Camera2D *c, Player *p)
{
    float width = GetScreenWidth();
    float factor = width/24.0f;
    c->zoom = factor*0.23f + p->control->zoom;
    if (p->position.x > width/(c->zoom*2)) {
        if (p->position.x + (width*0.5 - 2*c->offset.x)/(c->zoom) > TILE_SIZE*TILE_MAP_WIDTH) {
            c->target = (Vector2){TILE_SIZE*TILE_MAP_WIDTH - (width - 2*c->offset.x)/c->zoom, 0};
        } else {
            c->target = (Vector2){p->position.x - (width*0.5)/(c->zoom), 0.0f};
        }
    } else {
        c->target = (Vector2){0.0f, 0.0f};
    }
}

void statusline_draw(void)
{
      float height = 11.0f;
      float width  = s->camera.target.x + (GetScreenWidth() - s->camera.offset.x)/s->camera.zoom - 3;

      DrawRectangle((s->camera.target.x), (GetScreenHeight() - 2*s->camera.offset.y )/s->camera.zoom - height, width, height, BLACK);

      int y = (GetScreenHeight() - 2 * s->camera.offset.y) / s->camera.zoom - height;
      int font_size = 11;
      int spacing = 0;
      int offset = 3;
      char buf[1024];
      /* Coins count */
      {
          sprintf(buf, "%dx", s->player.lifes);
          offset += 1;
          DrawTextEx(s->font, buf,
          (Vector2){s->camera.target.x + offset, y - 1},
          font_size, spacing, WHITE);
          offset += MeasureTextEx(s->font, buf, font_size, spacing).x;
      }
      /* Status Line Coin picture */
      {
          offset += 1;
          DrawTextureV(textures[TX_STATUSLINE_COIN],
          (Vector2){s->camera.target.x + offset, y + 2.1f},
          WHITE);
          offset += textures[TX_STATUSLINE_COIN].width;
      }
      /* Rings count */
      {
          offset += 4;
          sprintf(buf, "%dx", s->count_rings);
          DrawTextEx(s->font, buf,
          (Vector2){s->camera.target.x + offset, y - 1},
          font_size, spacing, WHITE);
          offset += MeasureTextEx(s->font, buf, font_size, spacing).x;
      }
      /* Status Line Ring picture */
      {
          offset += 1;
          DrawTextureV(textures[TX_STATUSLINE_RING],
          (Vector2){s->camera.target.x + offset, y + 1.0f},
          WHITE);
          offset += textures[TX_STATUSLINE_RING].width;
      }
      /* Fill + Score */
      {
          sprintf(buf, "%08d", s->score);
          DrawTextEx(s->font, buf,
                     (Vector2){width - MeasureTextEx(s->font, buf, font_size, spacing).x - 1, y - 1},
                     font_size, spacing, WHITE);
      }

      /* DrawTextEx(s->font, TextFormat(" %dX", s->count_rings), (Vector2){ s->camera.target.x + 1, (GetScreenHeight() - 2*s->camera.offset.y)/s->camera.zoom - height - 1 }, font_size, 1, WHITE); */
      /* ImageDrawTextEx(Image *dst, Font font, const char *text, Vector2 position, float fontSize, float spacing, Color tint); */
}

void map_draw(void)
{
    /* DrawLineEx(s->camera.target, Vector2Add(s->camera.target, (Vector2){ (GetScreenWidth() - s->camera.offset.x)/s->camera.zoom, 0.0f}), 4.0f/s->camera.zoom, BLACK); */
    int xi_start = (int)s->camera.target.x/TILE_SIZE;
    int xi_end = (int)(s->camera.target.x + (GetScreenWidth() - s->camera.offset.x)/s->camera.zoom)/TILE_SIZE;
    /* DrawTextureRec(s->map_tx, (Rectangle) { */
        /* .x = s->camera.target.x, */
        /* .y = s->camera.target.y, */
        /* .width  = (GetScreenWidth() - s->camera.offset.x)/s->camera.zoom, */
        /* .height = (GetScreenHeight() - s->camera.offset.y)/s->camera.zoom, */
    /* }, s->camera.target, WHITE); */
    for (size_t y = 0; y < TILE_MAP_HEIGHT; y++) {
        int width = TILE_SIZE;
        for (int x = xi_start; x < xi_end+1; x++) {
            Vector2 offset;
            int start = x*TILE_SIZE;
            int tile = s->tiles[x+y*TILE_MAP_WIDTH];
            switch (tile) {
            case BLOCK:
                DrawTextureV(textures[TX_BLOCK], (Vector2){ .x = start, .y = y*TILE_SIZE }, WHITE);
                break;
            case SPIKE:
                offset = (Vector2){ 2.0f, 2.0f };
                DrawTextureV(textures[TX_SPIKE], (Vector2){ .x = start + offset.x, .y = y*TILE_SIZE + offset.y }, WHITE);
                break;
            case CHECKPOINT:
                offset = (Vector2){ 0.0f, 0.0f };
                DrawTextureV(textures[TX_CHECKPOINT], (Vector2){ .x = start + offset.x, .y = y*TILE_SIZE + offset.y }, WHITE);
                break;
            case CHECKPOINT_ACTIVATED:
                offset = (Vector2){ 0.0f, 2.0f };
                DrawTextureV(textures[TX_CHECKPOINT_ACTIVATED], (Vector2){ .x = start + offset.x, .y = y*TILE_SIZE + offset.y }, WHITE);
                break;
            case RING_V:
                offset = (Vector2){ 2.0f, 1.0f };
                DrawTextureV(textures[TX_RING], (Vector2){ .x = start + offset.x, .y = y*TILE_SIZE + offset.y }, WHITE);
                break;
            case RING_H:
                offset = (Vector2){ 1.0f, -2.0f };
                DrawTextureEx(textures[TX_RING], (Vector2){ .x = start + offset.x, .y = (y+1)*TILE_SIZE + offset.y }, -90.0f, 1 , WHITE);
                break;
            case RING_V_COLLECTED:
                offset = (Vector2){ 2.0f, 1.0f - TILE_SIZE };
                DrawTextureV(textures[TX_RING_COLLECTED], (Vector2){ .x = start + offset.x, .y = y*TILE_SIZE + offset.y }, WHITE);
                break;
            case RING_H_COLLECTED:
                offset = (Vector2){ 1.0f - TILE_SIZE, -2.0f };
                DrawTextureEx(textures[TX_RING_COLLECTED], (Vector2){ .x = start + offset.x, .y = (y+1)*TILE_SIZE + offset.y }, -90.0f, 1, WHITE);
                break;
            case COIN:
                DrawTextureV(textures[TX_COIN], (Vector2){ .x = start, .y = y*TILE_SIZE }, WHITE);
                break;
            case PORTAL:
                offset = (Vector2){ 3.0f, 0.0f };
                DrawTextureV(textures[TX_PORTAL], (Vector2){ .x = start + offset.x, .y = y*TILE_SIZE }, WHITE);
                break;
            }
            width = TILE_SIZE;
            start += width;
        }
    }

}

void player_draw(Player *player)
{
    DrawTextureV(textures[TX_SMALL_BALL], (Vector2){ .x = player->position.x - textures[TX_SMALL_BALL].width*0.5, .y = player->position.y - textures[TX_SMALL_BALL].height + 1 }, RED);
}

void margins_draw(void)
{
  Vector2 margin_start = Vector2Subtract(
      s->camera.target, Vector2Scale(s->camera.offset, 1.0f / s->camera.zoom));
  DrawRectangleLinesEx(
      (Rectangle){.x = margin_start.x,
                  .y = margin_start.y,
                  .width = GetScreenWidth() / s->camera.zoom,
                  .height = GetScreenHeight() / s->camera.zoom
                 },
      DEFAULT_CAMERA_OFFSET,
      WHITE
  );
}

int get_tile_index(int x, int y)
{
    /* Returns tile ID using world position */
    x /= TILE_SIZE;
    y /= TILE_SIZE;

    return x+y*TILE_MAP_WIDTH;
}

/* Function to get tile index using world coordinates */
int MapGetTileWorld(int *tiles, int x, int y)
{
    if (x < 0 || y < 0) return INVALID;

    /* Returns tile ID using world position */
    x /= TILE_SIZE;
    y /= TILE_SIZE;

    if (x > TILE_MAP_WIDTH - 1 || y > TILE_MAP_HEIGHT - 1) return INVALID;
    else return tiles[x+y*TILE_MAP_WIDTH];
}

bool IsKeyPressedOrDown(int key)
{
    return(IsKeyPressed(key) || IsKeyDown(key));
}

/* Check pixel bellow to determine if Player is grounded */
void GroundCheck(Player *p)
{
    int x = (int)p->position.x;
    int y = (int)p->position.y + 1;
    p->isGrounded = false;

    int x_corners[3];

    size_t i = 0;
    x_corners[i++] = x; /* Center point check */
    x_corners[i++] = (x - p->width*0.5f); /* Left bottom corner check */
    x_corners[i]   = (x + p->width*0.5f - 1); /* Right bottom corner check */

    i = 0;
    do {
        switch(MapGetTileWorld(s->tiles, x_corners[i], y)) {
            case BLOCK: p->isGrounded = true; break;
            /* case BLOCK: int h = (y & ~TILE_ROUND) -1; p->isGrounded = (y >= h); break; */
            /* case RING_V: */
            /* case RING_H: */
            case INVALID: TraceLog(LOG_DEBUG, "Player outside the map."); game_reset();
            default: break;
            /* default: p->isGrounded = true; */
        }
        i++;
    } while (!p->isGrounded && i < 3);
}

/* Simplified horizontal acceleration / deacceleration logic */
void MoveCalc(Player *p)
{
    /* Read Input for horizontal movement direction */
    p->direction = (p->control->right - p->control->left);
    /* Check if direction value is above dead zone - direction is held */
    float deadZone = 0.0;
    if (ttc_absf(p->direction) > deadZone) {
        p->velocity.x += p->direction*p->acc*delta; /* v[t] = v[t-1] + a*dt */
        p->velocity.x = ttc_clampf(p->velocity.x, -p->maxSpd, p->maxSpd);
    } else {
        /* No direction means deacceleration */
        float xsp = p->velocity.x;
        if (ttc_absf(0 - xsp) < p->dcc*delta) p->velocity.x = 0;
        else if (xsp > 0) p->velocity.x -= p->dcc*delta;
        else p->velocity.x += p->dcc*delta;
    }
}

/* Gravity calculation and Jump detection */
void GravityCalc(Player *p)
{
    if (p->isGrounded) { /* When the bounce touches the bottom block/s */
        if (p->isJumping) {
            p->isJumping     = false;
            p->control->jump = false;    /* Cancel input button */
        } else if (p->control->jump) {
            Jump(p);
        } else if (p->velocity.y > 5.0f) { /* Should bouncing */
            float factor = 0.6f;
            p->velocity.y *= -factor;
            p->isJumping     = true;
            p->isGrounded    = false;
            p->control->jump = false;
        } else {
            p->velocity.y = 0;
            p->vsp        = 0;
        }
    } else {
        if (p->isJumping) {
            if (!p->control->jump) {
                p->isJumping = false;

                if (p->velocity.y < p->jumpRelease) {
                    p->velocity.y = p->jumpRelease;
                }
            }
        }

        if (p->hitOnCeiling) {
            p->velocity.y = 0;
            p->vsp        = 0;
        }
    }

    p->velocity.y += p->gravity*delta;

    /* Limit falling to negative jump value */
    if (p->velocity.y > -p->jumpImpulse) {
        p->velocity.y = -p->jumpImpulse;
    }
}

/* Set values when jump is activated */
void Jump(Player *p)
{
    p->velocity.y = p->jumpImpulse;
    p->isJumping = true;
    p->isGrounded = false;
}

/* Detect and solve horizontal collision with block tiles */
void CollisionHorizontalBlocks(Player *p)
{
    float dx = p->velocity.x*delta;

    p->hitOnWall = false;

    /* Get bounding box side offset */
    int side;
    if (dx > 0.0)      side =  (p->width>>1);
    else if (dx < 0.0) side = -(p->width>>1);
    else return;

    int x = p->position.x;
    int y = p->position.y;
    int mid = -p->height>>1;
    int top = -p->height + 1;

    /* 3 point check */

    int ypos[3];

    size_t i = 0;
    ypos[i++] = y;
    ypos[i++] = y + mid;
    ypos[i]   = y + top;

    for (i = 0; i < 3; ++i) {
        int x_pos = x + side + dx;
        Vector2 offset;
        int tid;
        switch (MapGetTileWorld(s->tiles, x_pos, ypos[i])) {
        case BLOCK:
            if (dx > 0.0) x = ((x_pos) & ~TILE_ROUND) - side;
            else x = ((x_pos) & ~TILE_ROUND) + TILE_SIZE - side;
            p->position.x = (float)x;
            p->velocity.x = 0.0;
            p->hsp = 0.0;

            p->hitOnWall = true;
            return;
        /* case RING_V: */
        /*     Rectangle player_rect = { */
        /*         p->position.x, */
        /*         p->position.y, */
        /*         p->width, */
        /*         p->height, */
        /*     }; */

        /*     Vector2 offset = { 2.0f, 1.0f }; */

        /*     Rectangle ring_rect = { */
        /*         (x_pos/TILE_SIZE)*TILE_SIZE + offset.x, */
        /*         (ypos[i]/TILE_SIZE)*TILE_SIZE + offset.y, */
        /*         textures[TX_RING].width, */
        /*         textures[TX_RING].height, */
        /*     }; */

        /*     if (CheckCollisionRecs(player_rect, ring_rect)) { */
        /*         p->position.x = (float)x; */
        /*         p->velocity.x = 0.0; */
        /*         p->hsp = 0.0; */

        /*         p->hitOnWall = true; */
        /*         return; */
        /*     } */
        /*     break; */
        case RING_H: break;
        case SPIKE:
            offset = (Vector2){ 2.0f, 0.0f };
            Rectangle spike_rectangle = (Rectangle) {
                (x_pos/TILE_SIZE)*TILE_SIZE + offset.x,
                (ypos[i]/TILE_SIZE)*TILE_SIZE + 2,
                textures[TX_SPIKE].width,
                textures[TX_SPIKE].height
            };
            Rectangle player_rectangle = (Rectangle) {
                s->player.position.x - s->player.width/2 + dx,
                s->player.position.y - s->player.height,
                s->player.width,
                s->player.height
            };

            if (CheckCollisionRecs(spike_rectangle, player_rectangle)) {
                s->player.was_deflated = true;
                textures[TX_SMALL_BALL].width = s->player.width*0.8f;
            }
            break;
        case RING_HOLE_V:
            tid = get_tile_index(x_pos, ypos[i]);
            /* tid -= TILE_MAP_WIDTH; */
            /* assert(tid > TILE_MAP_WIDTH*TILE_MAP_HEIGHT && tid < 0); */
            s->tiles[tid] = RING_V_COLLECTED;
            s->score += 50;
            s->count_rings--;
            break;
        case RING_HOLE_H:
            tid = get_tile_index(x_pos, ypos[i]);
            /* tid -= TILE_MAP_WIDTH; */
            /* assert(tid > TILE_MAP_WIDTH*TILE_MAP_HEIGHT && tid < 0); */
            s->tiles[tid] = RING_H_COLLECTED;
            s->score += 50;
            s->count_rings--;
            break;
        case COIN:
            tid = get_tile_index(x_pos, ypos[i]);
            s->tiles[tid] = EMPTY;
            s->score += 100;
            break;
        case CHECKPOINT:
            offset = (Vector2){ 0.0f, 2.0f };
            tid = get_tile_index(x_pos, ypos[i]);
            s->tiles[tid] = CHECKPOINT_ACTIVATED;
            s->player.checkpoint.x = (x_pos/TILE_SIZE)*TILE_SIZE + offset.x;
            s->player.checkpoint.y = (ypos[i]/TILE_SIZE)*TILE_SIZE + offset.y;
            s->score += 20;
            break;
        default: break;
        }
    }
    p->position.x += dx;
}

/* Detect and solve vertical collision with block tiles */
void CollisionVerticalBlocks(Player *p)
{
    /* Get vertical speed in pixels */
    /* int dy = (int)ttc_clampf(p->velocity.y*delta, -TILE_SIZE, TILE_SIZE); */
    int dy = (int)p->velocity.y*delta;
    p->hitOnCeiling = false;
    p->hitOnFloor = false;

    /* Get bounding box side offset */
    int side = 0;
    if (dy > 0) side = 0;
    else if (dy < 0) side = -p->height + 1;
    else return;

    int x = (int)p->position.x;
    int y = (int)p->position.y;
    int xl = -p->width/2;
    int xr = p->width/2 - 1;

    int xpos[3];

    uint8_t i = 0;
    xpos[i++] = x;
    xpos[i++] = x + xl;
    xpos[i]   = x + xr;

    int dy_max = TILE_SIZE;
    int n = abs(dy)/dy_max;
    int carry = dy;
    if (n > 0) {
        carry = ttc_sign(dy)*(dy%dy_max);
        dy = ttc_sign(dy)*dy_max;
    }
    do { /* n + 1 times */
        int tid;
        if (n < 0) dy = carry;
        int y_pos = y + side + dy;
        for (i = 0; i < 3; ++i) {
            switch (MapGetTileWorld(s->tiles, xpos[i], y_pos)) {
            case BLOCK:
                if (dy > 0) {
                    y = ((y_pos) & ~TILE_ROUND) - 1 - side;
                    p->hitOnFloor = true;
                } else {
                    y = ((y_pos) & ~TILE_ROUND) + TILE_SIZE - side;
                    p->hitOnCeiling = true;
                }

                p->position.y = (float)y;
                return;
            case RING_V: break;
            case RING_H: break;

            case SPIKE:
                int offset = 2;
                Rectangle spike_rectangle = (Rectangle) {
                    (xpos[i]/TILE_SIZE)*TILE_SIZE + offset,
                    (y_pos/TILE_SIZE)*TILE_SIZE + 2,
                    textures[TX_SPIKE].width,
                    textures[TX_SPIKE].height
                };
                Rectangle player_rectangle = (Rectangle) {
                    s->player.position.x - s->player.width*0.5,
                    s->player.position.y - s->player.height + 1 + dy,
                    s->player.width,
                    s->player.height
                };

                if (CheckCollisionRecs(spike_rectangle, player_rectangle)) {
                    textures[TX_SMALL_BALL].height = s->player.height*0.8f;
                    s->player.was_deflated = true;
                }
                break;
            case RING_HOLE_H:
                tid = get_tile_index(xpos[i], y_pos);
                /* tid -= TILE_MAP_WIDTH; */
                /* assert(tid > TILE_MAP_WIDTH*TILE_MAP_HEIGHT && tid < 0); */
                s->tiles[tid] = RING_H_COLLECTED;
                s->score += 50;
                s->count_rings--;
                break;
            case RING_HOLE_V:
                tid = get_tile_index(xpos[i], y_pos);
                /* tid -= TILE_MAP_WIDTH; */
                /* assert(tid > TILE_MAP_WIDTH*TILE_MAP_HEIGHT && tid < 0); */
                s->tiles[tid] = RING_V_COLLECTED;
                s->score += 50;
                s->count_rings--;
                break;
            case COIN:
                tid = get_tile_index(xpos[i], y_pos);
                s->tiles[tid] = EMPTY;
                s->score += 100;
                break;
            case CHECKPOINT:
                tid = get_tile_index(xpos[i], y_pos);
                s->tiles[tid] = CHECKPOINT_ACTIVATED;
                s->player.checkpoint.x = (xpos[i]/TILE_SIZE)*TILE_SIZE;
                s->player.checkpoint.y = (y_pos/TILE_SIZE)*TILE_SIZE;
                s->score += 20;
                break;
            case INVALID:
                TraceLog(LOG_DEBUG, "Player outside the map.");
                game_reset();
                return;
            default: break;
            }
        }
        y += dy;
    } while (n--);

    p->position.y = (float)y;
}

int ttc_sign(int x)
{
    if (x < 0) return -1;
    else if (x == 0) return 0;
    else return 1;
}

/* Return sign of the float as int (-1, 0, 1) */
int ttc_signf(float x)
{
    if (x < 0) return -1;
    else if (x < 0.0001f) return 0;
    else return 1;
}

/* Return absolute value of float */
float ttc_absf(float x)
{
    if (x < 0.0f) x *= -1.0f;
    return x;
}

/* Clamp value between min and max */
float ttc_clampf(float value, float min, float max)
{
    const float res = value < min ? min : value;
    return res > max ? max : res;
}
#endif /* GAME_SHARED */
