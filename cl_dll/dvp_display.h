// not to be used for team membership

inline const char* DVPLabel(const char* internalName, bool isDVP)
{
	if (!internalName)
		return "";

	if (!isDVP)
		return internalName;

	if (std::strcmp(internalName, "blue") == 0)
		return "DEVELOPERS";

	if (std::strcmp(internalName, "red") == 0)
		return "PLAYERS";

	return internalName;
}
