#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include <math.h>

#define assert(cond, ...)                                                      \
  { assert_line(__LINE__, cond, __VA_ARGS__) }
#if CONFIGURATION == RELEASE
#undef assert
#define assert(x, ...) (void)(x)
#endif

// Memory Sizes
#define KB(x) (x * 1024ull)
#define MB(x) ((KB(x)) * 1024ull)
#define GB(x) ((MB(x)) * 1024ull)

//

#include <stddef.h>
#include <stdint.h>

#if !defined(__cplusplus)
#if (defined(_MSC_VER) && _MSC_VER < 1800) ||                                  \
    (!defined(_MSC_VER) && !defined(__STDC_VERSION__))
#ifndef true
#define true (0 == 0)
#endif
#ifndef false
#define false (0 != 0)
#endif
typedef unsigned char bool;
#else
#include <stdbool.h>
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool is_power_of_two(uintptr_t x) { return (x & (x - 1)) == 0; }

uintptr_t align_forward(uintptr_t ptr, size_t align) {
  uintptr_t p, a, modulo;

  assert(is_power_of_two(align));

  p = ptr;
  a = (uintptr_t)align;
  // Same as (p % a) but faster as 'a' is a power of two
  modulo = p & (a - 1);

  if (modulo != 0) {
    // If 'p' address is not aligned, push the address to the
    // next value which is aligned
    p += a - modulo;
  }
  return p;
}

#ifndef DEFAULT_ALIGNMENT
#define DEFAULT_ALIGNMENT (2 * sizeof(void *))
#endif

//
float sin_breathe(float time, float rate) {
  return (sin(time * rate) + 1.0) / 2.0;
}
//

typedef struct Arena Arena;
struct Arena {
  unsigned char *buf;
  size_t buf_len;
  size_t prev_offset; // This will be useful for later on
  size_t curr_offset;
};

void arena_init(Arena *a, void *backing_buffer, size_t backing_buffer_length) {
  a->buf = (unsigned char *)backing_buffer;
  a->buf_len = backing_buffer_length;
  a->curr_offset = 0;
  a->prev_offset = 0;
}

void *arena_alloc_align(Arena *a, size_t size, size_t align) {
  // Align 'curr_offset' forward to the specified alignment
  uintptr_t curr_ptr = (uintptr_t)a->buf + (uintptr_t)a->curr_offset;
  uintptr_t offset = align_forward(curr_ptr, align);
  offset -= (uintptr_t)a->buf; // Change to relative offset

  // Check to see if the backing memory has space left
  if (offset + size <= a->buf_len) {
    void *ptr = &a->buf[offset];
    a->prev_offset = offset;
    a->curr_offset = offset + size;

    // Zero new memory by default
    memset(ptr, 0, size);
    return ptr;
  }
  // Return NULL if the arena is out of memory (or handle differently)
  return NULL;
}

// Because C doesn't have default parameters
void *arena_alloc(Arena *a, size_t size) {
  return arena_alloc_align(a, size, DEFAULT_ALIGNMENT);
}

void arena_free(Arena *a, void *ptr) {
  // Do nothing
}

void *arena_resize_align(Arena *a, void *old_memory, size_t old_size,
                         size_t new_size, size_t align) {
  unsigned char *old_mem = (unsigned char *)old_memory;

  assert(is_power_of_two(align));

  if (old_mem == NULL || old_size == 0) {
    return arena_alloc_align(a, new_size, align);
  } else if (a->buf <= old_mem && old_mem < a->buf + a->buf_len) {
    if (a->buf + a->prev_offset == old_mem) {
      a->curr_offset = a->prev_offset + new_size;
      if (new_size > old_size) {
        // Zero the new memory by default
        memset(&a->buf[a->curr_offset], 0, new_size - old_size);
      }
      return old_memory;
    } else {
      void *new_memory = arena_alloc_align(a, new_size, align);
      size_t copy_size = old_size < new_size ? old_size : new_size;
      // Copy across old memory to the new memory
      memmove(new_memory, old_memory, copy_size);
      return new_memory;
    }

  } else {
    assert(0 && "Memory is out of bounds of the buffer in this arena");
    return NULL;
  }
}

// Because C doesn't have default parameters
void *arena_resize(Arena *a, void *old_memory, size_t old_size,
                   size_t new_size) {
  return arena_resize_align(a, old_memory, old_size, new_size,
                            DEFAULT_ALIGNMENT);
}

void arena_free_all(Arena *a) {
  a->curr_offset = 0;
  a->prev_offset = 0;
}

// Extra Features
typedef struct Temp_Arena_Memory Temp_Arena_Memory;
struct Temp_Arena_Memory {
  Arena *arena;
  size_t prev_offset;
  size_t curr_offset;
};

Temp_Arena_Memory temp_arena_memory_begin(Arena *a) {
  Temp_Arena_Memory temp;
  temp.arena = a;
  temp.prev_offset = a->prev_offset;
  temp.curr_offset = a->curr_offset;
  return temp;
}

void temp_arena_memory_end(Temp_Arena_Memory temp) {
  temp.arena->prev_offset = temp.prev_offset;
  temp.arena->curr_offset = temp.curr_offset;
}

//
// Game Code
//

typedef enum EntityArchetype {
  arch_nil = 0,
  arch_player,
  arch_hoe,
  arch_shovel,
  arch_weed,
  arch_rock,
  arch_item_wood,
  arch_item_plant_matter,
  arch_item_stone,
  ARCH_MAX
} EntityArchetype;
char *getArchetypeName(EntityArchetype arch) {
  switch (arch) {
  case arch_player:
    return "player";
  case arch_hoe:
    return "hoe";
  case arch_shovel:
    return "shovel";
  case arch_weed:
    return "weed";
  case arch_rock:
    return "rock";
  case arch_item_wood:
    return "item wood";
  case arch_item_stone:
    return "item stone";
  case arch_item_plant_matter:
    return "item plant matter";
  default:
    return "nil";
  }
};

typedef enum SpriteId {
  sprite_nil = 0,
  sprite_player,
  sprite_hoe,
  sprite_shovel,
  sprite_weed,
  sprite_rock,
  sprite_wood,
  sprite_stone_material,
  sprite_plant_material,
  SPRITE_MAX
} SpriteId;

SpriteId getArchetypeSpriteId(EntityArchetype arch) {
  switch (arch) {
  case arch_player:
    return sprite_player;
  case arch_hoe:
    return sprite_hoe;
  case arch_shovel:
    return sprite_shovel;
  case arch_weed:
    return sprite_weed;
  case arch_rock:
    return sprite_rock;
  case arch_item_wood:
    return sprite_wood;
  case arch_item_stone:
    return sprite_stone_material;
  case arch_item_plant_matter:
    return sprite_plant_material;
  default:
    return sprite_nil;
  }
};

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
  int health;
  SpriteId sprite_id;
  bool is_item;
  bool is_destroyable_world_item;
} Entity;

typedef enum GameState {
  state_nil = 0,
  state_start,
  state_pause,
  state_play,
  state_gameover,
  STATE_MAX
} GameState;

#define MAX_ENTITY_COUNT 1024
#define MAX_INVENTORY_COUNT ARCH_MAX
typedef struct World {
  Entity entities[MAX_ENTITY_COUNT];
  int inventory[MAX_INVENTORY_COUNT];
  int timeInMinutes;
  double timeElapsed;
  int dayCount;
  float energy;
  float hydration;
  GameState state;
  float screenHeight;
  float screenWidth;
  Entity *player;
  Color backgroundColor;
  Camera2D camera;
} World;

World *world = 0;

Entity *entity_create() {
  Entity *entity_found = 0;
  for (int i = 0; i < MAX_ENTITY_COUNT; i++) {
    Entity *existing_entity = &(world->entities[i]);
    if (!existing_entity->is_valid) {
      entity_found = existing_entity;
      break;
    }
  }
  assert(entity_found, "No more free entities!");

  memset(entity_found, 0, sizeof(Entity));

  entity_found->is_valid = true;
  return entity_found;
}

const float tileWidth = 40;

int world_pos_to_tile_pos(float world_pos) {
  return roundf(world_pos / tileWidth);
}

float tile_pos_to_world_pos(int tile_pos) { return tileWidth * tile_pos; }

Vector2 round_v2_to_tile(Vector2 v2) {
  v2.x = tile_pos_to_world_pos(world_pos_to_tile_pos(v2.x));
  v2.y = tile_pos_to_world_pos(world_pos_to_tile_pos(v2.y));
  return v2;
}

const int playerHealth = 5;
const float playerPickupRadius = 20.0;
const int rockHealth = 3;
const int weedHealth = 2;

Entity *SetupPlayer(Vector2 pos) {
  Entity *entity = entity_create();

  entity->pos = round_v2_to_tile(pos);
  entity->pos.y -= tileWidth * 0.5;
  entity->health = playerHealth;
  /* entity->pos.x -= tileWidth * 0.5; */

  entity->archetype = arch_player;
  entity->sprite_id = sprite_player;
  return entity;
}
Camera2D SetupCamera(Vector2 initialPlayerPosition) {
  Camera2D camera = {0};

  camera.target = world->player->pos;
  camera.offset =
      (Vector2){world->screenWidth / 2.0f, world->screenHeight / 2.0f};
  camera.rotation = 0.0f;
  camera.zoom = 1.25f;

  return camera;
}

void SetupRock(Vector2 pos) {
  Entity *entity = entity_create();

  entity->pos = round_v2_to_tile(pos);
  entity->pos.y -= tileWidth * 0.5;
  /* entity->pos.x += tileWidth * 0.125; */
  entity->health = rockHealth;
  entity->is_destroyable_world_item = true;

  entity->archetype = arch_rock;
  entity->sprite_id = sprite_rock;
}

void SetupWeed(Vector2 pos) {
  Entity *entity = entity_create();

  entity->pos = round_v2_to_tile(pos);
  entity->pos.y -= tileWidth * 0.5;
  entity->pos.x += tileWidth * 0.25;
  entity->health = weedHealth;
  entity->is_destroyable_world_item = true;

  entity->archetype = arch_weed;
  entity->sprite_id = sprite_weed;
}

void SetupItemWood(Vector2 pos) {
  Entity *entity = entity_create();

  entity->pos = round_v2_to_tile(pos);
  entity->pos.y -= tileWidth * 0.5;
  entity->pos.x += tileWidth * 0.5;

  entity->is_item = true;

  entity->archetype = arch_item_wood;
  entity->sprite_id = sprite_wood;
}

Vector2 v2(float x, float y) { return (Vector2){x, y}; }

void UpdateCameraCenterSmoothFollow(Camera2D *camera, Entity *player,
                                    float delta, int width, int height) {
  static float minSpeed = 30;
  static float minEffectLength = 5;
  static float fractionSpeed = 0.9f;

  camera->offset = (Vector2){width / 2.0f, height / 2.0f};
  Vector2 diff = Vector2Subtract(player->pos, camera->target);
  float length = Vector2Length(diff);

  if (length > minEffectLength) {
    float speed = fmaxf(fractionSpeed * length, minSpeed);
    camera->target =
        Vector2Add(camera->target, Vector2Scale(diff, speed * delta / length));
  }
}

void InitWorld(World *world) {
  world->state = state_start;
  world->dayCount = 0;
  world->timeElapsed = 0;
  world->timeInMinutes = 720; // 12noon
  world->energy = 100;
  world->hydration = 100;
  world->screenWidth = 1280;
  world->screenHeight = 720;
  world->backgroundColor = GetColor(0x4b692fff);

  Vector2 initialPlayerPosition = {world->screenWidth / 2.0f,
                                   world->screenHeight / 2.0f};
  world->player = SetupPlayer(initialPlayerPosition);
  world->camera = SetupCamera(initialPlayerPosition);
};

#define ARENA_SIZE MB(20)
/* static unsigned char backing_buffer[ARENA_SIZE]; */

void UpdateStartState(World *world);
void UpdatePlayState(World *world, Arena *arena);
/* void UpdatePauseState(World *world); */
void UpdateGameOverState(World *world, Arena *arena);
void UpdateState(World *world, Arena *arena) {
  switch (world->state) {
  case state_start:
    return UpdateStartState(world);
  case state_play:
    return UpdatePlayState(world, arena);
  case state_gameover:
    return UpdateGameOverState(world, arena);
  default:
    return;
  }
};

static char gameTitle[16] = "Farm To Table";

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void) {
  // Initialization
  //--------------------------------------------------------------------------------------

  void *backing_buffer = malloc(ARENA_SIZE);
  Arena arena = {0};
  arena_init(&arena, backing_buffer, ARENA_SIZE);
  world = arena_alloc(&arena, sizeof(World));
  /* printf("FIRST ARENA ALLOC: current offset - %lu, previous offset - %lu, "
   */
  /*        "Arena Size - %llu", */
  /*        a.curr_offset, a.prev_offset, ARENA_SIZE); */
  InitWorld(world);

  InitWindow(world->screenWidth, world->screenHeight, gameTitle);

  SetTargetFPS(60); // Set our game to run at 60 frames-per-second

  sprites[sprite_nil] = LoadTexture("assets/sprites/nil_texture.png");

  sprites[sprite_player] = LoadTexture("assets/sprites/player.png");
  sprites[sprite_weed] = LoadTexture("assets/sprites/weed.png");
  sprites[sprite_rock] = LoadTexture("assets/sprites/rock.png");

  sprites[sprite_shovel] = LoadTexture("assets/sprites/shovel.png");
  sprites[sprite_hoe] = LoadTexture("assets/sprites/hoe.png");

  sprites[sprite_wood] = LoadTexture("assets/sprites/wood.png");
  sprites[sprite_stone_material] =
      LoadTexture("assets/sprites/stone_material.png");
  sprites[sprite_plant_material] =
      LoadTexture("assets/sprites/plant_material.png");

  for (int i = 0; i < 10; i++) {
    SetupRock(v2(i * 100, i * 100));
  }
  for (int i = 0; i < 10; i++) {
    SetupWeed(v2(i * 150, i * 322));
  }

  //--------------------------------------------------------------------------------------
  // Main game loop
  while (!WindowShouldClose()) // Detect window close button or ESC key
  {
    UpdateState(world, &arena);
  }
  //--------------------------------------------------------------------------------------

  // De-Initialization
  //--------------------------------------------------------------------------------------
  CloseWindow(); // Close window and OpenGL context
  free(backing_buffer);
  //--------------------------------------------------------------------------------------

  return 0;
}

void UpdateStartState(World *world) {
  if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    world->state = state_play;
  }

  BeginDrawing();

  ClearBackground(world->backgroundColor);
  Vector2 center = {world->screenWidth / 2.0f, world->screenHeight / 2.0f};
  DrawText(gameTitle, center.x, center.y, 24, BLACK);

  char click[16] = "Click To Start";
  DrawText(click, center.x, center.y + 20, 24, BLACK);

  EndDrawing();
}

void UpdateGameOverState(World *world, Arena *arena) {
  BeginDrawing();

  ClearBackground(world->backgroundColor);
  Vector2 center = {world->screenWidth / 2.0f, world->screenHeight / 2.0f};
  DrawText(gameTitle, center.x, center.y, 24, BLACK);

  Temp_Arena_Memory tmp = temp_arena_memory_begin(arena);
  char *resultsStr = arena_alloc(arena, 128);
  sprintf(resultsStr, "You Survived %d days, %02d hours, and %02d minutes",
          world->dayCount + 1, world->timeInMinutes / 60,
          world->timeInMinutes % 60);
  DrawText(resultsStr, center.x, center.y + 20, 24, BLACK);
  temp_arena_memory_end(tmp);

  char click[32] = "TODO: Click To Play Again";
  DrawText(click, center.x, center.y + 40, 24, BLACK);

  EndDrawing();
};

void UpdatePlayState(World *world, Arena *arena) {
  const float deltaT = GetFrameTime();
  const float playerSpeed = 300;
  const float defaultFatigueRate = 1;

  if (world->energy <= 0) {
    world->state = state_gameover;
    return;
  }

  // TODO: maybe make the clock only update every 5 or ten minutes like in
  // SDV 1 seconds = 1 minute, 24 minutes in game is a 24 hour day
  const float deltaTScale = 1;
  world->timeElapsed += deltaT;
  if (world->timeElapsed >= deltaTScale) {
    world->timeElapsed = 0;
    world->timeInMinutes += 1;
    world->energy -= defaultFatigueRate;
  }
  if (world->timeInMinutes >= (60 * 24)) {
    world->dayCount += 1;
    world->timeInMinutes = 0;
  }

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

  world->player->pos = Vector2Add(world->player->pos, movement);
  UpdateCameraCenterSmoothFollow(&world->camera, world->player, deltaT,
                                 world->screenWidth, world->screenHeight);

  Vector2 mouseScreenPosition = GetMousePosition();
  Vector2 mouseWorldPosition =
      GetScreenToWorld2D(mouseScreenPosition, world->camera);
  // NOTE: alignment didnt feel right before - this slight adjustment fixes
  mouseWorldPosition = Vector2Subtract(mouseWorldPosition, v2(10, 10));
  Vector2 mouseTilePosition = round_v2_to_tile(mouseWorldPosition);

  const float scale = 4.0;

  //----------------------------------------------------------------------------------

  // Draw
  //----------------------------------------------------------------------------------
  BeginDrawing();

  ClearBackground(world->backgroundColor);

  BeginMode2D(world->camera);

  // TODO: maybe this isn't the right way to approach this
  // Draw the 3d grid, rotated 90 degrees and centered around 0,0
  // just so we have something in the XY plane
  rlPushMatrix();
  rlTranslatef(0, 25 * tileWidth, 0);
  rlRotatef(90, 1, 0, 0);
  // I think the 1000 here is how many tiles
  DrawGrid(1000, tileWidth);
  rlPopMatrix();

  Rectangle mouseRectangle = (Rectangle){
      mouseTilePosition.x, mouseTilePosition.y, tileWidth, tileWidth};
  DrawRectangleRec(mouseRectangle, RED);

  for (int i = 0; i < MAX_ENTITY_COUNT; i++) {
    Entity *existing_entity = &world->entities[i];
    if (existing_entity && existing_entity->is_valid) {
      Texture2D sprite = sprites[existing_entity->sprite_id];

      Rectangle entityBounds = {existing_entity->pos.x, existing_entity->pos.y,
                                sprite.width * scale, sprite.height * scale};

      // Check if point is inside rectangle
      // TODO: instead - check if the mouse tile is the same as the entity's
      // tile
      bool mouseInBounds = CheckCollisionRecs(mouseRectangle, entityBounds);

      /* Color col = RAYWHITE; */
      /* if (mouseInBounds) { */
      /*   col = RED; */
      /* } */

      /* Debug Rectangles  */
      /* DrawRectangleRec(bounds, col); */

      // make collectibles bounce
      Vector2 translation = v2(0, 0);
      if (existing_entity->is_item) {
        translation.y = sin_breathe(GetTime(), 5.0) * 10;
      }

      DrawTextureEx(sprite, Vector2Add(existing_entity->pos, translation), 0.0f,
                    scale, RAYWHITE);

      if (existing_entity->is_destroyable_world_item && mouseInBounds &&
          IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        existing_entity->health -= 1;
        if (existing_entity->health <= 0) {
          existing_entity->is_valid = false;
          if (existing_entity->archetype == arch_weed) {
            SetupItemWood(existing_entity->pos);
          }
        }
      }

      if (existing_entity->is_item &&
          fabs(Vector2Distance(world->player->pos, existing_entity->pos)) <
              playerPickupRadius) {
        existing_entity->is_valid = false;
        world->inventory[existing_entity->archetype] += 1;
      }

      // DEBUG - print all entities' positions below them
      /* char posStr[100]; */
      /* sprintf(posStr, "(%.2f, %.2f)", existing_entity->pos.x, */
      /*         existing_entity->pos.y); */
      /* DrawText(posStr, existing_entity->pos.x, existing_entity->pos.y +
       * 30,
       */
      /*          20, RED); */
    }
  }

  EndMode2D();

  int titleFontX = world->screenWidth - 300;
  int titleFontY = 10;
  int titleFontSize = 40;
  DrawText(gameTitle, titleFontX, titleFontY, titleFontSize, RED);

  Temp_Arena_Memory tmp = temp_arena_memory_begin(arena);
  // NOTE: not sure that 16 is big enough once the integer gets up to a
  // certain size(?)
  char *timeStr = arena_alloc(arena, 16);
  /* printf("TMP ARENA ALLOCD: current offset - %lu, previous offset - %lu,
   * "
   */
  /*        "Arena Size - %llu", */
  /*        a.curr_offset, a.prev_offset, ARENA_SIZE); */
  /* char timeStr[16]; */
  sprintf(timeStr, "Day %d, %02d:%02d", world->dayCount + 1,
          world->timeInMinutes / 60, world->timeInMinutes % 60);
  DrawText(timeStr, titleFontX, titleFontY + 30, titleFontSize, BLACK);
  temp_arena_memory_end(tmp);
  /* printf("TMP ARENA RELEASED: current offset - %lu, previous offset -
   * %lu,
   * " */
  /*        "Arena Size - %llu", */
  /*        a.curr_offset, a.prev_offset, ARENA_SIZE); */

  // TODO: is there any reason to put these strings in the temp_arena rather
  // than the stack
  DrawText("Inventory:", titleFontX, titleFontY + 50, titleFontSize, RED);
  for (int i = 0; i < MAX_INVENTORY_COUNT; i++) {
    if (world->inventory[i] > 0) {

      char posStr[1000];
      sprintf(posStr, "%s: %d", getArchetypeName(i), world->inventory[i]);
      DrawText(posStr, titleFontX, titleFontY + 30 * (i + 2), titleFontSize,
               RED);
    }
  }

  DrawRectangle(titleFontX, titleFontY + 200, 50, world->energy * 5,
                world->energy > 30 ? GREEN : RED);

  /* Debug Render Mouse Position */
  /* char posStr[1000]; */
  /* sprintf(posStr, "(%.2f, %.2f)", mouseWorldPosition.x,
   * mouseWorldPosition.y); */
  /* DrawText(posStr, mouseWorldPosition.x, mouseWorldPosition.y, 20,
     RED); */

  EndDrawing();
  //----------------------------------------------------------------------------------
}
