/****
*
* Copyright © 2021-2025 The Phoenix Project Software. Some Rights Reserved.
*
* AURA
*
* clientiewport - Client viewport for the VGUI2 client interface. Based upon
* BugfixedHL-Rebased. Used as per the license agreement.
* 
* 
****/

#include <KeyValues.h>
#include <vgui_controls/AnimationController.h>

#include "client_viewport.h"
//#include "../hud.h" // why do I have to go back a dir ?

// moving to hud.cpp later ??
//int g_iPlayerClass;
//int g_iTeamNumber;
//int g_iUser1 = 0;
//int g_iUser2 = 0;
//int g_iUser3 = 0;

CClientViewport * gViewPort = nullptr;

CClientViewport::CClientViewport()
	: BaseClass(nullptr, "CClientViewport")
{
	Assert(!gViewPort);
	gViewPort = this;

	SetSize(10, 10); // quiet "parent not sized yet" spew
	SetKeyBoardInputEnabled(false);
	SetMouseInputEnabled(false);

	vgui2::HScheme scheme = vgui2::scheme()->LoadSchemeFromFile("resource/ClientScheme.res", "ClientScheme");
	SetScheme(scheme);
	SetProportional(true);

	// create the anim controller
	//m_pAnimController = new vgui2::AnimationController(this);
	//m_pAnimController->SetScheme(scheme);
	//m_pAnimController->SetProportional(true);

	// skipping hud animations as we are still on TriAPI at aura 2.4. maybe later we will add vgui2 hud. https://github.com/tmp64/BugfixedHL-Rebased/commit/0bafa2c19d0376fd6d14b06713b0204080abf8b8#diff-2f3795fbbf7d163fb0d68a2e3e7d401dc7c1fe7404fb4660d4b4b03909f506c2

	/*
	void CClientViewport::MsgFunc_ValClass(const char* pszName, int iSize, void* pbuf)
	{
	}

	void CClientViewport::MsgFunc_TeamNames(const char* pszName, int iSize, void* pbuf)
	{
	}

	void CClientViewport::MsgFunc_Feign(const char* pszName, int iSize, void* pbuf)
	{
	}

	void CClientViewport::MsgFunc_Detpack(const char* pszName, int iSize, void* pbuf)
	{
	}

	void CClientViewport::MsgFunc_VGUIMenu(const char* pszName, int iSize, void* pbuf)
	{
	}

	void CClientViewport::MsgFunc_MOTD(const char* pszName, int iSize, void* pbuf)
	{
	}

	void CClientViewport::MsgFunc_BuildSt(const char* pszName, int iSize, void* pbuf)
	{
	}

	void CClientViewport::MsgFunc_RandomPC(const char* pszName, int iSize, void* pbuf)
	{
	}

	void CClientViewport::MsgFunc_ServerName(const char* pszName, int iSize, void* pbuf)
	{
	}

	void CClientViewport::MsgFunc_ScoreInfo(const char* pszName, int iSize, void* pbuf)
	{
	}

	void CClientViewport::MsgFunc_TeamScore(const char* pszName, int iSize, void* pbuf)
	{
	}

	void CClientViewport::MsgFunc_TeamInfo(const char* pszName, int iSize, void* pbuf)
	{
	}

	void CClientViewport::MsgFunc_Spectator(const char* pszName, int iSize, void* pbuf)
	{
	}

	void CClientViewport::MsgFunc_AllowSpec(const char* pszName, int iSize, void* pbuf)
	{
	}

	void CClientViewport::MsgFunc_SpecFade(const char* pszName, int iSize, void* pbuf)
	{
	}

	void CClientViewport::MsgFunc_ResetFade(const char* pszName, int iSize, void* pbuf)
	{
	}

	//-------------------------------------------------------
// TeamFortressViewport stubs
//-------------------------------------------------------
	void CClientViewport::UpdateCursorState()
	{
	}

	void CClientViewport::ShowCommandMenu(int menuIndex)
	{
	}

	void CClientViewport::HideCommandMenu()
	{
	}

	void CClientViewport::InputSignalHideCommandMenu()
	{
	}

	void CClientViewport::InputPlayerSpecial(void)
	{
	}

	bool CClientViewport::SlotInput(int iSlot)
	{
		return false;
	}

	bool CClientViewport::AllowedToPrintText(void)
	{
		return true;
	}

	void CClientViewport::DeathMsg(int killer, int victim)
	{
	}

	void CClientViewport::GetAllPlayersInfo(void)
	{
		for (int i = 1; i < MAX_PLAYERS; i++)
		{
			gEngfuncs.pfnGetPlayerInfo(i, &g_PlayerInfoList[i]);
		}
	}

	bool CClientViewport::IsScoreBoardVisible(void)
	{
		return false;
	}

	void CClientViewport::ShowScoreBoard(void)
	{
	}

	void CClientViewport::HideScoreBoard(void)
	{
	}

	void CClientViewport::UpdateSpectatorPanel()
	{
	}

	int CClientViewport::KeyInput(int down, int keynum, const char* pszCurrentBinding)
	{
		return 1;
	}
	*/
}