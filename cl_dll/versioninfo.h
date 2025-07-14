// Aura Version Info Header
// If you're using Aura for your own mod, change the values for the zamnhlmp definers.

// These are in the order they appear in PrintVersion()
#define auraVersion "v2.3.2"
#define season "Season 9: White Sands"
#define Authors "The Phoenix Project Software"
#define auraCL "https://github.com/phoenixprojectsoftware/Aura-CL"
#define auraSE "https://github.com/phoenixprojectsoftware/Aura-SE"
#define zamnhlmpRepo "https://github.com/phoenixprojectsoftware/zamnhlmp"

#if defined(_DEBUG) && !defined(_STEAMWORKS)
#define buildCfg "Debug"
#elif defined(_DEBUG) && defined(_STEAMWORKS)
#define buildCfg "SteamDebug"
#elif !defined(_DEBUG) && !defined(_STEAMWORKS)
#define buildCfg "Release"
#elif !defined(_DEBUG) && defined(_STEAMWORKS) && !defined(CLOSED_BETA)
#define buildCfg "SteamRelease"
#elif !defined(_DEBUG) && defined(_STEAMWORKS) && defined(CLOSED_BETA)
#define buildCfg "SteamClosedBeta"
#endif