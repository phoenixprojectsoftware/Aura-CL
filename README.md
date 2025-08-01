# Aura-CLient
The wacky clientside binary used for our multiplayer games, based on OpenAG. Aura uses some pretty cool stuff here, mainly the Source SDK headers and VGUI2.
---

# Building
## Build configurations
Unlike the server there are quite a few build configs here that you should be worrying about, so let's get into it.
1. Debug & Release - modders should use these because it excludes all the Steam stuff. Unless you're feeling brave... Members of The Phoenix Project Software SVG should avoid these build configs.
2. SteamClosedBeta - acts the same as SteamRelease but with some different stuff going on, mainly in the HUD watermark, for Closed Beta builds.
3. SteamDebug & SteamRelease - like Debug & Release but with the `_STEAMWORKS` definition. **Members of The Phoenix Project Software SVG must use this config.**
## Windows
1. Install CMake. Latest version will do - if anything breaks in an update we'll let you know.
2. Install Git. It's pretty important that you do this or the build will fail. Actually, you should be using Git for coding in general, anyway. If you're not, sort yourself out, please.
3. In the root of this repo, run `.GENERATE WINDOWS BUILD FILES.BAT` it will initialise the Git Submodules:
	- OpenAL
	- Discord RPC (YaLTeR fork)
	- HL1 Source SDK (sabianroberts fork)
Okay, cool. Since that went well, you should notice a new build folder. In that, there's only one file you care about and that's **`Aura-Client.sln`**. Wasn't too bad, right?

Now you can build the Aura client as you would any other VS solution, and it will build the Source SDK libraries for you too, including Tier1&2.

Remember that if you create, remove, or rename a file, **you must edit the respective CMakeLists.txt file in the subdirectory. If you don't, the build will fail.**

### Manual Build
1. `cd build`
2. `cmake --build . --config <buildconfig>`

## Linux
Unlike the server, Linux here is a little easier. Open this folder in a Terminal.
1. Have CMake and Git installed. CMake must be 3.20+.
2. With `sudo apt-get install`, install the following:
	- `gcc`
	- `clang`
	- `mesa-common-dev`
	- `libgl1-mesa-dev`
3. `mkdir build`
4. `cd build`
5. `cmake ..`
6. `cmake --build . --config <buildconfig>`