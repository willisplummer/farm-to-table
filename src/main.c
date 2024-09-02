#include "raylib.h"
#include "raymath.h"

typedef enum EntityArchetype {
  nil = 0,
  player = 1,
  hoe = 2,
  shovel = 3,
} EntityArchetype;

typedef struct Entity {
  EntityArchetype archetype;
  Vector2 pos;
} Entity;

#define MAX_ENTITY_COUNT 1024
typedef struct World {
  Entity entities[MAX_ENTITY_COUNT];
} World;

// {0} automatically zeros the data allocated for the struct
static World world = {0};

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
  Texture2D playerTexture = LoadTexture("assets/player.png");

  Vector2 playerPosition = {(float)screenWidth / 2, (float)screenHeight / 2};

  SetTargetFPS(60); // Set our game to run at 60 frames-per-second
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

    DrawTextureEx(playerTexture, playerPosition, 0.0f, 4.0f, RAYWHITE);

    EndDrawing();
    //----------------------------------------------------------------------------------
  }

  // De-Initialization
  //--------------------------------------------------------------------------------------
  CloseWindow(); // Close window and OpenGL context
  //--------------------------------------------------------------------------------------

  return 0;
}
