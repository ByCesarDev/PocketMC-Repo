# PocketMC 🎮

[Leer en Español](README_ES.md)

An advanced cross-platform client and engine based on classic Minecraft PE gameplay, optimized with modern interface features, multimedia systems, and performance enhancements.

---

## 🌟 Highlighted Features

### 👕 3D Dressing Room (Skindex)
- **60 FPS Interactive Preview:** View your character rotate in real time inside the redesigned Bedrock-style dressing room.
- **Model Geometry Support:** Select and sync your preferred geometry instantly: **Slim / Alex (3px arms)** or **Normal / Steve (4px arms)**.
- **Skin Packs:** Load and equip skins directly from Bedrock skin packs (`skins.json`) or add custom skins using the integrated interface and interactive modals.

### 🖼️ 3D Panorama Selector
- **Real-Time Backgrounds:** Customize the main menu background with a rotating 3D cubemap at 60 FPS.
- **Large Registry:** Over 14 official backgrounds from the game's history (PocketMC Classic, Buzzy Bees, Nether Update, Caves & Cliffs, Wild, Tricky Trials, etc.).
- **Persistence:** Settings are saved automatically in your local options configuration.

### 🎵 C418 Background Music System
- **Iconic Tracks:** Enjoy C418's classic soundtrack (`Sweden`, `Minecraft`, `Clark`, `Subwoofer Lullaby`, etc.).
- **Sweden Rule:** `Sweden` always plays on first startup for nostalgia, then integrates into the random rotation.
- **Smart Mute:** When setting music volume to 0%, the track is temporarily muted for up to 30 seconds (600 ticks) without restarting. It only stops completely if the volume remains at 0% after that period.

### ☁️ Optimized Graphics (Fancy vs Fast)
- **Volumetric Clouds (Fancy ON):** 3D clouds with thickness, depth, and shading.
- **Flat Clouds (Fancy OFF / Fast):** Simplified and optimized 2D cloud layer.
- **Leaf Block Rendering:** Semi-transparent leaves in fancy mode, and solid blocks in fast mode for maximum performance.
- **Deferred Chunk Rebuilding:** The heavy chunk rebuilding process (`levelRenderer->allChanged()`) is postponed and executed only when exiting the main options screen, preventing stutters in submenus.

### 📜 Advanced Interface & Scrolling
- **Hardware Clipping (`glScissor`):** Scrollable lists (settings, panoramas, languages) are strictly drawn within their containers, eliminating visual overflow.
- **Mouse Wheel Scroll:** Native mouse wheel scroll support for list navigation on PC.
- **Responsive Layout:** Adaptive menus for any window size or mobile screen rotation without duplicating UI elements.

---

## 🛠️ Build and Platforms

PocketMC can be compiled for a variety of architectures and operating systems:

### 🪟 Windows (CMake / MSVC)
1. Install Visual Studio with C++ and CMake support.
2. Open the project folder in Visual Studio or run:
   ```pwsh
   cmake -B build
   cmake --build build --config Debug
   ```

### 🐧 Linux Server / Client
1. Install the required dependencies (`libopenal-dev`, `libglfw3-dev`, `libpng-dev`, etc.).
2. Configure and build using CMake.

### 🤖 Android (arm64-v8a & armeabi-v7a)
- Build the shared library (`.so`) and package the APK using the integrated Android NDK build script.
- Audio and texture assets are packaged directly into the internal assets storage of the APK.

---

## 🔗 Credits and Base Project
Original project based on reverse engineering of **Minecraft Pocket Edition 0.6.1**:
- Reference base repository: [Minecraft PE 0.6.1](https://gitea.sffempire.ru/Kolyah35/minecraft-pe-0.6.1)
