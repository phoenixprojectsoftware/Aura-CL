// Aura Version Info Header
// If you're using Aura for your own mod, change the values for the zamnhlmp definers.

// Aura Version
#define AURA_VER_MAJOR 3
#define AURA_VERSION_MINOR 2
#define AURA_VERSION_PATCH 0

// These are in the order they appear in PrintVersion()
#ifdef _HALO
#define gameName "Halo: GoldSource"
#else
#define gameName "Half-Life: Cross Product Multiplayer"
#endif
#define season "Season 10: Uplink"
#define Authors "The Phoenix Project Software"
#define auraCL "https://github.com/phoenixprojectsoftware/Aura-CL"
#define auraSE "https://github.com/phoenixprojectsoftware/Aura-SE"
#define zamnhlmpRepo "https://github.com/phoenixprojectsoftware/zamnhlmp"

#if defined(_DEBUG) && !defined(_STEAMWORKS) && !defined(_HALO)
#define buildCfg "Debug"
#elif defined(_DEBUG) && defined(_STEAMWORKS) && !defined(_HALO)
#define buildCfg "SteamDebug"
#elif !defined(_DEBUG) && !defined(_STEAMWORKS) && !defined(_HALO)
#define buildCfg "Release"
#elif !defined(_DEBUG) && defined(_STEAMWORKS) && !defined(CLOSED_BETA) && !defined(_HALO)
#define buildCfg "SteamRelease"
#elif !defined(_DEBUG) && defined(_STEAMWORKS) && defined(CLOSED_BETA) && !defined(_HALO)
#define buildCfg "SteamClosedBeta"
#elif defined(_HALO) && !defined(_DEBUG)
#define buildCfg "HaloRelease"
#elif defined(_HALO) && defined(_DEBUG)
#define buildCfg "HaloDebug"
#endif

#ifdef _LINUX
#define buildSys "Linux"
#endif

#ifdef _WIN32
#if _MSC_VER >= 1910 && _MSC_VER <= 1916
#define buildSys "Microsoft Visual C++ 2017"
#elif _MSC_VER >= 1920 && _MSC_VER <= 1929
#define buildSys "Microsoft Visual C++ 2019"
#elif _MSC_VER >= 1930 && _MSC_VER <= 1944
#define buildSys "Microsoft Visual C++ 2022"
#elif _MSC_VER >= 1950
#define buildSys "Microsoft Visual C++ 2026"
#endif
#endif