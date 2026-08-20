# PocketMC 🎮
[Read in English](README.md)

Un cliente y motor de juego nativo multiplataforma avanzado que recrea y moderniza la clásica experiencia de **Minecraft Pocket Edition 0.6.1**. PocketMC traslada el encanto nostálgico del motor clásico de vóxeles a sistemas de escritorio y móviles modernos, incorporando características de personalización premium, optimizaciones en los motores de renderizado y una arquitectura de sistema mejorada.

---

## 📖 Contexto e Historial del Proyecto
PocketMC es un proyecto desarrollado en C++ nativo enfocado en portar, restaurar y optimizar el código base de la icónica versión **Minecraft PE 0.6.1**. En lugar de ejecutar el juego en entornos emulados (como BlueStacks u otros emuladores de Android), PocketMC se compila directamente para el sistema operativo destino, permitiendo su ejecución nativa en Windows, Linux y Android.

### ¿Por qué MCPE 0.6.1?
Lanzada en 2013, Minecraft Pocket Edition 0.6.1 fue una de las versiones clásicas más importantes de la historia de los dispositivos móviles. Introdujo carteles, armaduras, crías de animales, nuevos bloques decorativos, gravedad física en arena/grava y el emblemático Núcleo del Reactor del Nether. PocketMC preserva minuciosamente las mecánicas físicas y peculiaridades originales de este motor clásico de vóxeles, mientras implementa modernizaciones técnicas como vestidor de skins interactivo en 3D, interfaces gráficas completamente responsivas, recarga optimizada de chunks y un sistema renovado de sonido posicional.

---

## 📂 Arquitectura de Código Fuente y Módulos

El repositorio está organizado de forma modular, aislando la lógica interna del videojuego, las plataformas gráficas de renderizado, la red UDP y los sistemas de entrada del sistema operativo:

```
PocketMC/
├── src/
│   ├── client/           # Renderizador del cliente, interfaz GUI, sonido y modelos de entidades
│   ├── world/            # Física de bloques, items, niveles, generadores de terreno y entidades
│   ├── network/          # Wrappers JNI/nativos, paquetes de red, sincronización cliente/servidor
│   ├── platform/         # Gestión de ventanas, lectura de archivos y entrada por SO
│   ├── raknet/           # Código fuente de la biblioteca de red RakNet
│   ├── nbt/              # Lector/escritor NBT para guardar mundos (level.dat)
│   └── util/             # Utilidades matemáticas, vectores y buffers
├── project/              # Proyectos de compilación (Android JNI, scripts y proyectos de Visual Studio)
└── data/                 # Recursos multimedia del juego (texturas, sonidos, fuentes, traducciones)
```

### 1. Bucle de Juego y Reloj Interno (`NinecraftApp`, `Minecraft`, `Timer`)
- **`main.cpp` e Inicio por SO:** Inicializa el contexto OpenGL, carga el sistema de ventanas (GLFW en escritorio, Native Activity en Android) y arranca el hilo principal de la aplicación.
- **`NinecraftApp`:** Controla eventos a gran escala como redimensionamiento de ventana, repintado de pantalla y la planificación de ticks lógicos.
- **`Minecraft`:** La clase central del cliente. Controla el jugador local, los modos de juego, el mundo actual (`Level`), los estados del motor de render y la pila de interfaces de usuario activas.
- **`Timer`:** PocketMC funciona mediante un bucle de reloj dual. Los ticks lógicos (física, IA, daño, gravedad) se actualizan a un ritmo exacto y fijo de **20 Hz (20 ciclos por segundo)**, mientras que el bucle de renderizado se ejecuta a la máxima frecuencia permitida por tu monitor (60 FPS o superior).

### 2. Motor de Vóxeles y Mundo (`src/world`)
- **Generador de Terreno (`world/level/generator`):** Recrea la generación de terreno clásica de Minecraft PE mediante ruido fractal de Perlin y Simplex, ubicando biomas, árboles, vegetación y depósitos de mineral idénticos a los del año 2013.
- **Estructura de Datos de Chunks (`world/level/chunk`):** El mundo está segmentado en Chunks verticales de 16x16 bloques horizontales por 128 bloques de altura. Los bytes de cada chunk almacenan los IDs de bloques, metadatos, arreglos de luz (Sky-Light / Block-Light) y límites de altura.
- **Registro de Bloques e Items (`world/level/tile`, `world/item`):**
  - **`Tile.cpp`:** Define las propiedades físicas de cada bloque del juego (transparencia, fricción, resistencia, coordenadas de textura, opacidad luminosa y sonido de pisada).
  - **`Item.cpp`:** Controla las herramientas, armas, armaduras e ítems utilizables.
- **Colisiones y Entidades (`world/entity`, `world/phys`):**
  - El jugador y las entidades están regulados por cajas de colisión físicas alineadas en los ejes (**AABB** - `AABB.cpp`).
  - Resuelve desplazamientos, velocidades, fricciones contra el suelo y resistencia en fluidos (agua/lava).

### 3. Pipeline de Renderizado Gráfico (`src/client/renderer`)
- **`LevelRenderer` y Mallado de Chunks:**
  - Los bloques visibles se agrupan y compilan en buffers de vértices (VBOs) en la GPU para optimizar el dibujado por hardware.
  - Sincroniza dinámicamente el modo de **Gráficos Detallados (Fancy)**:
    - **Fancy ON:** Las hojas de árboles activan transparencias y las nubes se generan como mallas 3D volumétricas con grosor e iluminación lateral.
    - **Fancy OFF:** Las hojas se muestran opacas y las nubes pasan a ser un plano 2D optimizado con bajo recuento de polígonos.
- **`Tesselator`:** Buffer intermedio de vértices y texturas que compila la geometría 2D y 3D en memoria RAM para enviarla a la tarjeta gráfica en una sola llamada de dibujo (Draw Call).
- **`TileRenderer`:** Renderizador de bloques especiales que requieren rotaciones y traslaciones individuales (escaleras, carteles, cofres, cultivos).

### 4. Pila de Interfaces Gráficas (`src/client/gui`)
- **`Screen` e Interfaces:** Utiliza una estructura de pila LIFO. Puedes abrir pantallas encima de otras (`pushScreen`) y cerrarlas (`popScreen`) para volver al juego.
- **Diseño Responsivo:** Todas las posiciones y márgenes de los botones se recalculan en tiempo real dentro del método `setupPositions()` al redimensionar la ventana o rotar un dispositivo móvil, impidiendo botones duplicados o desalineados.
- **Recorte por Hardware (`glScissor`):** Los listados extensos (idiomas, skins o panoramas) aplican un test de recorte OpenGL para asegurar que los elementos desplazables no se dibujen fuera del contenedor delimitado. Soporta scroll nativo mediante la rueda del ratón en PC.

### 5. Multijugador y Conectividad (`src/network`)
- **Motor RakNet (`src/raknet`):** Utiliza RakNet para gestionar conexiones de red robustas bajo el protocolo UDP.
- **Sincronización (`ClientSideNetworkHandler` / `ServerSideNetworkHandler`):** Empaqueta y deserializa movimientos de jugadores, chat, cambios en el terreno y estados de entidades en tiempo real.

### 6. Sonido Posicional y Música (`src/client/sound`)
- **OpenAL:** Controla los efectos de sonido posicional en 3D (ruptura de bloques, pasos, monstruos) mediante buffers de audio OpenAL.
- **Decodificador C418:** Reproduce la música de fondo original decodificando los archivos `.m4a` mediante la API de hardware del sistema operativo correspondiente (Media Foundation en Windows, `MediaPlayer` en Android).

---

## 💎 Características Exclusivas de PocketMC

PocketMC implementa varias adiciones premium al motor original:

1. **Vestidor 3D (Dressing Room):**
   * Visualiza a tu avatar en 3D rotar interactivamente.
   * Carga packs de skins personalizados y oficiales (`skins.json`).
   * Sincroniza el modelo Slim/Normal seleccionado con los menús de inicio, pausa y renderizado ingame del jugador.
2. **Selector de Panoramas Interactivo:**
   * Cambia el fondo del menú principal a un cubemap 3D en rotación.
   * Galería con más de 14 panoramas clásicos e históricos de las actualizaciones del juego.
3. **Optimizaciones de Rendimiento:**
   * **Hot Reload Diferido:** Evita congelaciones al cambiar opciones gráficas; la recarga pesada de chunks solo se ejecuta una vez al cerrar el menú raíz de Opciones.
   * **Nubes Híbridas 2D/3D:** Regenera dinámicamente la malla de nubes al instante para adecuarse al ajuste gráfico seleccionado.

---

## 🛠️ Instrucciones de Compilación

### 🪟 Windows (MSVC y CMake)
1. Instala **Visual Studio 2022** con la carga de trabajo *Desarrollo de escritorio con C++*.
2. Abre la carpeta del proyecto en Visual Studio (CMake se detectará automáticamente), o compila desde una consola:
   ```pwsh
   cmake -B build
   cmake --build build --config Release
   ```
3. Ejecuta el archivo resultante ubicado en `build/Release/MinecraftPE.exe`.

### 🐧 Linux (Servidor/Cliente)
1. Instala las dependencias necesarias (`openal`, `glfw3`, `libpng`, `cmake`, y un compilador de C++).
2. Configura y compila:
   ```bash
   cmake -B build
   cmake --build build
   ```

### 🤖 Android (NDK Nativo)
- PocketMC se compila como una biblioteca compartida nativa (`libminecraftpe.so`) que es cargada mediante JNI por el wrapper de Java (`MainActivity.java`).
- Compila para las arquitecturas `arm64-v8a` y `armeabi-v7a` usando el conjunto de herramientas del NDK de Android (`build.sh` o scripts de ndk-build).
- Los recursos del juego (texturas, sonidos, fuentes, shaders) se empaquetan optimizados dentro de los assets del APK.
