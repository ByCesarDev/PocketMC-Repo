# PocketMC 🎮

[English](#-english) | [Español](#-español)

---

## 🇬🇧 English

An advanced cross-platform client and engine based on classic Minecraft PE gameplay, optimized with modern interface features, multimedia systems, and performance enhancements.

### 🌟 Highlighted Features

#### 👕 3D Dressing Room (Skindex)
- **60 FPS Interactive Preview:** View your character rotate in real time inside the redesigned Bedrock-style dressing room.
- **Model Geometry Support:** Select and sync your preferred geometry instantly: **Slim / Alex (3px arms)** or **Normal / Steve (4px arms)**.
- **Skin Packs:** Load and equip skins directly from Bedrock skin packs (`skins.json`) or add custom skins using the integrated interface and interactive modals.

#### 🖼️ 3D Panorama Selector
- **Real-Time Backgrounds:** Customize the main menu background with a rotating 3D cubemap at 60 FPS.
- **Large Registry:** Over 14 official backgrounds from the game's history (PocketMC Classic, Buzzy Bees, Nether Update, Caves & Cliffs, Wild, Tricky Trials, etc.).
- **Persistence:** Settings are saved automatically in your local options configuration.

#### 🎵 C418 Background Music System
- **Iconic Tracks:** Enjoy C418's classic soundtrack (`Sweden`, `Minecraft`, `Clark`, `Subwoofer Lullaby`, etc.).
- **Sweden Rule:** `Sweden` always plays on first startup for nostalgia, then integrates into the random rotation.
- **Smart Mute:** When setting music volume to 0%, the track is temporarily muted for up to 30 seconds (600 ticks) without restarting. It only stops completely if the volume remains at 0% after that period.

#### ☁️ Optimized Graphics (Fancy vs Fast)
- **Volumetric Clouds (Fancy ON):** 3D clouds with thickness, depth, and shading.
- **Flat Clouds (Fancy OFF / Fast):** Simplified and optimized 2D cloud layer.
- **Leaf Block Rendering:** Semi-transparent leaves in fancy mode, and solid blocks in fast mode for maximum performance.
- **Deferred Chunk Rebuilding:** The heavy chunk rebuilding process (`levelRenderer->allChanged()`) is postponed and executed only when exiting the main options screen, preventing stutters in submenus.

#### 📜 Advanced Interface & Scrolling
- **Hardware Clipping (`glScissor`):** Scrollable lists (settings, panoramas, languages) are strictly drawn within their containers, eliminating visual overflow.
- **Mouse Wheel Scroll:** Native mouse wheel scroll support for list navigation on PC.
- **Responsive Layout:** Adaptive menus for any window size or mobile screen rotation without duplicating UI elements.

### 🛠️ Build and Platforms

PocketMC can be compiled for a variety of architectures and operating systems:

#### 🪟 Windows (CMake / MSVC)
1. Install Visual Studio with C++ and CMake support.
2. Open the project folder in Visual Studio or run:
   ```pwsh
   cmake -B build
   cmake --build build --config Debug
   ```

#### 🐧 Linux Server / Client
1. Install the required dependencies (`libopenal-dev`, `libglfw3-dev`, `libpng-dev`, etc.).
2. Configure and build using CMake.

#### 🤖 Android (arm64-v8a & armeabi-v7a)
- Build the shared library (`.so`) and package the APK using the integrated Android NDK build script.
- Audio and texture assets are packaged directly into the internal assets storage of the APK.

### 🔗 Credits and Base Project
Original project based on reverse engineering of **Minecraft Pocket Edition 0.6.1**:
- Reference base repository: [Minecraft PE 0.6.1](https://gitea.sffempire.ru/Kolyah35/minecraft-pe-0.6.1)

---

## 🇪🇸 Español

Un cliente y motor avanzado para plataformas cruzadas basado en la jugabilidad clásica de Minecraft PE, optimizado con características modernas de interfaz, sistemas multimedia y rendimiento.

### 🌟 Características Destacadas

#### 👕 Vestidor 3D (Dressing Room)
- **Previsualización Interactiva a 60 FPS:** Visualiza a tu personaje rotar en tiempo real dentro del vestidor rediseñado estilo Bedrock Edition.
- **Soporte de Modelos:** Elige y sincroniza instantáneamente tu geometría preferida: **Slim / Alex (brazos de 3px)** o **Normal / Steve (brazos de 4px)**.
- **Packs de Skins:** Carga y equipa skins directamente desde packs de Bedrock (`skins.json`) o añade skins personalizadas con la interfaz integrada y modales interactivos.

#### 🖼️ Selector de Panoramas 3D
- **Fondos en Tiempo Real:** Personaliza el fondo del menú principal con un cubemap 3D giratorio a 60 FPS.
- **Registro Amplio:** Más de 14 fondos oficiales de la historia del juego (PocketMC Classic, Buzzy Bees, Nether Update, Caves & Cliffs, Wild, Tricky Trials, etc.).
- **Persistencia:** Ajustes guardados automáticamente en tu archivo de configuración local.

#### 🎵 Sistema de Música de Fondo C418
- **Pistas Emblemáticas:** Disfruta de la banda sonora clásica de C418 (`Sweden`, `Minecraft`, `Clark`, `Subwoofer Lullaby`, etc.).
- **Regla Sueca:** `Sweden` se reproduce obligatoriamente en el primer arranque para evocar nostalgia, integrándose luego al ciclo aleatorio.
- **Mute Inteligente:** Al bajar la música al 0%, la pista se silencia temporalmente por 30 segundos (600 ticks) sin reiniciarse, deteniéndose definitivamente solo si transcurre ese lapso.

#### ☁️ Gráficos Optimizados (Fancy vs Fast)
- **Nubes Volumétricas (Fancy ON):** Nubes en 3D con relieve, profundidad y sombreado.
- **Nubes Planas (Fancy OFF / Fast):** Capa 2D simplificada y optimizada de nubes.
- **Hojas de Árboles:** Hojas caladas y transparentes en modo detallado, y bloques sólidos en modo rápido para máximo rendimiento.
- **Carga de Chunks Diferida:** El pesado proceso de reconstrucción de chunks (`levelRenderer->allChanged()`) se pospone y ejecuta solo al salir de la pantalla principal de opciones, evitando tirones en los submenús.

#### 📜 Interfaz y Scroll Avanzado
- **Recorte por Hardware (`glScissor`):** Las listas desplazables (ajustes, panoramas, idiomas) se dibujan estrictamente dentro de sus contenedores, sin desbordamiento visual.
- **Scroll con Ratón:** Soporte nativo para desplazarse verticalmente usando la rueda de scroll en PC.
- **Diseño Responsivo:** Pantallas adaptables a cualquier tamaño de ventana o rotación de pantalla móvil sin duplicación de elementos de interfaz.

### 🛠️ Compilación y Plataformas

PocketMC se puede compilar en una gran variedad de arquitecturas y sistemas operativos:

#### 🪟 Windows (CMake / MSVC)
1. Instala Visual Studio con soporte C++ y CMake.
2. Abre la carpeta del proyecto en Visual Studio o ejecuta:
   ```pwsh
   cmake -B build
   cmake --build build --config Debug
   ```

#### 🐧 Linux Server / Client
1. Instala las dependencias necesarias (`libopenal-dev`, `libglfw3-dev`, `libpng-dev`, etc.).
2. Configura y compila mediante CMake.

#### 🤖 Android (arm64-v8a & armeabi-v7a)
- Compila la biblioteca compartida `.so` y empaqueta el APK mediante el script de compilación Android NDK integrado en el proyecto.
- Los assets de audio y texturas se empaquetan optimizados dentro del almacenamiento interno de assets del paquete.

### 🔗 Créditos y Base del Proyecto
Proyecto original basado en la ingeniería inversa de **Minecraft Pocket Edition 0.6.1**:
- Repositorio base de referencia: [Minecraft PE 0.6.1](https://gitea.sffempire.ru/Kolyah35/minecraft-pe-0.6.1)
