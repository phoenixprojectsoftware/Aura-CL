#pragma once

#include <cstddef>

class CStageLevel
{
public:
	CStageLevel();

	int GetMapCount() const;

	int FindMap(const char* pszMapName) const;

	const char* GetMapName(int iIndex) const;

	const char* GetCurrentMapName() const;

	const char* GetCurrentMapHash() const;

	bool IsMap() const;

	bool IsMap(const char* pszMapName) const;

	bool IsCurrentMapHashValid() const;

	void ForceRevalidate();

private:
	static constexpr int MAX_MAP_HASHES = 4;
	static constexpr std::size_t MAX_MAP_NAME_LENGTH = 64;
	static constexpr std::size_t SHA256_STRING_LENGTH = 65;

	struct StageMap
	{
		const char* pszName;

		const char* pszHashes[MAX_MAP_HASHES];
	};

private:
	const StageMap* FindMapEntry(const char* pszMapName) const;

	bool RefreshValidationCache() const;

	bool CalculateFileSHA256(const char* pszFileName, char* pszOutput, std::size_t outputSize) const;

	bool DoesHashMatch(const StageMap& map, const char* pszHash) const;

private:
	static const StageMap s_OfficialMaps[];
	static const int s_iOfficialMapCount;

	mutable bool m_bCacheInitialised;
	mutable bool m_bCurrentMapOfficial;
	mutable bool m_bCurrentMapHashValid;

	mutable char m_szCachedMapName[MAX_MAP_NAME_LENGTH];
	mutable char m_szCachedMapHash[SHA256_STRING_LENGTH];
};

extern CStageLevel g_StageLevel;
