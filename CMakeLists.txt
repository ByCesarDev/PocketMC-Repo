cmake_minimum_required(VERSION 3.21)
project(MinecraftPE)

include(cmake/CPM.cmake)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POLICY_VERSION_MINIMUM 3.10)

include(cmake/EnumOption.cmake)

# Absolute project root
set(PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")

if(EMSCRIPTEN)
    # When configuring web builds with "emcmake cmake -B build -S .", set PLATFORM to Web by default
    set(PLATFORM Web CACHE STRING "Platform to build for.")
endif()

enum_option(PLATFORM "Desktop;Web" "Platform to build for.")

find_package(Threads REQUIRED)
find_package(OpenSSL)

if (${PLATFORM} STREQUAL "Desktop")
    set(PLATFORM_CPP "PLATFORM_DESKTOP")

    if (MINGW)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++ -static-libgcc")
    endif()

    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-c++11-narrowing -Wno-narrowing -Wno-invalid-source-encoding -Wno-reserved-user-defined-literal")
    endif()

    if (WIN32)
        if (MSVC)
            add_compile_options(/FS)
        endif()
        add_definitions(-D_CRT_SECURE_NO_WARNINGS -DNOMINMAX)
        include_directories(misc/windows)
        set(EXTRA_LIBS ws2_32 winhttp)
        list(APPEND EXTRA_LIBS comdlg32)
    elseif(UNIX)
        find_library(pthread NAMES pthread)
        set(EXTRA_LIBS pthread m)
    endif()

elseif (${PLATFORM} STREQUAL "Web")
    set(PLATFORM_CPP "PLATFORM_WEB")
    set(EXTRA_LIBS "idbfs.js")
endif()

# I totally shocked
if(${PLATFORM} MATCHES "Web")
    set(PNG_LIB png)
    set(AL_LIBTYPE "STATIC")

    add_library(zlib INTERFACE IMPORTED)
    set_target_properties(zlib PROPERTIES
        INTERFACE_LINK_OPTIONS "-sUSE_ZLIB=1"
    )

    add_library(png INTERFACE IMPORTED)
    set_target_properties(png PROPERTIES
        INTERFACE_COMPILE_OPTIONS "-sUSE_LIBPNG=1" 
        INTERFACE_LINK_OPTIONS "-sUSE_LIBPNG=1"
    )

    add_library(glfw INTERFACE IMPORTED)
    set_target_properties(glfw PROPERTIES
        INTERFACE_LINK_OPTIONS "-sUSE_GLFW=3"
    )
else()
    set(PNG_LIB png_shared)
    set(AL_LIBTYPE "SHARED")

    CPMAddPackage(
        NAME "zlib"
        GIT_REPOSITORY "https://github.com/madler/zlib"
        GIT_TAG "v1.3.2"
    )

    CPMAddPackage(
        NAME "libpng"
        GIT_REPOSITORY "https://github.com/pnggroup/libpng.git"
        GIT_TAG "v1.6.55"
        OPTIONS
            "ZLIB_ROOT ${zlib_SOURCE_DIR}"
            "ZLIB_INCLUDE_DIRS ${zlib_SOURCE_DIR}"
            "PNG_TOOLS OFF"
            "PNG_TESTS OFF"
    )

    CPMAddPackage(
        NAME "glfw"
        GIT_REPOSITORY "https://github.com/glfw/glfw.git"
        GIT_TAG "3.4"
        EXCLUDE_FROM_ALL TRUE
        OPTIONS
            "GLFW_BUILD_EXAMPLES OFF"
            "GLFW_BUILD_TESTS OFF"
            "GLFW_BUILD_DOCS OFF"
    )
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(ALSOFT_STATIC_CXX_RUNTIME OFF)
else()
    set(ALSOFT_STATIC_CXX_RUNTIME ON)
endif()

CPMAddPackage(
    NAME "openal"
    GIT_REPOSITORY "https://github.com/kcat/openal-soft.git"
    GIT_TAG "1.23.1"
    OPTIONS
        "ALSOFT_EXAMPLES OFF"
        "ALSOFT_TESTS OFF"
        "ALSOFT_UTILS OFF"
        "LIBTYPE ${AL_LIBTYPE}"
        "ALSOFT_ENABLE_MODULES OFF"
        "ALSOFT_STATIC_STDCXX ${ALSOFT_STATIC_CXX_RUNTIME}"
        "ALSOFT_STATIC_LIBGCC ${ALSOFT_STATIC_CXX_RUNTIME}"
)

# TODO: Clear this paths with *
file(GLOB SERVER_SOURCES
    "${PROJECT_ROOT}/project/lib_projects/raknet/jni/RaknetSources/*.cpp"
    "${PROJECT_ROOT}/src/NinecraftApp.cpp"
    "${PROJECT_ROOT}/src/Performance.cpp"
    "${PROJECT_ROOT}/src/SharedConstants.cpp"

    "${PROJECT_ROOT}/src/client/IConfigListener.cpp"
    "${PROJECT_ROOT}/src/client/Minecraft.cpp"
    "${PROJECT_ROOT}/src/client/OptionStrings.cpp"
    "${PROJECT_ROOT}/src/client/Option.cpp"
    "${PROJECT_ROOT}/src/client/Options.cpp"
    "${PROJECT_ROOT}/src/client/OptionsFile.cpp"
    "${PROJECT_ROOT}/src/util/ServerProfiler.cpp"

    "${PROJECT_ROOT}/src/client/gamemode/CreativeMode.cpp"
    "${PROJECT_ROOT}/src/client/gamemode/GameMode.cpp"
    "${PROJECT_ROOT}/src/client/gamemode/SurvivalMode.cpp"

    "${PROJECT_ROOT}/src/client/player/LocalPlayer.cpp"
    "${PROJECT_ROOT}/src/client/player/AccountManager.cpp"
    "${PROJECT_ROOT}/src/client/player/ApiClient.cpp"
    "${PROJECT_ROOT}/src/client/player/ProfileManager.cpp"
    "${PROJECT_ROOT}/src/client/player/RemotePlayer.cpp"
    "${PROJECT_ROOT}/src/client/player/input/KeyboardInput.cpp"

    "${PROJECT_ROOT}/src/locale/I18n.cpp"

    "${PROJECT_ROOT}/src/main.cpp"
    "${PROJECT_ROOT}/src/main_dedicated.cpp"

    "${PROJECT_ROOT}/src/mods/*.cpp"

    "${PROJECT_ROOT}/src/nbt/Tag.cpp"

    "${PROJECT_ROOT}/src/network/ClientSideNetworkHandler.cpp"
    "${PROJECT_ROOT}/src/network/NetEventCallback.cpp"
    "${PROJECT_ROOT}/src/network/Packet.cpp"
    "${PROJECT_ROOT}/src/network/RakNetInstance.cpp"
    "${PROJECT_ROOT}/src/network/ServerSideNetworkHandler.cpp"
    "${PROJECT_ROOT}/src/network/command/CommandServer.cpp"

    "${PROJECT_ROOT}/src/platform/CThread.cpp"
    "${PROJECT_ROOT}/src/platform/HttpClient.cpp"
    "${PROJECT_ROOT}/src/platform/PngLoader.cpp"
    "${PROJECT_ROOT}/src/platform/time.cpp"

    "${PROJECT_ROOT}/src/platform/input/Controller.cpp"
    "${PROJECT_ROOT}/src/platform/input/Keyboard.cpp"
    "${PROJECT_ROOT}/src/platform/input/Mouse.cpp"
    "${PROJECT_ROOT}/src/platform/input/Multitouch.cpp"

    "${PROJECT_ROOT}/src/server/ArgumentsSettings.cpp"
    "${PROJECT_ROOT}/src/server/ServerLevel.cpp"
    "${PROJECT_ROOT}/src/server/ServerPlayer.cpp"

    "${PROJECT_ROOT}/src/util/DataIO.cpp"
    "${PROJECT_ROOT}/src/util/Mth.cpp"
    "${PROJECT_ROOT}/src/util/PerfTimer.cpp"
    "${PROJECT_ROOT}/src/util/StringUtils.cpp"

    "${PROJECT_ROOT}/src/world/Direction.cpp"

    "${PROJECT_ROOT}/src/world/entity/*.cpp"

    "${PROJECT_ROOT}/src/world/entity/ai/control/MoveControl.cpp"

    "${PROJECT_ROOT}/src/world/entity/animal/*.cpp"

    "${PROJECT_ROOT}/src/world/entity/item/*.cpp"

    "${PROJECT_ROOT}/src/world/entity/monster/*.cpp"

    "${PROJECT_ROOT}/src/world/entity/player/Inventory.cpp"
    "${PROJECT_ROOT}/src/world/entity/player/Player.cpp"

    "${PROJECT_ROOT}/src/world/entity/projectile/Arrow.cpp"
    "${PROJECT_ROOT}/src/world/entity/projectile/Throwable.cpp"

    "${PROJECT_ROOT}/src/world/food/SimpleFoodData.cpp"

    "${PROJECT_ROOT}/src/world/inventory/*.cpp"
    "${PROJECT_ROOT}/src/world/item/*.cpp"
    "${PROJECT_ROOT}/src/world/item/crafting/*.cpp"
    "${PROJECT_ROOT}/src/world/level/*.cpp"

    "${PROJECT_ROOT}/src/world/level/biome/Biome.cpp"
    "${PROJECT_ROOT}/src/world/level/biome/BiomeSource.cpp"

    "${PROJECT_ROOT}/src/world/level/chunk/LevelChunk.cpp"

    "${PROJECT_ROOT}/src/world/level/dimension/Dimension.cpp"

    "${PROJECT_ROOT}/src/world/level/levelgen/*.cpp"
    "${PROJECT_ROOT}/src/world/level/levelgen/feature/Feature.cpp"
    "${PROJECT_ROOT}/src/world/level/levelgen/synth/*.cpp"
    "${PROJECT_ROOT}/src/world/level/levelgen/vanilla/*.cpp"
    "${PROJECT_ROOT}/src/world/level/levelgen/vanilla/noise/*.cpp"
    "${PROJECT_ROOT}/src/world/level/levelgen/vanilla/biome/*.cpp"
    "${PROJECT_ROOT}/src/world/level/levelgen/vanilla/biomegrid/*.cpp"
    "${PROJECT_ROOT}/src/world/level/levelgen/vanilla/ground/*.cpp"
    "${PROJECT_ROOT}/src/world/level/levelgen/vanilla/object/tree/*.cpp"
    "${PROJECT_ROOT}/src/world/level/levelgen/vanilla/populator/*.cpp"

    "${PROJECT_ROOT}/src/world/level/material/Material.cpp"

    "${PROJECT_ROOT}/src/world/level/pathfinder/Path.cpp"

    "${PROJECT_ROOT}/src/world/level/storage/*.cpp"
    "${PROJECT_ROOT}/src/world/level/tile/*.cpp"
    "${PROJECT_ROOT}/src/world/level/tile/entity/*.cpp"

    "${PROJECT_ROOT}/src/world/phys/HitResult.cpp"
)
list(REMOVE_ITEM SERVER_SOURCES "${PROJECT_ROOT}/src/world/level/tile/OreFeature.cpp")

file(GLOB CLIENT_SOURCES 
    "${PROJECT_ROOT}/src/client/*.cpp"

    "${PROJECT_ROOT}/src/client/gamemode/*.cpp"
    "${PROJECT_ROOT}/src/client/gui/*.cpp"
    "${PROJECT_ROOT}/src/client/gui/components/*.cpp"
    "${PROJECT_ROOT}/src/client/gui/screens/*.cpp"
    "${PROJECT_ROOT}/src/client/gui/panoramas/*.cpp"
    "${PROJECT_ROOT}/src/client/gui/screens/crafting/*.cpp"
    "${PROJECT_ROOT}/src/client/gui/screens/touch/*.cpp"
    # Added SkindexScreen

    "${PROJECT_ROOT}/src/client/model/*.cpp"
    "${PROJECT_ROOT}/src/client/model/geom/*.cpp"

    "${PROJECT_ROOT}/src/client/particle/*.cpp"

    "${PROJECT_ROOT}/src/client/player/*.cpp"
    "${PROJECT_ROOT}/src/client/player/input/*.cpp"
    "${PROJECT_ROOT}/src/client/player/input/touchscreen/*.cpp"

    "${PROJECT_ROOT}/src/client/renderer/*.cpp"
    "${PROJECT_ROOT}/src/client/renderer/culling/*.cpp"
    "${PROJECT_ROOT}/src/client/renderer/entity/*.cpp"
    "${PROJECT_ROOT}/src/client/renderer/ptexture/*.cpp"
    "${PROJECT_ROOT}/src/client/renderer/tileentity/*.cpp"

    "${PROJECT_ROOT}/src/client/sound/*.cpp"
    
    "${PROJECT_ROOT}/src/locale/*.cpp"
    
    "${PROJECT_ROOT}/src/mods/*.cpp"

    "${PROJECT_ROOT}/src/nbt/*.cpp"

    "${PROJECT_ROOT}/src/network/*.cpp"
    "${PROJECT_ROOT}/src/network/command/*.cpp"

    "${PROJECT_ROOT}/src/platform/*.cpp"
    "${PROJECT_ROOT}/src/platform/audio/SoundSystemAL.cpp"
    "${PROJECT_ROOT}/src/platform/input/*.cpp"

    "${PROJECT_ROOT}/src/raknet/**.cpp"
    
    "${PROJECT_ROOT}/src/server/**.cpp"
    
    "${PROJECT_ROOT}/src/util/**.cpp"

    "${PROJECT_ROOT}/src/world/*.cpp"
    "${PROJECT_ROOT}/src/world/phys/*.cpp"
    "${PROJECT_ROOT}/src/world/entity/*.cpp"
    "${PROJECT_ROOT}/src/world/entity/ai/control/*.cpp"
    "${PROJECT_ROOT}/src/world/entity/animal/*.cpp"
    "${PROJECT_ROOT}/src/world/entity/item/*.cpp"
    "${PROJECT_ROOT}/src/world/entity/monster/*.cpp"
    "${PROJECT_ROOT}/src/world/entity/player/*.cpp"
    "${PROJECT_ROOT}/src/world/entity/projectile/*.cpp"

    "${PROJECT_ROOT}/src/world/food/*.cpp"
    "${PROJECT_ROOT}/src/world/inventory/*.cpp"

    "${PROJECT_ROOT}/src/world/item/*.cpp"
    "${PROJECT_ROOT}/src/world/item/crafting/*.cpp"

    "${PROJECT_ROOT}/src/world/level/*.cpp"
    "${PROJECT_ROOT}/src/world/level/biome/*.cpp"
    "${PROJECT_ROOT}/src/world/level/chunk/*.cpp"
    "${PROJECT_ROOT}/src/world/level/dimension/*.cpp"
    "${PROJECT_ROOT}/src/world/level/levelgen/*.cpp"
    "${PROJECT_ROOT}/src/world/level/levelgen/feature/*.cpp"
    "${PROJECT_ROOT}/src/world/level/levelgen/synth/*.cpp"
    "${PROJECT_ROOT}/src/world/level/levelgen/vanilla/*.cpp"
    "${PROJECT_ROOT}/src/world/level/levelgen/vanilla/noise/*.cpp"
    "${PROJECT_ROOT}/src/world/level/levelgen/vanilla/biome/*.cpp"
    "${PROJECT_ROOT}/src/world/level/levelgen/vanilla/biomegrid/*.cpp"
    "${PROJECT_ROOT}/src/world/level/levelgen/vanilla/ground/*.cpp"
    "${PROJECT_ROOT}/src/world/level/levelgen/vanilla/object/tree/*.cpp"
    "${PROJECT_ROOT}/src/world/level/levelgen/vanilla/populator/*.cpp"
    "${PROJECT_ROOT}/src/world/level/material/*.cpp"
    "${PROJECT_ROOT}/src/world/level/pathfinder/*.cpp"
    "${PROJECT_ROOT}/src/world/level/storage/*.cpp"
    "${PROJECT_ROOT}/src/world/level/tile/*.cpp"
    "${PROJECT_ROOT}/src/world/level/tile/entity/*.cpp"

    "${PROJECT_ROOT}/src/SharedConstants.cpp"
    "${PROJECT_ROOT}/src/main.cpp"
    "${PROJECT_ROOT}/src/NinecraftApp.cpp"

    "${PROJECT_ROOT}/src/AppPlatform_glfw.cpp"
    "${PROJECT_ROOT}/src/main.cpp"
)
list(REMOVE_ITEM CLIENT_SOURCES "${PROJECT_ROOT}/src/world/level/tile/OreFeature.cpp")

# Ensure extra source files we add manually get compiled (helps when CMake wasn't re-run after adding files)
list(APPEND SERVER_SOURCES "src/world/item/crafting/ExtraRecipes.cpp")
list(APPEND CLIENT_SOURCES "src/world/item/crafting/ExtraRecipes.cpp")
list(APPEND SERVER_SOURCES "src/world/item/crafting/StonecutterRecipe.cpp")
list(APPEND CLIENT_SOURCES "src/world/item/crafting/StonecutterRecipe.cpp")
list(APPEND SERVER_SOURCES "src/world/entity/monster/PigZombieBrute.cpp")
list(APPEND CLIENT_SOURCES "src/world/entity/monster/PigZombieBrute.cpp")

# Buckets
list(APPEND SERVER_SOURCES "src/world/item/BucketItem.cpp")
list(APPEND CLIENT_SOURCES "src/world/item/BucketItem.cpp")

# Screens
list(APPEND CLIENT_SOURCES "src/client/gui/screens/UnifiedInventoryScreen.cpp")
list(APPEND CLIENT_SOURCES "src/client/gui/screens/ArmorScreen.cpp")

if (${PLATFORM} STREQUAL "Desktop")
    list(APPEND CLIENT_SOURCES glad/src/glad.c)
endif()

# Server
if(UNIX AND NOT ANDROID)
    add_executable("${PROJECT_NAME}-server" ${SERVER_SOURCES})

    target_compile_definitions("${PROJECT_NAME}-server" PUBLIC "STANDALONE_SERVER" "SERVER_PROFILER")

    target_include_directories("${PROJECT_NAME}-server" PUBLIC 
        "${CMAKE_SOURCE_DIR}/src/"
        "project/lib_projects/raknet/jni/RaknetSources"
    )

    target_link_libraries("${PROJECT_NAME}-server" ${CMAKE_THREAD_LIBS_INIT})
endif()

if(WIN32)
    # Generate app_configured.rc with the absolute path to app.ico baked in
    configure_file(
        "${PROJECT_ROOT}/app.rc"
        "${CMAKE_CURRENT_BINARY_DIR}/app_configured.rc"
        @ONLY
    )
    list(APPEND CLIENT_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/app_configured.rc")
endif()

add_executable(${PROJECT_NAME} ${CLIENT_SOURCES})

target_compile_definitions(${PROJECT_NAME} PUBLIC ${PLATFORM_CPP})

target_include_directories(${PROJECT_NAME} PUBLIC 
    "${CMAKE_SOURCE_DIR}/glad/include/"
    "${CMAKE_SOURCE_DIR}/src"
    "${openal_SOURCE_DIR}/include"
    "${glfw_SOURCE_DIR}/include"
    "lib/include"
)

if(${PLATFORM} MATCHES "Web")
    set(CMAKE_CXX_STANDARD 11)
    # uuuh i hate it
    set(EM_FLAGS "-pthread -sUSE_PTHREADS=1 -sUSE_LIBPNG=1 -sSHARED_MEMORY=1")
    
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EM_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EM_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${EM_FLAGS} --preload-file ${CMAKE_SOURCE_DIR}/data@/data")

    target_compile_options(${PROJECT_NAME} PUBLIC
        "-Os"
        "-Wno-invalid-source-encoding"
        "-Wno-narrowing"
        "-Wno-deprecated-register"
        "-Wno-reserved-user-defined-literal"
    )

    target_link_options(${PROJECT_NAME} PUBLIC
        "-Os"
        "-sALLOW_MEMORY_GROWTH=1"
        "-sFORCE_FILESYSTEM=1"
        "-sLEGACY_GL_EMULATION=1"
        "-sGL_UNSAFE_OPTS=0"
        "-sEMULATE_FUNCTION_POINTER_CASTS=1"
        "-sALLOW_TABLE_GROWTH=1"
        "-sEXPORTED_RUNTIME_METHODS=['FS','stringToUTF8','UTF8ToString','cwrap','ccall','HEAP8','HEAPU8','HEAP32','HEAPU32']"
        "-sEXPORTED_FUNCTIONS=['_main']"
    )

    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        message("DEBUG MODE")

        target_link_options(${PROJECT_NAME} PUBLIC
            "-sASSERTIONS=2"
            "-sSTACK_OVERFLOW_CHECK=2"
            "-sSTACK_SIZE=5242880"
            "-sGL_DEBUG=1"
        )
    endif()

    target_compile_definitions(${PROJECT_NAME} PUBLIC "__EMSCRIPTEN__" "NO_SOUND" "NO_NETWORK")
endif()

# Client
target_compile_definitions(${PROJECT_NAME} PUBLIC "OPENGL_ES" "NO_EGL" ${PLATFORM})
target_link_libraries(${PROJECT_NAME} zlib ${PNG_LIB} OpenAL::OpenAL glfw ${EXTRA_LIBS})

if (OpenSSL_FOUND)
    target_link_libraries(${PROJECT_NAME} OpenSSL::SSL OpenSSL::Crypto)
    target_compile_definitions(${PROJECT_NAME} PUBLIC HTTPCLIENT_USE_OPENSSL)
endif()

if (NOT UNIX)
    add_custom_command(
        TARGET ${PROJECT_NAME} 
        POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}> $<TARGET_FILE_DIR:${PROJECT_NAME}> 
            COMMAND_EXPAND_LISTS
    )
endif()

if(NOT ${PLATFORM} MATCHES "Web")
    add_custom_command(
        TARGET ${PROJECT_NAME} 
        POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/data" $<TARGET_FILE_DIR:${PROJECT_NAME}>/data
    )
else()
    add_custom_command(
        TARGET ${PROJECT_NAME} 
        POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/misc/web/index.html" $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endif()

message(STATUS "Compiling with the flags:")
message(STATUS "  PLATFORM=" ${PLATFORM_CPP})
 
