# PocketMC 🎮
[Leer en Español](README_ES.md)

An advanced cross-platform native client and game engine reconstructing the classic **Minecraft Pocket Edition 0.6.1** experience. PocketMC brings the nostalgic charm of classic pocket voxel gaming to modern desktops and mobile operating systems with premium feature extensions, optimized rendering pipelines, and robust platform compatibility.

---

## 📖 Project Overview & Goals
PocketMC is a native C++ project focused on porting, restoring, and modernizing the iconic **Minecraft PE 0.6.1** codebase. Rather than running the game inside heavy emulator stacks (such as BlueStacks or Android emulators), PocketMC compiles natively to run directly on Windows, Linux, and Android.

### Why MCPE 0.6.1?
Released in 2013, Minecraft Pocket Edition 0.6.1 was a pivotal update that introduced signs, armor, baby animals, new blocks, sand/gravel gravity, and the iconic Nether Reactor Core. PocketMC reconstructs this vintage build, preserving its exact gameplay mechanics and physics quirks, while introducing modern features like custom 3D skins, responsive GUI layouts, high-performance chunk rendering, and an updated audio system.

---

## 📂 Source Code Architecture & Modules

The repository is organized into distinct subfolders, separating game logic, rendering pipelines, networking, and platform abstractions:

```
PocketMC/
├── src/
│   ├── client/           # Client renderer, GUI system, options, sound, and entity models
│   ├── world/            # Block physics, items, levels, terrain generators, and entities
│   ├── network/          # JNI/native network wrapper, packets, client/server sync
│   ├── platform/         # OS-dependent windowing, file I/O, and input mapping
│   ├── raknet/           # The RakNet networking library source code
│   ├── nbt/              # Named Binary Tag parser for level.dat saving/loading
│   └── util/             # Math utilities, vectors, and string helpers
├── project/              # Platform project files (Android JNI wrappers, visual studio)
└── data/                 # Game assets (textures, sounds, fonts, lang files)
```

### 1. Game Loop & Core Execution (`NinecraftApp`, `Minecraft`, `Timer`)
- **`main.cpp` & OS Entry Points:** Initializes the OpenGL context, windowing system (GLFW on desktop, native activity on Android), and starts the main application loop.
- **`NinecraftApp`:** Handles high-level application events, window resizing, rendering, and tick scheduling.
- **`Minecraft`:** The heartbeat class of the client. It owns the player instance, game modes, current world (level), renderer state, and GUI screen stack.
- **`Timer`:** PocketMC runs on a dual-rate loop. The game tick updates physics, animations, and entities at a fixed rate of **20 Hz (20 ticks per second)**, while the render loop redraws the screen at the maximum refresh rate of your monitor (60 FPS or higher).

### 2. World Voxel Engine (`src/world`)
- **Terrain Generation (`world/level/generator`):** Generates seed-based infinite worlds using multi-octave Perlin and Simplex noise. It places trees, foliage, and ore veins exactly as in the classic mobile version.
- **Voxel Data Structure (`world/level/chunk`):** Worlds are split into Chunks measuring 16x16 blocks horizontally and 128 blocks vertically. Chunk bytes store Block IDs, metadata (nibble arrays for light/data), and sky-light heights.
- **Block & Item Registries (`world/level/tile`, `world/item`):**
  - **`Tile.cpp`:** Registers every voxel block, defining its transparency, category, texture coords, light-blocking values, and destruction time.
  - **`Item.cpp`:** Handles item instances, tools, combat mechanics, and food consumption.
- **Entity & Collision System (`world/entity`, `world/phys`):**
  - All mobs, players, and drops are entities governed by an **Axis-Aligned Bounding Box (AABB)** collision system (`AABB.cpp`).
  - Implements gravity, block collision resolution, friction, and fluid mechanics (water/lava current pushes).

### 3. Rendering Pipeline (`src/client/renderer`)
- **`LevelRenderer` & Chunk Meshing:**
  - Chunks are compiled into geometric vertex buffers (VBOs) for hardware acceleration.
  - Toggles **Fancy Graphics** dynamically:
    - **Fancy ON:** Renders tree leaf blocks with transparency and generates volumetric 3D cloud blocks with thickness and shading.
    - **Fancy OFF:** Renders leaf blocks as solid textures and replaces clouds with an optimized 2D flat plane.
- **`Tesselator`:** A legacy utility buffer that batch-compiles vertex data, texture coords, and colors, flushing them to OpenGL in single draw calls.
- **`TileRenderer`:** Handles rendering of complex blocks (chests, signs, crops, stairs) that require specialized translation/rotation matrices.

### 4. User Interface Screen Stack (`src/client/gui`)
- **`Screen` & Screen Management:** PocketMC uses a stack-based screen architecture. You can `pushScreen` (e.g. pause menu, settings) and `popScreen` to return to the active game.
- **Responsive Layout System:** All screens recalculate their layouts dynamically inside `setupPositions()` during window resize events, preventing UI duplicate buttons or clipping.
- **Scrolled Selection List Clipping:** Lists (such as language selection or panorama files) utilize hardware OpenGL scissor tests (`glScissor`) to ensure scroll items never draw outside their visible panel boundaries. It also supports desktop mouse wheel scrolling.

### 5. Multiplayer & Networking (`src/network`)
- **RakNet Engine (`src/raknet`):** Integrates the RakNet networking engine for UDP client-server connections.
- **`ClientSideNetworkHandler` & `ServerSideNetworkHandler`:** Translates game updates (block placements, player movements, chat, entities) into packets, compressing and distributing them across the session.

### 6. Positional Sound & Music System (`src/client/sound`)
- **OpenAL Engine:** Manages positional 3D sound effects (block breaking, mob sounds, footsteps) using OpenAL source-buffer architectures.
- **C418 Music Decoder:** Decodes and streams background soundtracks (`Sweden`, `Clark`, `Minecraft`, etc.) through hardware-accelerated interfaces (Media Foundation on Windows, `MediaPlayer` on Android).

---

## 💎 Bespoke Feature Enhancements in PocketMC

PocketMC is not just a direct port; it has been modernized with premium feature extensions:

1. **Skindex Dressing Room:**
   - Features a Bedrock-style 3D avatar preview.
   - Dynamically loads official skin packs (`skins.json`) and parses local textures.
   - Synchronizes your Steve/Alex geometry choice natively with all GUI menus and in-game render models.
2. **Interactive 3D Backgrounds:**
   - Replaces static panorama backdrops with a fully active 3D cubemap rotating box.
   - Includes a gallery of 14+ different panoramas representing multiple updates.
3. **Graphics & Performance Optimizations:**
   - **Deferred Chunk Rebuilds:** Graphics option changes defer rebuilding the level mesh until you exit the options menu entirely, avoiding micro-stutters during customization.
   - **2D/3D Hybrid Cloud Shader:** Dynamically regenerates cloud VBO meshes to match Fast/Fancy preferences.

---

## 🛠️ Build and Compilation Instructions

### 🪟 Windows (MSVC & CMake)
1. Install **Visual Studio 2022** with the **Desktop development with C++** workload.
2. Open the project root folder in Visual Studio (CMake is detected automatically), or build from the command line:
   ```pwsh
   cmake -B build
   cmake --build build --config Release
   ```
3. Run the compiled executable located in `build/Release/MinecraftPE.exe`.

### 🐧 Linux (Client/Server)
1. Install dependencies (`openal`, `glfw3`, `libpng`, `cmake`, and a C++ compiler).
2. Configure and build:
   ```bash
   cmake -B build
   cmake --build build
   ```

### 🤖 Android (Native NDK)
- PocketMC builds as a native shared library (`libminecraftpe.so`) loaded via JNI by a custom Android wrapper (`MainActivity.java`).
- Compiles for `arm64-v8a` and `armeabi-v7a` using the native Android NDK toolchain (`build.sh` or ndk-build scripts).
- Game assets (textures, sounds, fonts, shaders) are automatically packaged into the APK's asset folder on build.
