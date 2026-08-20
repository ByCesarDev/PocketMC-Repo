# PocketMC 🎮

[Read in English](README.md)

Un cliente y motor avanzado para plataformas cruzadas basado en la jugabilidad clásica de Minecraft PE, optimizado con características modernas de interfaz, sistemas multimedia y rendimiento.

---

## 🌟 Características Destacadas

### 👕 Vestidor 3D (Dressing Room)
- **Previsualización Interactiva a 60 FPS:** Visualiza a tu personaje rotar en tiempo real dentro del vestidor rediseñado estilo Bedrock Edition.
- **Soporte de Modelos:** Elige y sincroniza instantáneamente tu geometría preferida: **Slim / Alex (brazos de 3px)** o **Normal / Steve (brazos de 4px)**.
- **Packs de Skins:** Carga y equipa skins directamente desde packs de Bedrock (`skins.json`) o añade skins personalizadas con la interfaz integrada y modales interactivos.

### 🖼️ Selector de Panoramas 3D
- **Fondos en Tiempo Real:** Personaliza el fondo del menú principal con un cubemap 3D giratorio a 60 FPS.
- **Registro Amplio:** Más de 14 fondos oficiales de la historia del juego (PocketMC Classic, Buzzy Bees, Nether Update, Caves & Cliffs, Wild, Tricky Trials, etc.).
- **Persistencia:** Ajustes guardados automáticamente en tu archivo de configuración local.

### 🎵 Sistema de Música de Fondo C418
- **Pistas Emblemáticas:** Disfruta de la banda sonora clásica de C418 (`Sweden`, `Minecraft`, `Clark`, `Subwoofer Lullaby`, etc.).
- **Regla Sueca:** `Sweden` se reproduce obligatoriamente en el primer arranque para evocar nostalgia, integrándose luego al ciclo aleatorio.
- **Mute Inteligente:** Al bajar la música al 0%, la pista se silencia temporalmente por 30 segundos (600 ticks) sin reiniciarse, deteniéndose definitivamente solo si transcurre ese lapso.

### ☁️ Gráficos Optimizados (Fancy vs Fast)
- **Nubes Volumétricas (Fancy ON):** Nubes en 3D con relieve, profundidad y sombreado.
- **Nubes Planas (Fancy OFF / Fast):** Capa 2D simplificada y optimizada de nubes.
- **Hojas de Árboles:** Hojas caladas y transparentes en modo detallado, y bloques sólidos en modo rápido para máximo rendimiento.
- **Carga de Chunks Diferida:** El pesado proceso de reconstrucción de chunks (`levelRenderer->allChanged()`) se pospone y ejecuta solo al salir de la pantalla principal de opciones, evitando tirones en los submenús.

### 📜 Interfaz y Scroll Avanzado
- **Recorte por Hardware (`glScissor`):** Las listas desplazables (ajustes, panoramas, idiomas) se dibujan estrictamente dentro de sus contenedores, sin desbordamiento visual.
- **Scroll con Ratón:** Soporte nativo para desplazarse verticalmente usando la rueda de scroll en PC.
- **Diseño Responsivo:** Pantallas adaptables a cualquier tamaño de ventana o rotación de pantalla móvil sin duplicación de elementos de interfaz.

---

## 🛠️ Compilación y Plataformas

PocketMC se puede compilar en una gran variedad de arquitecturas y sistemas operativos:

### 🪟 Windows (CMake / MSVC)
1. Instala Visual Studio con soporte C++ y CMake.
2. Abre la carpeta del proyecto en Visual Studio o ejecuta:
   ```pwsh
   cmake -B build
   cmake --build build --config Debug
   ```

### 🐧 Linux Server / Client
1. Instala las dependencias necesarias (`libopenal-dev`, `libglfw3-dev`, `libpng-dev`, etc.).
2. Configura y compila mediante CMake.

### 🤖 Android (arm64-v8a & armeabi-v7a)
- Compila la biblioteca compartida `.so` y empaqueta el APK mediante el script de compilación Android NDK integrado en el proyecto.
- Los assets de audio y texturas se empaquetan optimizados dentro del almacenamiento interno de assets del paquete.

---

## 🔗 Créditos y Base del Proyecto
Proyecto original basado en la ingeniería inversa de **Minecraft Pocket Edition 0.6.1**:
- Repositorio base de referencia: [Minecraft PE 0.6.1](https://gitea.sffempire.ru/Kolyah35/minecraft-pe-0.6.1)
