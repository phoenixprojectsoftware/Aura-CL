set(CMAKE_POLICY_VERSION_MINIMUM 3.23)
cmake_minimum_required(VERSION 3.23)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<OR:$<CONFIG:Debug>,$<CONFIG:SteamDebug>>:Debug>DLL")

# Custom build types support
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;SteamDebug;SteamRelease;SteamClosedBeta" CACHE STRING "" FORCE)
set(CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}" CACHE STRING "Choose the type of build." FORCE)

# Set custom flags for SteamDebug and SteamRelease
set(CMAKE_C_FLAGS_STEAMDEBUG "${CMAKE_C_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_STEAMDEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
set(CMAKE_EXE_LINKER_FLAGS_STEAMDEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG}")
set(CMAKE_SHARED_LINKER_FLAGS_STEAMDEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG}")

set(CMAKE_C_FLAGS_STEAMRELEASE "${CMAKE_C_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS_STEAMRELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
set(CMAKE_EXE_LINKER_FLAGS_STEAMRELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE}")
set(CMAKE_SHARED_LINKER_FLAGS_STEAMRELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE}")

set(CMAKE_C_FLAGS_STEAMCLOSEDBETA "${CMAKE_C_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS_STEAMCLOSEDBETA "${CMAKE_CXX_FLAGS_RELEASE}")
set(CMAKE_EXE_LINKER_FLAGS_STEAMCLOSEDBETA "${CMAKE_EXE_LINKER_FLAGS_RELEASE}")
set(CMAKE_SHARED_LINKER_FLAGS_STEAMCLOSEDBETA "${CMAKE_SHARED_LINKER_FLAGS_RELEASE}")

# Project metadata
project(Aura-Client)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include("CMake/helpers.cmake")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Detect if we're building a Steam-enabled configuration
set(IS_STEAM_BUILD OFF)
if(CMAKE_BUILD_TYPE MATCHES "SteamDebug|SteamRelease|SteamClosedBeta")
    set(IS_STEAM_BUILD ON)
endif()

# Steamworks include path
if(IS_STEAM_BUILD)
    target_include_directories(client PRIVATE ${CMAKE_SOURCE_DIR}/external/steamworks)
    target_compile_definitions(client PRIVATE _STEAMWORKS)
endif()

# Add _STEAMWORKS define for IntelliSense and build config
foreach(STEAM_CONFIG SteamDebug SteamRelease SteamClosedBeta)
    string(TOUPPER "${STEAM_CONFIG}" UPPER_CONFIG)
    set(CMAKE_C_FLAGS_${UPPER_CONFIG} "${CMAKE_C_FLAGS_${UPPER_CONFIG}} /D_STEAMWORKS")
    set(CMAKE_CXX_FLAGS_${UPPER_CONFIG} "${CMAKE_CXX_FLAGS_${UPPER_CONFIG}} /D_STEAMWORKS")
    if("${STEAM_CONFIG}" STREQUAL "SteamClosedBeta")
        set(CMAKE_C_FLAGS_${UPPER_CONFIG} "${CMAKE_C_FLAGS_${UPPER_CONFIG}} /DCLOSED_BETA")
		set(CMAKE_CXX_FLAGS_${UPPER_CONFIG} "${CMAKE_CXX_FLAGS_${UPPER_CONFIG}} /DCLOSED_BETA")
    endif()
endforeach()

# Add _DEBUG define only for SteamDebug
set(CMAKE_C_FLAGS_STEAMDEBUG "${CMAKE_C_FLAGS_STEAMDEBUG} /D_DEBUG")
set(CMAKE_CXX_FLAGS_STEAMDEBUG "${CMAKE_CXX_FLAGS_STEAMDEBUG} /D_DEBUG")

# Set up some compiler-related things
if(MSVC)
    msvc_set_static_runtime()
    msvc_fix_xp_build()
    msvc_enable_lto()
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")

    option(ENABLE_LTO "Enable Link-Time Optimizations" OFF)
    if(ENABLE_LTO)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -flto")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -flto")
    endif()

    option(STATIC_LIBSTDC++ "Link libstdc++ Statically" OFF)
    if(STATIC_LIBSTDC++)
        set(CMAKE_SHARED_LINKER_FLAGS
            "${CMAKE_SHARED_LINKER_FLAGS} -static-libgcc -static-libstdc++")
    endif()
endif()

# Set up current_version.h (unchanged)
find_package(Git)
if(GIT_FOUND)
    message(STATUS "Found git at ${GIT_EXECUTABLE}")
    execute_process(COMMAND "${GIT_EXECUTABLE}" show -s --format=%ct
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            OUTPUT_VARIABLE CURRENT_VERSION
            OUTPUT_STRIP_TRAILING_WHITESPACE
            RESULT_VARIABLE CURRENT_VERSION_RESULT)

    if(CURRENT_VERSION_RESULT EQUAL "0")
        set(CURRENT_VERSION "${CURRENT_VERSION}ull")
        message(STATUS "Set current version to ${CURRENT_VERSION}")
    else()
        set(CURRENT_VERSION UINT64_MAX)
        message(WARNING "Could not get current version from git; update checking will be disabled")
    endif()
else()
    set(CURRENT_VERSION UINT64_MAX)
    message(WARNING "Could not find git; update checking will be disabled")
endif()

configure_file(cl_dll/current_version.h.in current_version.h)

# Add discord-rpc (unchanged)
set(BUILD_EXAMPLES OFF CACHE BOOL "build discord-rpc examples" FORCE)
set(CLANG_FORMAT_SUFFIX "-dummy")
add_subdirectory(external/discord-rpc)

# Apply some warning disables
if(MSVC)
    target_compile_options(discord-rpc PRIVATE /wd4191 /wd4571 /wd4623 /wd5039 /wd5045)
else()
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(discord-rpc PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-Wno-class-memaccess>)
    endif()
endif()

# Enable SSE globally for all targets (needed for mathlib.h intrinsics)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse -msse2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse -msse2")

# Source layout
add_subdirectory(cl_dll)
add_subdirectory(common)
add_subdirectory(dlls)
add_subdirectory(game_shared)
add_subdirectory(pm_shared)
add_subdirectory(public)
add_subdirectory(source_sdk)

get_property(SOURCE_FILES GLOBAL PROPERTY SOURCE_FILES)
set(SOURCE_FILES ${SOURCE_FILES} "${CMAKE_BINARY_DIR}/current_version.h")

set(CMAKE_SHARED_LIBRARY_PREFIX  "")
add_library(client SHARED ${SOURCE_FILES})

target_compile_definitions(client PRIVATE ${SOURCE_SDK_DEFINES})


target_include_directories(client PRIVATE
    ${CMAKE_BINARY_DIR}
    common
    dlls
    engine
    external
    external/discord-rpc/include
    game_shared
    pm_shared
    public
    utils/vgui/include
	${SOURCE_SDK_INCLUDE_PATHS})

if(APPLE)
    target_include_directories(client SYSTEM PRIVATE /usr/include/malloc)
endif()

# Compiler and definition settings (mostly unchanged)
if(MSVC)
    target_compile_options(client PRIVATE /wd4018 /wd4838 /wd4996)
else()
    target_compile_options(client PRIVATE
        -Wall
        -Wno-parentheses
        -Wno-sign-compare
        -Wno-unused-function
        -Wno-unused-variable
        -march=pentium-m
		-msse
		-msse2
        -fno-strict-aliasing
        -fno-exceptions)

    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(client PRIVATE
            -Wno-overloaded-virtual
            -Wno-unused-private-field
            -Wno-new-returns-null)
    else()
        target_compile_options(client PRIVATE
            -Wno-unused-but-set-variable
            -Wno-stringop-truncation
            -Wno-format-truncation
            $<$<COMPILE_LANGUAGE:CXX>:-Wno-class-memaccess>)
    endif()

    target_compile_definitions(client PRIVATE
        -DGNUC -DPOSIX -D_POSIX
        -Dstricmp=strcasecmp
        -Dstrnicmp=strncasecmp
        -D_strnicmp=strncasecmp
        -D_snprintf=snprintf
        -D_snwprintf=swprintf
        -D_vsnprintf=vsnprintf
        -D_alloca=alloca)

    if(UNIX)
        target_compile_definitions(client PRIVATE -DLINUX -D_LINUX)
    endif()
endif()

# Platform-specific linkage (unchanged)
if(WIN32)
    target_compile_definitions(client PRIVATE -D_CRT_SECURE_NO_WARNINGS -DWINDOWS -D_WINDOWS)
    target_link_libraries(client
        opengl32
        ${CMAKE_SOURCE_DIR}/lib/public/SDL2.lib
        ${CMAKE_SOURCE_DIR}/lib/public/game_controls.lib
        ${CMAKE_SOURCE_DIR}/lib/public/libcurl_a.lib
        ${CMAKE_SOURCE_DIR}/lib/steam_api.lib
        ${CMAKE_SOURCE_DIR}/utils/vgui/lib/win32_vc6/vgui.lib
		${SOURCE_SDK_LIBS})
elseif(UNIX)
    target_compile_definitions(client PRIVATE -DDISABLE_JUMP_ORIGIN -DDISABLE_VEC_ORIGIN)

    if(APPLE)
        find_library(MAC_CARBON Carbon)
        find_library(MAC_OPENGL OpenGL)
        target_link_libraries(client
            ${MAC_CARBON}
            ${MAC_OPENGL}
            ${CMAKE_SOURCE_DIR}/linux/libSDL2-2.0.0.dylib
            ${CMAKE_SOURCE_DIR}/linux/release/vgui.dylib)
        target_compile_definitions(client PRIVATE GL_SILENCE_DEPRECATION)
    else()
        set(VGUI_LINK_FLAGS "-Wl,--no-as-needed -L${CMAKE_SOURCE_DIR}/linux/release -l:vgui.so")
        set(SDL2_LINK_FLAGS "-L${CMAKE_SOURCE_DIR}/linux -lSDL2")
        set_property(TARGET client PROPERTY LINK_FLAGS "${VGUI_LINK_FLAGS} ${SDL2_LINK_FLAGS}")
		target_link_libraries(client
		${SOURCE_SDK_LIBS})
    endif()
endif()

# Steamworks support
if(IS_STEAM_BUILD)
    target_compile_definitions(client PRIVATE _STEAMWORKS)
    if(WIN32)
        target_link_libraries(client ${CMAKE_SOURCE_DIR}/lib/steam_api.lib)
    elseif(UNIX)
        target_link_libraries(client ${CMAKE_SOURCE_DIR}/lib/libsteam_api.so)
    endif()
endif()

target_compile_definitions(client PRIVATE
    -DCLIENT_DLL
    -DCLIENT_WEAPONS
    -DUPDATE_CHECK)

target_link_libraries(client discord-rpc)
