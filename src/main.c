#include <raylib.h>
#include <raymath.h>

int main(void) {
  InitWindow(400, 224, "Template");

  while (!WindowShouldClose()) {
    BeginDrawing();
      ClearBackground(RAYWHITE);

    EndDrawing();
  }

  CloseWindow();

  return 0;
}
