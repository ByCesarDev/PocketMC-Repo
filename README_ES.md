# PocketMC 🎮
[Read in English](README.md)

Un motor y cliente nativo multiplataforma avanzado que recrea y moderniza la clásica experiencia de **Minecraft Pocket Edition 0.6.1**. PocketMC traslada el encanto nostálgico del motor clásico de vóxeles a sistemas de escritorio y móviles modernos, incorporando características de personalización premium, optimizaciones de rendimiento y un sistema multimedia mejorado.

---

## 📖 ¿Qué es PocketMC?
PocketMC es un proyecto en C++ nativo enfocado en portar, restaurar y optimizar la icónica versión **Minecraft PE 0.6.1**. En lugar de ejecutar el juego bajo emulación o virtualización, PocketMC se compila de forma nativa para ejecutarse directamente sobre Windows, Linux y Android. Preserva con fidelidad las mecánicas de juego clásicas (como el Reactor de Nether, la generación de terreno original y el sistema de inventario antiguo) mientras las enriquece con mejoras visuales modernas y capas de personalización avanzadas.

---

## 🌟 Características Destacadas

### 👕 Vestidor 3D (Dressing Room)
- **Previsualización Interactiva a 60 FPS:** Visualiza a tu personaje rotar en tiempo real dentro de un vestidor rediseñado al estilo Bedrock Edition.
- **Soporte de Modelos:** Elige y sincroniza instantáneamente tu geometría preferida: **Slim / Alex (brazos de 3 píxeles)** o **Normal / Steve (brazos de 4 píxeles)**.
- **Packs de Skins:** Carga y equipa skins directamente desde packs oficiales de Bedrock (`skins.json`) o añade skins personalizadas con la interfaz integrada y modales interactivos.

### 🖼️ Selector de Panoramas 3D
- **Fondos en Tiempo Real:** Personaliza el fondo del menú principal con un cubemap 3D giratorio a 60 FPS.
- **Registro Amplio:** Más de 14 fondos oficiales de la historia del juego (PocketMC Classic, Buzzy Bees, Nether Update, Caves & Cliffs, Wild Update, Tricky Trials, etc.).
- **Persistencia:** Ajustes guardados automáticamente en tu archivo de configuración local.

### 🎵 Banda Sonora C418 y Audio
- **Motor de Música Original:** Restaura la atmósfera inmersiva con las pistas clásicas de C418 (`Sweden`, `Minecraft`, `Clark`, `Dry Hands`, etc.).
- **Mute Inteligente:** Al bajar el volumen al 0%, la pista de música actual se silencia temporariamente por 30 segundos (600 ticks) sin reiniciarse, deteniéndose definitivamente solo si transcurre ese lapso.

### ☁️ Gráficos Avanzados (Fancy vs Fast)
- **Nubes Volumétricas (Fancy ON):** Nubes en 3D con relieve, profundidad y sombreado.
- **Nubes Planas (Fancy OFF / Fast):** Capa 2D simplificada y optimizada de nubes.
- **Hojas de Árboles:** Hojas caladas y transparentes en modo detallado, y bloques sólidos en modo rápido para máximo rendimiento.
- **Carga de Chunks Diferida:** El pesado proceso de reconstrucción de chunks (`levelRenderer->allChanged()`) se pospone y ejecuta solo al salir de la pantalla principal de opciones, evitando tirones en los submenús.

### 📜 Interfaz y Scroll Avanzado
- **Recorte por Hardware (`glScissor`):** Las listas desplazables (ajustes, panoramas, idiomas) se dibujan estrictamente dentro de sus contenedores, sin desbordamiento visual.
- **Scroll con Ratón:** Soporte nativo para desplazarse verticalmente usando la rueda de scroll en PC.
- **Diseño Responsivo:** Pantallas adaptables a cualquier tamaño de ventana o rotación de pantalla móvil sin duplicación de elementos de interfaz.

---

## 🛠️ Compilación y Desarrollo

PocketMC utiliza un diseño modular en C++ con soporte para CMake en escritorio y Android NDK para móviles.

### 🪟 Windows (CMake / MSVC)
1. Instala **Visual Studio 2022** con la carga de trabajo *Desarrollo de escritorio con C++*.
2. Abre la carpeta del proyecto en Visual Studio (CMake se detectará automáticamente), o compila desde la terminal:
   ```pwsh
   cmake -B build
   cmake --build build --config Release
   ```
3. Ejecuta el archivo resultante ubicado en `build/Release/MinecraftPE.exe`.

### 🐧 Linux (Cliente/Servidor)
1. Instala las dependencias de desarrollo (`openal`, `glfw3`, `libpng`, `cmake` y un compilador de C++).
2. Configura y compila:
   ```bash
   cmake -B build
   cmake --build build
   ```

### 🤖 Android (NDK Nativo)
- PocketMC se compila como una biblioteca compartida nativa (`libminecraftpe.so`) que es cargada mediante JNI por el wrapper de Java (`MainActivity.java`).
- Compila para las arquitecturas `arm64-v8a` y `armeabi-v7a` usando el conjunto de herramientas del NDK de Android (`build.sh` o scripts de ndk-build).
- Los recursos del juego (texturas, sonidos, fuentes, shaders) se empaquetan optimizados dentro de los assets del APK.

---

## 🔗 Créditos y Base del Proyecto
Proyecto basado en la ingeniería inversa de **Minecraft Pocket Edition 0.6.1**:
- Repositorio base de referencia: [Minecraft PE 0.6.1](https://gitea.sffempire.ru/Kolyah35/minecraft-pe-0.6.1)
