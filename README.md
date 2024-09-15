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

## LSP
run `bear -- make` to get latest compiler config in `compile_commands.json` for the language server after changes to the `Makefile`

## Next Steps

-[x] set background color
-[x] import my player asset
-[x] implement player movement
-[x] gameloop should account for delta_t
-[x] make player sprite the right size
-[x] make sure I have a working debugger with nvim (and generally I have LSP for C etc) -- leader>db to set break point and <leader>dr to run the debugger
-[ ] hot reloading - https://seletz.github.io/posts/hotreload-gamecode-in-c/ https://github.com/seletz/raylib-hot-code-reload-c-example?tab=readme-ov-file

## Mechanics To Build
### Core Loop
- Time/Day
  - every minute of gameplay is 1 hour of game world time
  - time displays in corner of screen
- Health/Energy Bar
  - starts at 100%
  - depletes some 5% every hour of in-world time (this would give you 20 hours of not helping yourself to go from 100 to 0 but should pull this out into an easily adjusted variable)
- Inventory/Starting Items
  - player starts with axe, net, fishing hook, sleeping bag
  - player can toggle between items in inventory (shows them in hand? at minimum highlights the selected item in inventory)
- Foraging Items From World
  - interact with berry bush, berries pop out, bush is empty
  - player can walk over berries and add them to inventory -- I think having a foraging bag or something so that edibles are not just in base inv seems good
  - player can eat the berries from inventory (replenishes health by some amount specified on item entity)
- Sleep
  - Player starts with sleeping bag, can use it to make camp (takes some amount of time)
  - At camp, player can sleep (resting at basic camp restores 30% of current energy so 1.3x energy level - this should be easily adjustable too)
  - Overnight, things can happen in the world (food can rot, animals can steal, tracks can get made around camp, etc -- maybe there's a little rundown of what happened (and if there are random other things like "extra restful night energy bonus" those show here too)
### Enhancements
  - Make Fire
  - Water Processing
    - Player needs drinkable water (needs to locate a stream, get water in their pot, boil it)
  - Hunting
    - player has a bow and X arrows, while wandering the landscape they can enounter animals (higher chance based on factors eventually but to start just random) and then attempt to shoot them with the bow - really basic mechanic to start, but later would be cool to have this go into some kind of mini-game big buck hunter type view
  - Trapping
    - player can use some of their wire/string to create and set traps
    - they have to check them daily and there is a chance of animals getting caught
    - if not checked soon enough animal will escape or get eaten by something else
    - Traps can break with some probability
    - Fishing with the net works on the same principles as traps
  - Crafting
    - player can mine rock, wood, clay and use them to create items
      - a better base that increases amount of energy restored
      - smoker
      - food cache
  - More elaborate systems around temperature, thirst, health, mental fortitude
