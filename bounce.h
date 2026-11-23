#ifndef _BOUNCE_H_
#define _BOUNCE_H_

#define SHARED_FUNCTIONS             \
    X(game_pre_reload, void*, void)  \
    X(game_post_reload, void, void*) \
    X(game_init, void, void)         \
    X(game_reset, void, void)        \
    X(game_update, void, void)       \
    X(game_draw, void, void)         \
    X(game_unload, void, void)

int reload_game(void);

#ifdef DYNAMIC_LOADING
#define X(name, ret, ...) ret (*name)(__VA_ARGS__) = NULL;
SHARED_FUNCTIONS
#undef X
#else
#define X(name, ret, ...) ret name(__VA_ARGS__);
SHARED_FUNCTIONS
#undef X

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

/* static_assert(COUNT_TEXTURES == 11, "The amount of textures has changed."); */
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
#endif /* DYNAMIC_LOADING */
#endif /* _BOUNCE_H_ */
