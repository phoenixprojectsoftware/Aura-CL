#ifndef VGUI_BRIDGE_H
#define VGUI_BRIDGE_H

bool VGUI2ViewportAvailable();
void ShowVGUI2ScoreBoard();
void HideVGUI2ScoreBoard();
bool IsVGUI2ScoreBoardMouseActive();
bool IsVGUI2ScoreBoardVisible();
void ActivateScoreBoardMouse();

extern char gServerName[128];

#endif
