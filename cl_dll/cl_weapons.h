/****
*
* Copyright (c) 2021-2024 The Phoenix Project Software. Some Rights Reserved.
*
* AURA
*
* cl_weapons.h - a shortcut to define the IDs of weapons clientside.
*
*
****/

#define WEAPON_NONE				0
#define WEAPON_CROWBAR			1
#define	WEAPON_GLOCK			2
#define WEAPON_PYTHON			3
#define WEAPON_MP5				4
// #define WEAPON_CHAINGUN			5 // our last weapon slot?!
#define WEAPON_CROSSBOW			6
#define WEAPON_SHOTGUN			7
#define WEAPON_RPG				8
#define WEAPON_GAUSS			9
#define WEAPON_EGON				10
#define WEAPON_HORNETGUN		11
#define WEAPON_HANDGRENADE		12
#define WEAPON_TRIPMINE			13
#define	WEAPON_SATCHEL			14
#define	WEAPON_SNARK			15
#define WEAPON_GRAPPLE   16
#define WEAPON_EAGLE			17
#define WEAPON_PIPEWRENCH 18
#define WEAPON_M249				19
#define WEAPON_DISPLACER		20
#define WEAPON_SHOCKRIFLE 22
#define WEAPON_SPORELAUNCHER 23
#define WEAPON_SNIPERRIFLE 24
#define WEAPON_KNIFE 25
#define WEAPON_PENGUIN   26
#define WEAPON_ONE 27

#ifdef _HALO
#define WEAPON_SMG 28
#define WEAPON_SWORD 29
#endif

#define WEAPON_BATTLERIFLE 30
#define WEAPON_HLDMAR 28
#define WEAPON_HEALER 29
#define WEAPON_THUMPER 21

#define WEAPON_ALLWEAPONS		(~(1<<WEAPON_SUIT))

#define WEAPON_SUIT				31	// ?????

#define MAX_WEAPONS			32


#define MAX_NORMAL_BATTERY	100

#define WEAPON_NOCLIP -1