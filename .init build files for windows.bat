@echo off
title Syncing Submodules

git submodule update --init --recursive

echo Look above in case anything went wrong. Otherwise,
pause
cls

title Generating Build Files

echo.
echo Please enter your Cross Product Beta app install path.
echo Example: C:\Program Files (x86)\Steam\steamapps\common\Cross Product Beta
echo.

set /p GAME_PATH=GAME_PATH:

if "%GAME_PATH%"=="" (
	echo.
	echo ERROR: GAME_PATH cannot be empty.
	pause
	exit /b 1
)

cmake -A Win32 -B build -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DGAME_PATH="%GAME_PATH%"

echo Look above in case anything went wrong. Otherwise,
pause
