# PocketMC 🎮
[Leer en Español](README_ES.md)

An advanced cross-platform native client and game engine reconstructing the classic **Minecraft Pocket Edition 0.6.1** experience. PocketMC brings the nostalgic charm of classic pocket voxel gaming to modern desktops and mobile operating systems with premium feature extensions, optimized rendering pipelines, and robust platform compatibility.

---

## 📖 What is PocketMC?
PocketMC is a native C++ project focused on porting, restoring, and modernizing the iconic **Minecraft PE 0.6.1** build. Rather than running the game in emulation or virtualization, PocketMC compiles natively to run directly on Windows, Linux, and Android. It preserves the classic mechanics—such as the Nether Reactor Core, classic terrain generation, and vintage inventory systems—while enriching them with modern quality-of-life enhancements and customization layers.

---

## 🌟 Highlighted Features

### 👕 3D Dressing Room (Skindex)
- **Interactive 60 FPS Preview:** Fully custom skin presentation module displaying your character rotating in 3D in real-time, matching modern Bedrock style.
- **Dual Geometry Support:** Instantly swap and sync your skin layout: **Slim / Alex (3-pixel arms)** or **Normal / Steve (4-pixel arms)**.
- **Custom & Bedrock Packs:** Seamlessly load official Bedrock skin packs (`skins.json`) or manage custom skins using the user-friendly interface.

### 🖼️ Interactive 3D Panoramas
- **Cubemap Rotator:** The main menu background is rendered as a fully interactive 3D rotating box at 60 FPS.
- **Historical Gallery:** Select from over 14 historical Minecraft backgrounds (including PocketMC Classic, Buzzy Bees, Nether Update, Caves & Cliffs, Wild Update, Tricky Trials, and more).
- **Instant Previews:** Test backgrounds directly in options and apply changes globally without restarting the application.

### 🎵 Custom Audio & C418 Soundtrack
- **Original Music Engine:** Restores the immersive atmosphere with C418's classic tracks (`Sweden`, `Minecraft`, `Clark`, `Dry Hands`, etc.).
- **Smart Muting:** Muting or lowering music volume to 0% retains the current track in a paused/silent state for 30 seconds (600 ticks) instead of immediately killing the playback player, allowing instant resumption if volume is restored.

### ☁️ Modern Graphic Pipeline (Fancy vs Fast)
- **Volumetric Clouds:** Supports 3D cloud blocks with thickness, shading, and height in Fancy Mode, falling back to an optimized flat 2D layout in Fast Mode.
- **Leaf Transparencies:** Dynamically toggles tree leaves between transparent (Fancy) and solid/opaque (Fast) to prioritize hardware performance.
- **Deferred Render Reloading:** Graphic and window adjustments defer heavy chunk reconstruction (`levelRenderer->allChanged()`) until leaving the root Options screen, keeping menu navigation smooth and lag-free.

### 📜 Responsive UI & Hardware Clipping
- **glScissor Clipping:** Scrollable selection lists (settings, categories, language) use strict OpenGL viewport clipping to ensure items never overflow container borders.
- **Smooth Navigation:** Full native support for mouse wheel scrolling and keyboard inputs on desktop systems.
- **Adaptive Layout:** Completely responsive design scales dynamically with any window resize or mobile screen rotation, eliminating duplicated buttons.

---

## 🛠️ Compilation & Developer Guide

PocketMC uses a modular C++ design with CMake for desktop systems and Android NDK for mobile.

### 🪟 Windows (MSVC & CMake)
1. Install **Visual Studio 2022** with the *Desktop development with C++* workload.
2. Open the project root folder in Visual Studio (CMake is detected automatically), or build from the command line:
   ```pwsh
   cmake -B build
   cmake --build build --config Release
   ```
3. Run the compiled executable located in `build/Release/MinecraftPE.exe`.

### 🐧 Linux (Client/Server)
1. Install development dependencies (`openal`, `glfw3`, `libpng`, `cmake`, and a C++ compiler).
2. Configure and build:
   ```bash
   cmake -B build
   cmake --build build
   ```

### 🤖 Android (Native NDK)
- PocketMC builds as a native shared library (`libminecraftpe.so`) loaded via JNI by a custom Android wrapper (`MainActivity.java`).
- Compiles for `arm64-v8a` and `armeabi-v7a` using the native Android NDK toolchain (`build.sh` or ndk-build scripts).
- Game assets (textures, sounds, fonts, shaders) are automatically packaged into the APK's asset folder on build.

---

## 🔗 Credits & Context
This project is an advanced derivative of reverse engineering efforts surrounding **Minecraft Pocket Edition 0.6.1**:
- Original codebase reference: [Minecraft PE 0.6.1 on Gitea](https://gitea.sffempire.ru/Kolyah35/minecraft-pe-0.6.1)
