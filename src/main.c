#include "raylib.h"

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

  InitWindow(screenWidth, screenHeight, gameTitle);
  Texture2D playerTexture = LoadTexture("assets/player.png");

  Vector2 playerPosition = {(float)screenWidth / 2, (float)screenHeight / 2};

  SetTargetFPS(60); // Set our game to run at 60 frames-per-second
  //--------------------------------------------------------------------------------------

  // Main game loop
  while (!WindowShouldClose()) // Detect window close button or ESC key
  {
    const float deltaT = GetFrameTime();
    const float playerVelocity = 80.0f;

    // Update
    //----------------------------------------------------------------------------------
    if (IsKeyDown(KEY_RIGHT))
      playerPosition.x += (playerVelocity * deltaT);
    if (IsKeyDown(KEY_LEFT))
      playerPosition.x -= (playerVelocity * deltaT);
    if (IsKeyDown(KEY_UP))
      playerPosition.y -= (playerVelocity * deltaT);
    if (IsKeyDown(KEY_DOWN))
      playerPosition.y += (playerVelocity * deltaT);
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
