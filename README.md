# Castles III: Siege & Conquest

A medieval strategy game of conquest, resource management, and diplomacy set in the late Eastern Roman world.

## Navigation

- [Build Instructions](#build-instructions)
- [Dependencies](#dependencies)
- [Browser and GitHub Pages](#browser-and-github-pages)
- [Features](#features)
- [Task slots and workers](#task-slots-and-workers)
- [Roadmap](#roadmap)
- [Screenshots](#screenshots)

## Build Instructions
```bash
make run
```

## Dependencies

Install all dependencies via Homebrew:

```bash
brew install sdl2 sdl2_ttf sdl2_mixer nlohmann-json catch2 dylibbundler
```

## Browser and GitHub Pages

The browser deployment follows [c_rpg's Emscripten/Pages setup](https://github.com/christsantiris/c_rpg/blob/master/.github/workflows/browser-pages.yml), adapted for C++17, JSON exceptions, and this game's assets.

```bash
make browser       # Builds dist/browser/index.html, index.js, index.wasm, index.data
make browser-run   # Builds and serves http://127.0.0.1:8000
```

Requires Git, CMake, Make, and Python 3. If Emscripten is not on your PATH,
the first build downloads SDK **4.0.23** into the ignored `.emsdk/` directory.
Browser SDL libraries and nlohmann/json **3.11.3** are downloaded during the
first build; the native Homebrew dependencies are not needed for this target.
Serve the output over HTTP rather than opening `index.html` as a file.

For GitHub Pages, set this repository's **Settings → Pages → Build and deployment → Source**
to **GitHub Actions**. Push these changes to `main`, or run **Actions → Browser Pages → Run workflow**.
The workflow builds and deploys `dist/browser`; pull requests only build the artifact.
The deployment URL is https://christsantiris.github.io/castles3/ once the workflow succeeds.

Select the page's Play button to start and enable audio. Mouse coordinates stay
aligned when the canvas is scaled or fullscreen. Save/load dialogs use browser
prompts; saves and hall-of-fame scores persist in IndexedDB under the game's
own `/castles3-saves` mount, separate from other games on the same Pages domain.
Clearing site data removes them. If browser storage is unavailable, the page
reports that saves last only for the current session. Local saved games and
personal scores are excluded from the published bundle.

## Features
- Turn-based date system with resource collection
- Province conquest with animated combat progress
- Dynasty selection with unique starting territories
- Score-based unlock system
- Background music with toggle

## Task slots and workers

Workers and fighting units are separate. Starting worker counts and difficulty
settings are retained pending play-testing. Military workers perform tasks;
infantry, archers, and knights fight battles. More assigned workers shorten task
duration, independently of the category's unlocked task-slot count.

Campaign time stops while a battle is preparing, running, paused, or displaying
results. The player must dismiss results before campaign time resumes. Other
arriving marches wait rather than replacing that battle. Battle animation has
its own frame clock. Any future automatic-battle mode must likewise retain a
result-review pause; there is currently no automatic-battle toggle.

Resource roles: marching pays food; troops periodically require gold upkeep;
recruiting uses iron and/or timber. Diplomatic tasks generally require gold,
except trade, which swaps resources. Current scouting has no gold fee. Castle
construction is planned to require all resources, but is not implemented yet.
These are the intended resource roles; costs and balance remain subject to
play-testing rather than being changed alongside task-slot fixes.

Every new game starts with **one administrative, one military, and one diplomatic
task slot**. The existing score thresholds unlock a second slot per category:

| Category | Tasks sharing its slots | Score for second slot |
| --- | --- | --- |
| Administrative (Stock) | Resource collection | 200 |
| Military | Marching and recruiting, combined | 400 |
| Diplomatic | Scouting, bribery, and trading, combined | 600 |

Worker selectors start at zero and reset after starting a task; the player must
explicitly assign at least one worker before each task starts.
Unused workers do not unlock extra slots. Assign all available workers to one
task, or split the category's worker pool between two tasks after its second
slot unlocks. Starting a task requires both a free category slot and enough
unassigned workers. Completing or canceling a task frees its slot and workers.
The thresholds live in `UnlockSystem::UnlockThresholds`; `TaskSlots` enforces
the shared limits and positive worker allocation in game logic, independently of the UI.
Task types declare static category/label metadata and register their storage once
in `World::tasks()`. Both admission checks and task bars scan that common view;
adding a task type does not require adding special cases to the category policy. Score and active tasks
are saved, so these rules also apply after loading. Older saves with excess
tasks are repaired on load: tasks are kept in task-bar order and overflow is
canceled, returning workers and applying the normal cancellation refunds.

## Roadmap

- [ ] Installers for Linux, Windows
- [ ] Special Quests e.g Holy grail or other papal requests
- [ ] Sabateurs to destroy AI castles or poison troops 
- [ ] Advanced combat system (difficult)
- [ ] Fix upkeep desertion increasing troop counts when the treasury exceeds the payment due.
- [ ] Add a balanced gold cost to scouting, consistent with diplomatic resource costs.
- [ ] Implement castle construction as an administrative task consuming food, timber, iron, and gold, including progress, cancellation, effects, and persistence.
- [ ] Implement spying as a distinct diplomatic task with costs, duration, outcomes, and persistence.
- [ ] Review AI participation in worker pools, task slots, resource costs, and upkeep after play-testing; retain current difficulty settings meanwhile.
- [ ] Consistently display task costs, assigned workers, expected duration, available slots, and reasons actions are blocked.
- [ ] Display upcoming worker/slot milestones and announce unlocks.
- [ ] Balance food for marches, iron/timber for recruitment, gold for diplomacy/upkeep, trade exchanges, and all-resource castle costs together.

## Screenshots

<img width="2544" height="1644" alt="image" src="https://github.com/user-attachments/assets/fb244b2c-a6bd-4373-b3ba-8efdb8904e17" />

<img width="2544" height="1644" alt="image" src="https://github.com/user-attachments/assets/65eaf3e5-2fd8-4451-9725-fbe907de3385" />



