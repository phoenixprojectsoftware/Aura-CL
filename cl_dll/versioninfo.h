// Aura Version Info Header
// If you're using Aura for your own mod, change the values for the zamnhlmp definers.

// These are in the order they appear in PrintVersion()
#define auraVersion "v3.0"
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