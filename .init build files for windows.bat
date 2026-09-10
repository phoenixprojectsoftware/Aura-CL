REM Simple CMake Generator for Aura-CL
REM Copyright (c) 2026 The Phoenix Project Software SVG. All rights reserved

@echo off 
title Syncing Submodules...
git submodule update --init --recursive
echo Look above in case anything went wrong. Otherwise,
pause

cls
title SET GAME PATH
echo.
echo Please enter your CROSS PRODUCT BETA install path.
echo Example: C:\Program Files (x86)\Steam\steamapps\common\Cross Product Beta
echo.

set /p GAME_PATH=GAME_PATH:

if "%GAME_PATH%"=="" (
	echo.
	echo ERROR: GAME_PATH cannot be empty.
	pause
	exit /b 1
)

cls
title CMake Generator for AURA-CL

:menu
cls
color 0e
echo Welcome to the CMake generator for Aura-CL
echo (C) 2026 The Phoenix Project Software SVG
echo.
echo Please choose an option:
echo 1. SteamClosedBeta
echo 2. SteamRelease (RTR)
echo 3. Quick refresh (changed files, don't need new defines or anything)
echo 4. HaloBeta
echo 5. HaloRelease
echo q. Quit
set /p input=COMMAND?
if %input% == 1 goto closedbeta
if %input% == 2 goto release
if %input% == 3 goto refresh
if %input% == 4 goto halobeta
if %input% == 5 goto halorelease
if %input% == q exit

:closedbeta
cls
title ZAMNHLMP CLOSED BETA
cmake --fresh -DCMAKE_CXX_FLAGS_INIT="-DCLOSED_BETA" -DCMAKE_C_FLAGS_INIT="-DCLOSED_BETA" -DGAME_PATH="%GAME_PATH%" -DGAMEDIR="zamnhlmp_dev" -B build -A Win32
echo We've now generated build files with the CLOSED_BETA define. Look above in case anything went wrong. Otherwise,
pause
goto menu

:release
cls
title ZAMNHLMP RTR
cmake --fresh -DGAME_PATH="%GAME_PATH%" -DGAMEDIR="zamnhlmp_dev" -B build -A Win32
echo We've now generated build files for zamnhlmp RTR. Look above in case anything went wrong. Otherwise,
pause
goto menu

:refresh
cls
title REFRESHING...
cmake -B build -A Win32
echo Look above in case anything went wrong. Otherwise,
pause
goto menu

:halobeta
cls
title HALO BETA
cmake --fresh -DCMAKE_CXX_FLAGS_INIT="-DCLOSED_BETA -D_HALO" -DCMAKE_C_FLAGS_INIT="-DCLOSED_BETA -D_HALO" -DGAME_PATH="%GAME_PATH%" -DGAMEDIR="excession_dev" -B build -A Win32
echo We've now generated build files with the CLOSED_BETA and _HALO defines. Look above in case anything went wrong. Otherwise,
pause
goto menu

:halorelease
cls
title HALO RTR
cmake --fresh -DCMAKE_CXX_FLAGS_INIT="-D_HALO" -DCMAKE_C_FLAGS_INIT="-D_HALO" -DGAME_PATH="%GAME_PATH%" -DGAMEDIR="excession_dev" -B build -A Win32
echo We've now generated build files with the _HALO define. Look above in case anything went wrong. Otherwise,
pause
goto menu
