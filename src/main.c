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

World *world = 0;
// {0} automatically zeros the data allocated for the struct
/* static World world = {0}; */

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

Entity *SetupPlayer(Vector2 pos) {
  Entity *entity = entity_create();

  entity->pos = round_v2_to_tile(pos);
  entity->pos.y -= tileWidth * 0.5;
  /* entity->pos.x -= tileWidth * 0.5; */

  entity->archetype = arch_player;
  entity->sprite_id = sprite_player;
  return entity;
}
void SetupRock(Vector2 pos) {
  Entity *entity = entity_create();

  entity->pos = round_v2_to_tile(pos);
  entity->pos.y -= tileWidth * 0.5;
  /* entity->pos.x += tileWidth * 0.125; */

  entity->archetype = arch_rock;
  entity->sprite_id = sprite_rock;
}
void SetupWeed(Vector2 pos) {
  Entity *entity = entity_create();

  entity->pos = round_v2_to_tile(pos);
  entity->pos.y -= tileWidth * 0.5;
  entity->pos.x += tileWidth * 0.25;

  entity->archetype = arch_weed;
  entity->sprite_id = sprite_weed;
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

#define ARENA_SIZE MB(20)
/* static unsigned char backing_buffer[ARENA_SIZE]; */

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

  void *backing_buffer = malloc(ARENA_SIZE);
  Arena a = {0};
  arena_init(&a, backing_buffer, ARENA_SIZE);

  world = arena_alloc(&a, sizeof(&world));

  InitWindow(screenWidth, screenHeight, gameTitle);

  Entity *player =
      SetupPlayer((Vector2){screenWidth / 2.0f, screenHeight / 2.0f});

  Camera2D camera = {0};
  camera.target = player->pos;
  camera.offset = (Vector2){screenWidth / 2.0f, screenHeight / 2.0f};
  camera.rotation = 0.0f;
  camera.zoom = 1.25f;

  SetTargetFPS(60); // Set our game to run at 60 frames-per-second

  sprites[sprite_player] = LoadTexture("assets/player.png");
  sprites[sprite_weed] = LoadTexture("assets/weed.png");
  sprites[sprite_rock] = LoadTexture("assets/rock.png");

  for (int i = 0; i < 10; i++) {
    SetupRock(v2(i * 100, i * 100));
  }
  for (int i = 0; i < 10; i++) {
    SetupWeed(v2(i * 20, i * 15));
  }

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

    player->pos = Vector2Add(player->pos, movement);
    UpdateCameraCenterSmoothFollow(&camera, player, deltaT, screenWidth,
                                   screenHeight);

    Vector2 mouseScreenPosition = GetMousePosition();
    Vector2 mouseWorldPosition =
        GetScreenToWorld2D(mouseScreenPosition, camera);
    // NOTE: alignment didnt feel right before - this slight adjustment fixes
    mouseWorldPosition = Vector2Subtract(mouseWorldPosition, v2(10, 10));
    Vector2 mouseTilePosition = round_v2_to_tile(mouseWorldPosition);

    const float scale = 4.0;

    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

    ClearBackground(background);

    BeginMode2D(camera);

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

        Rectangle entityBounds = {existing_entity->pos.x,
                                  existing_entity->pos.y, sprite.width * scale,
                                  sprite.height * scale};

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

        DrawTextureEx(sprite, existing_entity->pos, 0.0f, scale, RAYWHITE);

        if (existing_entity->archetype != arch_player && mouseInBounds &&
            IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
          existing_entity->is_valid = false;
        }

        // TODO: I think this should be a temp arena allocation - it's only
        // needed for this frame -- or is it fine that it just comes from the
        // stack - maybe with too many objects I'd get a stack overflow here?

        // DEBUG - print all entities' positions below them
        /* char posStr[100]; */
        /* sprintf(posStr, "(%.2f, %.2f)", existing_entity->pos.x, */
        /*         existing_entity->pos.y); */
        /* DrawText(posStr, existing_entity->pos.x, existing_entity->pos.y + 30,
         */
        /*          20, RED); */
      }
    }

    EndMode2D();

    int titleFontX = screenWidth - 300;
    int titleFontY = 10;
    int titleFontSize = 40;
    DrawText("Farm 2 Table", titleFontX, titleFontY, titleFontSize, RED);

    /* Debug Render Mouse Position */
    /* char posStr[1000]; */
    /* sprintf(posStr, "(%.2f, %.2f)", mouseWorldPosition.x,
     * mouseWorldPosition.y); */
    /* DrawText(posStr, mouseWorldPosition.x, mouseWorldPosition.y, 20, RED); */

    EndDrawing();
    //----------------------------------------------------------------------------------
  }

  // De-Initialization
  //--------------------------------------------------------------------------------------
  CloseWindow(); // Close window and OpenGL context
  //--------------------------------------------------------------------------------------

  return 0;
}
