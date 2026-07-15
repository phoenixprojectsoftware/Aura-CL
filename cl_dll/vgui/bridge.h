#ifndef VGUI_BRIDGE_H
#define VGUI_BRIDGE_H

// Scoreboard
bool VGUI2ViewportAvailable();
void ShowVGUI2ScoreBoard();
void HideVGUI2ScoreBoard();
bool IsVGUI2ScoreBoardMouseActive();
bool IsVGUI2ScoreBoardVisible();
void ActivateScoreBoardMouse();

// Chat
void PrintVGUI2Chat(const char* text, int clientIndex);
void StartVGUI2ChatMessageMode(int messageMode);
void StopVGUI2ChatMessageMode();
void PreventGameUIEscape(bool prevent);

extern char gServerName[128];

#endif
