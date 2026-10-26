#include "hud.h"
#include "cl_util.h"

#include "stage_level.h"
#include "sha256.h"

#include <cstdio>
#include <cstring>

#ifdef _WIN32
#define STAGE_STRICMP _stricmp
#define STAGE_STRNICMP _strnicmp
#else
#include <strings.h>
#define STAGE_STRICMP strcasecmp
#define STAGE_STRNICMP strncasecmp
#endif

// -------------------------------------------------------------------------
// Official map list
// -------------------------------------------------------------------------

CStageLevel g_StageLevel;

const CStageLevel::StageMap CStageLevel::s_OfficialMaps[] =
{
	{
		"2Fort",
		{
			"0b713420361c7867581a0a4b4d04498ca08f591439e5c11c2aeb80bea5040168",
			nullptr,
			nullptr,
			nullptr
		}
	},
	{
		"Ambush",
		{
			"c691f41309beff6211350ad646dd63ce3192d6cdbf4632a52df70711d2a35ff3",
			nullptr,
			nullptr,
			nullptr
		}
	},
	{
		"Basement",
		{
			"5d44f3ea7c3b5648c0480191275ca44504bad3f20e418559e14eb4ccb6e82106",
			nullptr,
			nullptr,
			nullptr
		}
	},
	{
		"Blimpboom",
		{
			"c080324c93b70185755d61a4f70b17f6fda68094f6f8f67661c8a3a75f6ecb5d",
			nullptr,
			nullptr,
			nullptr
		}
	},
	{
		"Bounce",
		{
			"93179975dc82e28ac53f92e921f9a26dd2add9b443560c9149b708cfe0e7b13d",
			nullptr,
			nullptr,
			nullptr
		}
	},
	{
		"City",
		{
			"614a14685936fc9a10d8cfde62826be8e1b4351aa09f0e5d53f5ae53dd41a3f5",
			nullptr,
			nullptr,
			nullptr
		}
	},
	{
		"Cloaca",
		{
			"4f491c2487d250c234e18f4bb1de655f55eac47fd917f5c5055a7cebc4bcca01",
			nullptr,
			nullptr,
			nullptr
		}
	},
	{
		"Compound",
		{
			"eb011c027431faf9552bb6489d1f760d3a0b4ffe1928b5b4cab5763cb8bedd22",
			nullptr,
			nullptr,
			nullptr
		}
	},
	{
		"Corssfire",
		{
			"8f09823b83b656b88e8b1189118ab7f9906df56d10aac8c21878850c60d9612a",
			nullptr,
			nullptr,
			nullptr
		}
	},
	{
		"Cove",
		{
			"93e40a4d69c3b87c58d11b4cec0aea0972220e82b8373fb853e7f85fc3a7b5e8",
			nullptr,
			nullptr,
			nullptr
		}
	},
	{
		"Crossfire",
		{
			"af360af90581710859b20c70b9ed559385bd0c7aae666ab1a276ac28f38a659b",
			nullptr,
			nullptr,
			nullptr
		}
	},
	{
		"Datacore",
		{
			"e35e9eb8a444e7b03d2c358d7f89f78d3017c5f5cd06cc2ef94971bf94abd652",
			nullptr,
			nullptr,
			nullptr
		}
	},
	{
		"Disposal",
		{
			"905e7d5abfcd94df9d46dbd381d474819b0bf98cb2f60793a2ff8178f505a408",
			nullptr,
			nullptr,
			nullptr
		}
	},
	{
		"Doublefort",
		{
			"68ea2074188cd8903bad6eb30d382edc2658e26bc260bac54a34894912563781",
			nullptr,
			nullptr,
			nullptr
		}
	},
	{
		"Dyce",
		{
			"3f02c62a9c9447caab018d1335db7451f28140e7911f56f37e03d5574d57964b",
			nullptr,
			nullptr,
			nullptr
		}
	},
	{
		"Frenzy",
		{
			"752abc65a1bef9755efe9b0177f932cd9d617a59bffd43aed330076ca5c7f5cc",
			nullptr,
			nullptr,
			nullptr
		}
	},
	{
		"Gauntlet",
		{
			"3aa8938c283fe425d7ace913336b522a16ea195b09fa586d842320a15df79150",
			nullptr,
			nullptr,
			nullptr
		}
	},
	{
		"Hex",
		{
			"803fd22b3778fc50d0131ee5365470d7fb4ae4287525281368475492e3645acf",
			nullptr,
			nullptr,
			nullptr
		}
	},
	{
		"Kasino",
		{
			"b148a958965eac739f85f547583c1dbdc48e8f8ee55929fac59c08b8d23da747",
			nullptr,
			nullptr,
			nullptr
		}
	},
	{
		"Lambda_Bunker",
		{
			"732465cbeb6d4d90ebb515e2d1e7f6753dd33789ada82d36e6a2d377ec660c33",
			nullptr,
			nullptr,
			nullptr
		}
	},
	{
		"Mall",
		{
			"11fb1d82b588e17a6ce77e7d3bc8dae0d70a23e8c03412bff3bfda969e8227f1",
			nullptr,
			nullptr,
			nullptr
		}
	},
	{
		"Nivalis",
		{
			"6a9ea3e2a682b1c83cf94317db2f660bedce05e4de303e67ebc3068a20cdf933",
			nullptr,
			nullptr,
			nullptr
		}
	},
	{
		"Openfire",
		{
			"debb6e22440023334ca2e0b317e25dbf896b3ed4f8ff80be5699fb45abb5cfa1",
			nullptr,
			nullptr,
			nullptr
		}
	},
	{
		"Radiated",
		{
			"a9528a9075aa4d42c0e18512bb36799fec56a8125f592fec55417fd64493fd84",
			nullptr,
			nullptr,
			nullptr
		}
	},
	{
		"Rocket_Frenzy",
		{
			"d31fa79dacf6673fb91e0fb8e195ab4ca8eda38c1eac9ea39e452826c085584f",
			nullptr,
			nullptr,
			nullptr
		}
	},
	{
		"Rust",
		{
			"73dfd03e6da129438ccf20a594893c26e04adca497bb15e1df08d18bcf5060c2",
			nullptr,
			nullptr,
			nullptr
		}
	},
	{
		"Shipment",
		{
			"414eeaef284bebc2f74661900b9bcfbeaea69b675232e5e2cbcf3d9fa448f55d",
			nullptr,
			nullptr,
			nullptr
		}
	},
	{
		"Snark_Pit",
		{
			"64736971b15f1c361d62e637b427c3e099bad53aef3af8598f7f95f5cbea002f",
			nullptr,
			nullptr,
			nullptr
		}
	},
	{
		"Subtransit",
		{
			"37e0cfb830dffc1c5d3ee71bb2f52d77bf97d1d5fb6c422350a354b0f6c93ff0",
			nullptr,
			nullptr,
			nullptr
		}
	},
	{
		"Walk",
		{
			"8ff5080c7a305ae8a96141fe67ead5dbe9aea2b51a01707f183bb8c260d5607b",
			nullptr,
			nullptr,
			nullptr
		}
	},
	{
		"Works",
		{
			"e93f1e8bdb7926fbaeb1f9bb4ce561ac4ab3220638bee6b6f1ab458fe1006954",
			nullptr,
			nullptr,
			nullptr
		}
	}
};

const int CStageLevel::s_iOfficialMapCount =
static_cast<int>(
	sizeof(CStageLevel::s_OfficialMaps) /
	sizeof(CStageLevel::s_OfficialMaps[0])
	);

// -------------------------------------------------------------------------
// Construction
// -------------------------------------------------------------------------

CStageLevel::CStageLevel()
	: m_bCacheInitialised(false),
	m_bCurrentMapOfficial(false),
	m_bCurrentMapHashValid(false)
{
	m_szCachedMapName[0] = '\0';
	m_szCachedMapHash[0] = '\0';
}

// -------------------------------------------------------------------------
// Official map-list functions
// -------------------------------------------------------------------------

int CStageLevel::GetMapCount() const
{
	return s_iOfficialMapCount;
}

int CStageLevel::FindMap(const char* pszMapName) const
{
	if (!pszMapName || !pszMapName[0])
		return 0;

	for (int i = 0; i < GetMapCount(); ++i)
	{
		if (STAGE_STRICMP(s_OfficialMaps[i].pszName, pszMapName) == 0)
			return i + 1;
	}

	return 0;
}

const char* CStageLevel::GetMapName(int iIndex) const
{
	if (iIndex < 1 || iIndex > GetMapCount())
		return nullptr;

	return s_OfficialMaps[iIndex - 1].pszName;
}

const CStageLevel::StageMap* CStageLevel::FindMapEntry(
	const char* pszMapName
) const
{
	const int iIndex = FindMap(pszMapName);

	if (iIndex == 0)
		return nullptr;

	return &s_OfficialMaps[iIndex - 1];
}

// -------------------------------------------------------------------------
// Current map
// -------------------------------------------------------------------------

const char* CStageLevel::GetCurrentMapName() const
{
	static char szMapName[MAX_MAP_NAME_LENGTH];

	szMapName[0] = '\0';

	const char* pszLevelName = gEngfuncs.pfnGetLevelName();

	if (!pszLevelName || !pszLevelName[0])
		return szMapName;

	const char* pszMapName = pszLevelName;

	// GoldSrc commonly returns:
	// maps/Crossfire.bsp
	if (
		STAGE_STRNICMP(pszMapName, "maps/", 5) == 0 ||
		STAGE_STRNICMP(pszMapName, "maps\\", 5) == 0
		)
	{
		pszMapName += 5;
	}

	std::snprintf(
		szMapName,
		sizeof(szMapName),
		"%s",
		pszMapName
	);

	// Remove the .bsp extension.
	char* pszExtension = std::strrchr(szMapName, '.');

	if (
		pszExtension &&
		STAGE_STRICMP(pszExtension, ".bsp") == 0
		)
	{
		*pszExtension = '\0';
	}

	return szMapName;
}

const char* CStageLevel::GetCurrentMapHash() const
{
	RefreshValidationCache();

	return m_szCachedMapHash;
}

// -------------------------------------------------------------------------
// Public validation
// -------------------------------------------------------------------------

bool CStageLevel::IsMap() const
{
	if (!RefreshValidationCache())
		return false;

	return m_bCurrentMapOfficial &&
		m_bCurrentMapHashValid;
}

bool CStageLevel::IsMap(const char* pszMapName) const
{
	if (!pszMapName || !pszMapName[0])
		return false;

	// The requested map must exist in the official list.
	if (!FindMapEntry(pszMapName))
		return false;

	if (!RefreshValidationCache())
		return false;

	// It must also be the map currently loaded.
	if (STAGE_STRICMP(m_szCachedMapName, pszMapName) != 0)
		return false;

	return m_bCurrentMapOfficial &&
		m_bCurrentMapHashValid;
}

bool CStageLevel::IsCurrentMapHashValid() const
{
	if (!RefreshValidationCache())
		return false;

	return m_bCurrentMapHashValid;
}

void CStageLevel::ForceRevalidate()
{
	m_bCacheInitialised = false;
	m_bCurrentMapOfficial = false;
	m_bCurrentMapHashValid = false;

	m_szCachedMapName[0] = '\0';
	m_szCachedMapHash[0] = '\0';

	RefreshValidationCache();
}

// -------------------------------------------------------------------------
// Cache management
// -------------------------------------------------------------------------

bool CStageLevel::RefreshValidationCache() const
{
	const char* pszCurrentMap = GetCurrentMapName();

	if (!pszCurrentMap || !pszCurrentMap[0])
	{
		m_bCacheInitialised = false;
		m_bCurrentMapOfficial = false;
		m_bCurrentMapHashValid = false;

		m_szCachedMapName[0] = '\0';
		m_szCachedMapHash[0] = '\0';

		return false;
	}

	// Do not hash the BSP repeatedly during the same map.
	if (
		m_bCacheInitialised &&
		STAGE_STRICMP(m_szCachedMapName, pszCurrentMap) == 0
		)
	{
		return true;
	}

	m_bCacheInitialised = true;
	m_bCurrentMapOfficial = false;
	m_bCurrentMapHashValid = false;

	m_szCachedMapName[0] = '\0';
	m_szCachedMapHash[0] = '\0';

	std::snprintf(
		m_szCachedMapName,
		sizeof(m_szCachedMapName),
		"%s",
		pszCurrentMap
	);

	const StageMap* pMap = FindMapEntry(m_szCachedMapName);

	if (!pMap)
	{
		gEngfuncs.Con_Printf(
			"CStageLevel: '%s' is not an official map.\n",
			m_szCachedMapName
		);

		return true;
	}

	m_bCurrentMapOfficial = true;

	char szBspPath[128];

	std::snprintf(
		szBspPath,
		sizeof(szBspPath),
		"maps/%s.bsp",
		m_szCachedMapName
	);

	if (
		!CalculateFileSHA256(
			szBspPath,
			m_szCachedMapHash,
			sizeof(m_szCachedMapHash)
		)
		)
	{
		gEngfuncs.Con_Printf(
			"CStageLevel: Failed to hash '%s'.\n",
			szBspPath
		);

		return true;
	}

	m_bCurrentMapHashValid =
		DoesHashMatch(*pMap, m_szCachedMapHash);

	if (!m_bCurrentMapHashValid)
	{
		gEngfuncs.Con_Printf(
			"CStageLevel: BSP hash mismatch for '%s'.\n"
			"CStageLevel: Actual SHA-256: %s\n",
			m_szCachedMapName,
			m_szCachedMapHash
		);
	}
	else
	{
		gEngfuncs.Con_Printf(
			"CStageLevel: Verified official BSP '%s'.\n",
			m_szCachedMapName
		);
	}

	return true;
}

// -------------------------------------------------------------------------
// SHA-256
// -------------------------------------------------------------------------

bool CStageLevel::CalculateFileSHA256(
	const char* pszFileName,
	char* pszOutput,
	std::size_t outputSize
) const
{
	if (!pszFileName || !pszFileName[0])
		return false;

	if (!pszOutput || outputSize < SHA256_STRING_LENGTH)
		return false;

	pszOutput[0] = '\0';

	int iFileLength = 0;

	// Client-side GoldSrc filesystem loader.
	unsigned char* pFileData =
		gEngfuncs.COM_LoadFile(
			const_cast<char*>(pszFileName),
			5,
			&iFileLength
		);

	if (!pFileData || iFileLength <= 0)
	{
		if (pFileData)
			gEngfuncs.COM_FreeFile(pFileData);

		return false;
	}

	SHA256_CTX context;
	SHA256_BYTE digest[SHA256_BLOCK_SIZE];

	sha256_init(&context);

	sha256_update(
		&context,
		pFileData,
		static_cast<std::size_t>(iFileLength)
	);

	sha256_final(
		&context,
		digest
	);

	gEngfuncs.COM_FreeFile(pFileData);

	for (int i = 0; i < SHA256_BLOCK_SIZE; ++i)
	{
		std::snprintf(
			pszOutput + (i * 2),
			outputSize - (i * 2),
			"%02x",
			static_cast<unsigned int>(digest[i])
		);
	}

	pszOutput[SHA256_BLOCK_SIZE * 2] = '\0';

	return true;
}

bool CStageLevel::DoesHashMatch(
	const StageMap& map,
	const char* pszHash
) const
{
	if (!pszHash || !pszHash[0])
		return false;

	for (int i = 0; i < MAX_MAP_HASHES; ++i)
	{
		const char* pszExpectedHash = map.pszHashes[i];

		if (!pszExpectedHash)
			break;

		if (STAGE_STRICMP(pszExpectedHash, pszHash) == 0)
			return true;
	}

	return false;
}
