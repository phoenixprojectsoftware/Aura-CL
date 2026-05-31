#include "../engineclientcmd.h"
#include "GameMenuSound.h"

void CGameMenuSound::PlaySoundFile(const char* pszSound)
{
	if (!pszSound || !pszSound[0])
		return;

	char szCommand[256];
	snprintf(szCommand, sizeof(szCommand), "play \"%s\"\n", pszSound);

	VGUI2_Cmd(szCommand);
}

void CGameMenuSound::PlayOpen()
{
	PlaySoundFile("UI/launch_selectmenu.wav");
}

void CGameMenuSound::PlayClose()
{
	PlaySoundFile("UI/launch_dnmenu1.wav");
}

void CGameMenuSound::PlayHover()
{
	PlaySoundFile("UI/buttonrollover.wav");
}

void CGameMenuSound::PlayError()
{
	PlaySoundFile("UI/error.wav");
}
