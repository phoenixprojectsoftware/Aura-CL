#include <tier1/strtools.h>
#include "../hud.h"
#include "gameui_test_panel.h"
#include "gameui_viewport.h"

static const char* s_Messages[] = {
	u8"Hello world!",
	u8"Did you submit your status report to the administrator, today?",
	u8"Freeman, you fool!",
	u8"Do you know who ate all the donuts?",
	u8"\"DO YOU **** *****?\" - Gunnery Sergeant Hartman, your senior drill instructor",
	u8"None of it is real, is it?",
	u8"This is a dream... Wake up",
	u8"Or is it? Hey, vsause, Michael here!",

	u8"Hello. Have you eaten any GRAINGERS today?",

	u8"I like ginger nuts and fig rolls",

	u8"Play the damn game nobhead",

	u8"Never gonna give you up\n"
	u8"Never gonna let you down\n"
	u8"Never gonna run around and desert you\n"
	u8"Never gonna make you cry\n"
	u8"Never gonna say goodbye\n"
	u8"Never gonna tell a lie and hurt you",

	u8"What is love, baby dont hurt me, dont hurt me\n        - no more",
	u8"I fear no man. But that thing (GoldSrc)... it scares me",

	u8"One shudders to imagine what inhuman thoughts lie behind that engine API...\n"
	u8"What dreams of chronic and sustained cruelty?",

	u8"- Knock-knock\n"
	u8"- Who's there?\n"
	u8"- Joe\n"
	u8"- Joe wh-\n"
	u8"- Joe Death\n",

	u8"RYZEN 5 3600X\n"
	u8"@\n"
	u8"RADEON RX580 8GB\n"
	u8"@\n"
	u8"TO PLAY A 1998 GAME"
};

static const char* s_Credits[] = {
	u8"!!!! THE PHOENIX PROJECT SOFTWARE !!!!\n"
	u8"!!!! AURA TECH TEAM !!!!\n"
	u8"RafaXIS - co-creator of Cross Product, level design, audio, music\n"
	u8"Sabian Roberts - co-creator of Cross Product, director, level design, programming, audio, music\n"
	u8"\n"
	u8"BlueNightHawk - programming\n"
	u8"DeanAMX - programming\n"
	u8"GlitchGod - level design\n"
	u8"Ry13y04 - UX lead, gamemode design\n"
	u8"naii_ - level design\n"
	u8"\n"
	u8"MTB - animator, viewmodels\n"
	u8"Hitoshii - animator\n"
	u8"\n"
	u8"Boomerang_Monkey - tester\n"
	u8"xSousa - tester\n"
	u8"Unowninator - tester\n"
	u8"Berony - tester\n"
	u8"Galexion - tester\n"
	u8"Frostlander - tester\n"
	u8"\n"
	u8"!!!! WITH SPECIAL THANKS TO !!!!\n"
	u8"Martin Webrant & Adrenaline Gamer Team\n"
	u8"YaLTeR\n"
	u8"execut4ble\n"
	u8"fireblizzard\n"
	u8"tmp64\n"
	u8"Admer456\n"
	u8"Sam Vanheer\n"
	u8"Joel Troch\n"
	u8"DocRock\n"
	u8"Mr.Slavik\n"
	u8"...and all the players!\n"
	u8"\n"
	u8"!!!! CLOSED BETA TESTERS !!!!\n"
	u8"!!!! STEAM LAUNCH - JUNE-AUGUST 2025 !!!!\n"
	u8"Thank you to the following individuals for your generous assistance with getting the best possible launch on Steam, with the launch of Season 9 Patch II (v2.9.2).\n"
	u8"Pete Borland (Lord Bork)\n"
	u8"Dante Franklin (Captain Gamer)\n"
	u8"Daniel Galiszewski (Danielko99922)\n"
	u8"Michal Grzejszcyk (zaku1)\n"
	u8"Williamd Johnson (WJKattMAN)\n"
	u8"Egor Klushkin (WebPuck9)\n"
	u8"Frederik Kronberg (Blubber the fat)\n"
	u8"Vincent Prestigiacomo (Monkey)\n"
	u8"\n"
	u8"21.vasi\n"
	u8"9mmar\n"
	u8"arsmraz\n"
	u8"Azelf89\n"
	u8"Berony\n"
	u8"CAE17\n"
	u8"Citizen118\n"
	u8"dsouza\n"
	u8"dumkl\n"
	u8"Frostlander\n"
	u8"Galexion\n"
	u8"HitoshiiAFK\n"
	u8"Houndeye - Eilay 28\n"
	u8"JS999\n"
	u8"Kenzon hundred power\n"
	u8"knoxed\n"
	u8"Levi\n"
	u8"Meowstic08\n"
	u8"Napoleon\n"
	u8"NerfMagnet\n"
	u8"Skillez Elias\n"
	u8"sooqui\n"
	u8"Speaker\n"
	u8"TheoTTG\n"
	u8"Texas Offal\n"
	u8"xdf\n"
	u8"xstephx\n"
	u8"Yoda\n"
};

CGameUITestPanel::CGameUITestPanel(vgui2::Panel* pParent) : BaseClass(pParent, "GameUITestPanel")
{
	SetTitle("Credits", false);
	SetSizeable(true);
	SetSize(460, 230);
	SetDeleteSelfOnClose(true);
	MoveToCenterOfScreen();

	m_pText = new vgui2::RichText(this, "Text");
	m_pText->SetUnusedScrollbarInvisible(true);

	SetScheme(CGameUIViewport::Get()->GetScheme());
	InvalidateLayout();
}

void CGameUITestPanel::PerformLayout()
{
	constexpr int OFFSET = 4;
	BaseClass::PerformLayout();

	int x, y, wide, tall;
	GetClientArea(x, y, wide, tall);
	m_pText->SetBounds(x + OFFSET, y + OFFSET, wide - OFFSET * 2, tall - OFFSET * 2);
}

void CGameUITestPanel::Activate()
{
	BaseClass::Activate();

	// select random msg
	wchar_t wbuf[8192];
	int idx = gEngfuncs.pfnRandomLong(0, std::size(s_Credits) - 1);
	Q_UTF8ToWString(s_Credits[idx], wbuf, sizeof(wbuf));
	m_pText->SetText(wbuf);
}