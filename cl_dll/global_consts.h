//========= Copyright � 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================
#ifndef GLOBALCONSTS_H
#define GLOBALCONSTS_H
#ifdef _WIN32
#pragma once
#endif



enum 
{ 
	MAX_PLAYERS = 64,
	MAX_TEAMS = 64,
	MAX_TEAM_NAME = 16,
};

#define MAX_SCORES					10
#define MAX_SCOREBOARD_TEAMS		5

#define NUM_ROWS		(MAX_PLAYERS + (MAX_SCOREBOARD_TEAMS * 2))

#define MAX_SERVERNAME_LENGTH	64
#define MAX_TEAMNAME_SIZE 32

#define MAX_MOTD_LENGTH 1536
#define MAX_UNICODE_MOTD_LENGTH (MAX_MOTD_LENGTH * 2) // Some unicode characters take two or more bytes in UTF8. I mean, WTF?!?!? That's so dumb bruh. But that's just how COMPUTERS work. I GUESS.

#endif // GLOBALCONSTS_H