#include "raylib.h"
#include "raymath.h"

#define assert(cond, ...)                                                      \
  { assert_line(__LINE__, cond, __VA_ARGS__) }
#if CONFIGURATION == RELEASE
#undef assert
#define assert(x, ...) (void)(x)
#endif

typedef enum EntityArchetype {
  arch_nil = 0,
  arch_player = 1,
  arch_hoe = 2,
  arch_shovel = 3,
  arch_weed = 4,
  arch_rock = 5,
} EntityArchetype;

typedef enum SpriteId {
  sprite_nil,
  sprite_player,
  sprite_hoe,
  sprite_shovel,
  sprite_weed,
  sprite_rock,
  SPRITE_MAX
} SpriteId;

Texture2D sprites[SPRITE_MAX];
Texture2D *get_sprite(SpriteId id) {
  if (id >= 0 && id < SPRITE_MAX) {
    return &sprites[id];
  }
  return &sprites[0];
}

typedef struct Entity {
  EntityArchetype archetype;
  Vector2 pos;
  bool is_valid;
  SpriteId sprite_id;
} Entity;

#define MAX_ENTITY_COUNT 1024
typedef struct World {
  Entity entities[MAX_ENTITY_COUNT];
} World;

// {0} automatically zeros the data allocated for the struct
static World world = {0};

Entity *entity_create() {
  Entity *entity_found = 0;
  for (int i = 0; i < MAX_ENTITY_COUNT; i++) {
    Entity *existing_entity = &(world.entities[i]);
    if (!existing_entity->is_valid) {
      entity_found = existing_entity;
      break;
    }
  }
  assert(entity_found, "No more free entities!");
  entity_found->is_valid = true;
  return entity_found;
}

void SetupRock(Vector2 pos) {
  Entity *entity = entity_create();
  entity->pos = pos;
  entity->archetype = arch_rock;
  entity->sprite_id = sprite_rock;
}
void SetupWeed(Vector2 pos) {
  Entity *entity = entity_create();
  entity->pos = pos;
  entity->archetype = arch_weed;
  entity->sprite_id = sprite_weed;
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void) {
  // Initialization
  //--------------------------------------------------------------------------------------
  const char gameTitle[] = "Farm To Table";
  const int screenWidth = 1280;
  const int screenHeight = 720;
  Color background = GetColor(0x4b692fff);

  /* world = alloc(get_heap_allocator(), sizeof(world)) */

  InitWindow(screenWidth, screenHeight, gameTitle);

  Vector2 playerPosition = {(float)screenWidth / 2, (float)screenHeight / 2};

  SetTargetFPS(60); // Set our game to run at 60 frames-per-second

  sprites[sprite_player] = LoadTexture("assets/player.png");
  sprites[sprite_weed] = LoadTexture("assets/weed.png");
  sprites[sprite_rock] = LoadTexture("assets/rock.png");

  SetupRock((Vector2){0, 0});
  SetupRock((Vector2){100, 100});
  SetupRock((Vector2){500, 500});
  SetupWeed((Vector2){20, 20});
  SetupWeed((Vector2){50, 50});
  SetupWeed(
      (Vector2){500, 500}); // todo: why isnt ths at the same location as rock?

  //--------------------------------------------------------------------------------------

  // Main game loop
  while (!WindowShouldClose()) // Detect window close button or ESC key
  {
    const float deltaT = GetFrameTime();
    const float playerSpeed = 80.0f;

    // Update
    //----------------------------------------------------------------------------------
    Vector2 movement = {.x = 0, .y = 0};
    if (IsKeyDown(KEY_RIGHT))
      movement.x += 1;
    if (IsKeyDown(KEY_LEFT))
      movement.x -= 1;
    if (IsKeyDown(KEY_UP))
      movement.y -= 1;
    if (IsKeyDown(KEY_DOWN))
      movement.y += 1;

    movement = Vector2Normalize(movement);
    movement = Vector2Scale(movement, deltaT * playerSpeed);

    playerPosition = Vector2Add(playerPosition, movement);
    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

    ClearBackground(background);

    /* DrawText("move the player with arrow keys", 10, 10, 20, DARKGRAY); */

    DrawTextureEx(sprites[sprite_player], playerPosition, 0.0f, 4.0f, RAYWHITE);

    for (int i = 0; i < MAX_ENTITY_COUNT; i++) {
      Entity *existing_entity = &world.entities[i];
      if (existing_entity && existing_entity->is_valid) {
        DrawTextureEx(sprites[existing_entity->sprite_id], existing_entity->pos,
                      0.0f, 4.0f, RAYWHITE);
      }
    }

    EndDrawing();
    //----------------------------------------------------------------------------------
  }

  // De-Initialization
  //--------------------------------------------------------------------------------------
  CloseWindow(); // Close window and OpenGL context
  //--------------------------------------------------------------------------------------

  return 0;
}
