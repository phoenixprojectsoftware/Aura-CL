#pragma once

class CGameMenuSound
{
public:
	static void PlayOpen();
	static void PlayClose();
	static void PlayHover();
	//static void PlayClick();
	//static void PlayBack();
	static void PlayError();

private:
	static void PlaySoundFile(const char* pszSound);
};
