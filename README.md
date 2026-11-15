# Castles III: Siege & Conquest

A medieval strategy game of conquest, resource management, and diplomacy set in the late Eastern Roman world.

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

## Roadmap
- [ ] Save/Load
- [ ] Installers for Linux, Windows
- [ ] Special Quests
- [ ] Sabateurs
- [ ] Construct Castles (difficult)
- [ ] Advanced combat system (difficult)
      
<img width="2544" height="1644" alt="image" src="https://github.com/user-attachments/assets/fb244b2c-a6bd-4373-b3ba-8efdb8904e17" />

<img width="2544" height="1644" alt="image" src="https://github.com/user-attachments/assets/65eaf3e5-2fd8-4451-9725-fbe907de3385" />



