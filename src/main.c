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

// TODO
void UpdateCameraCenterSmoothFollow(Camera2D *camera, Vector2 playerPos,
                                    float delta, int width, int height) {
  static float minSpeed = 5;
  static float minEffectLength = 1000;
  static float fractionSpeed = 0.5f;

  camera->offset = (Vector2){width / 2.0f, height / 2.0f};
  Vector2 diff = Vector2Subtract(playerPos, camera->target);
  float length = Vector2Length(diff);

  if (length > minEffectLength) {
    float speed = fmaxf(fractionSpeed * length, minSpeed);
    camera->target =
        Vector2Add(camera->target, Vector2Scale(diff, speed * delta / length));
  }
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

  Camera2D camera = {0};
  camera.target = (Vector2){playerPosition.x, playerPosition.y};
  camera.offset = (Vector2){screenWidth / 2.0f, screenHeight / 2.0f};
  camera.rotation = 0.0f;
  camera.zoom = 1.25f;

  SetTargetFPS(60); // Set our game to run at 60 frames-per-second

  sprites[sprite_player] = LoadTexture("assets/player.png");
  sprites[sprite_weed] = LoadTexture("assets/weed.png");
  sprites[sprite_rock] = LoadTexture("assets/rock.png");

  SetupRock((Vector2){50, 80});
  SetupRock((Vector2){100, 100});
  SetupRock((Vector2){500, 500});
  SetupWeed((Vector2){20, 400});
  SetupWeed((Vector2){400, 50});
  SetupWeed((Vector2){500, 54});

  //--------------------------------------------------------------------------------------

  // Main game loop
  while (!WindowShouldClose()) // Detect window close button or ESC key
  {
    const float deltaT = GetFrameTime();
    const float playerSpeed = 300;

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
    UpdateCameraCenterSmoothFollow(&camera, playerPosition, deltaT, screenWidth,
                                   screenHeight);
    //
    //
    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

    ClearBackground(background);

    BeginMode2D(camera);

    DrawTextureEx(sprites[sprite_player], playerPosition, 0.0f, 4.0f, RAYWHITE);
    camera.target = playerPosition;

    for (int i = 0; i < MAX_ENTITY_COUNT; i++) {
      Entity *existing_entity = &world.entities[i];
      if (existing_entity && existing_entity->is_valid) {
        DrawTextureEx(sprites[existing_entity->sprite_id], existing_entity->pos,
                      0.0f, 4.0f, RAYWHITE);
      }
    }

    EndMode2D();

    DrawText("Farm 2 Table", screenWidth - 300, 10, 40, RED);

    EndDrawing();
    //----------------------------------------------------------------------------------
  }

  // De-Initialization
  //--------------------------------------------------------------------------------------
  CloseWindow(); // Close window and OpenGL context
  //--------------------------------------------------------------------------------------

  return 0;
}
