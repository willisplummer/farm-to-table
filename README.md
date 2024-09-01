# Farm To Table

Template is [here](https://github.com/willisplummer/ooga-booga-raylib)

[Raylib cheatsheet](https://www.raylib.com/cheatsheet/cheatsheet.html)

## Project Structure

- lib - libraries (mainly raylib 5.0 built libs)
- src - code
- assets - textures, audio, shaders, etc
- bin - built application

## Build

run `make` in the top-level directory then you can play by calling the executable with `bin/build_mac`

## Next Steps
-[x] set background color
-[x] import my player asset
-[x] implement player movement
-[x] gameloop should account for delta_t
-[x] make player sprite the right size
-[x] make sure I have a working debugger with nvim (and generally I have LSP for C etc) -- leader>db to set break point and <leader>dr to run the debugger
-[ ] hot reloading - https://seletz.github.io/posts/hotreload-gamecode-in-c/ https://github.com/seletz/raylib-hot-code-reload-c-example?tab=readme-ov-file

