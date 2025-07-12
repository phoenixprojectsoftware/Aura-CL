/***
*
*	Copyright (c) 1999, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
//
// cl_util.h
//

#include "net_api.h"
#include "color_tags.h"

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#include <stdio.h> // for safe_sprintf()
#include <stdarg.h>  // "
#include <string.h> // for strncpy()
#include <tier0/dbg.h>
#include "cvardef.h"
#include <convar.h>
#include "console.h"

// Macros to hook function calls into the HUD object
#define HOOK_MESSAGE(x) gEngfuncs.pfnHookUserMsg(#x, __MsgFunc_##x );

#define DECLARE_MESSAGE(y, x) int __MsgFunc_##x(const char *pszName, int iSize, void *pbuf) \
							{ \
							return gHUD.y.MsgFunc_##x(pszName, iSize, pbuf ); \
							}


#define HOOK_COMMAND(x, y) gEngfuncs.pfnAddCommand( x, __CmdFunc_##y );
#define DECLARE_COMMAND(y, x) void __CmdFunc_##x( void ) \
							{ \
								gHUD.y.UserCmd_##x( ); \
							}

inline float CVAR_GET_FLOAT( const char *x ) {	return gEngfuncs.pfnGetCvarFloat( x ); }
inline char* CVAR_GET_STRING( const char *x ) {	return gEngfuncs.pfnGetCvarString( x ); }
inline struct cvar_s *CVAR_CREATE( const char *cv, const char *val, const int flags ) {	return gEngfuncs.pfnRegisterVariable( cv, val, flags ); }

#define SPR_Load (*gEngfuncs.pfnSPR_Load)
#define SPR_Set (*gEngfuncs.pfnSPR_Set)
#define SPR_Frames (*gEngfuncs.pfnSPR_Frames)
#define SPR_GetList (*gEngfuncs.pfnSPR_GetList)

// SPR_Draw  draws a the current sprite as solid
#define SPR_Draw(a,x,y,b) (*gEngfuncs.pfnSPR_Draw)(a,x + gHUD.m_flHudLagOfs[0],y + gHUD.m_flHudLagOfs[1],b)
// SPR_DrawHoles  draws the current sprites,  with color index255 not drawn (transparent)
#define SPR_DrawHoles(a,x,y,b) (*gEngfuncs.pfnSPR_DrawHoles)(a,x + gHUD.m_flHudLagOfs[0],y + gHUD.m_flHudLagOfs[1],b)
// SPR_DrawAdditive  adds the sprites RGB values to the background  (additive transulency)
#define SPR_DrawAdditive(a,x,y,b) (*gEngfuncs.pfnSPR_DrawAdditive)(a,x + gHUD.m_flHudLagOfs[0],y + gHUD.m_flHudLagOfs[1],b)

// SPR_EnableScissor  sets a clipping rect for HUD sprites.  (0,0) is the top-left hand corner of the screen.
#define SPR_EnableScissor (*gEngfuncs.pfnSPR_EnableScissor)
// SPR_DisableScissor  disables the clipping rect
#define SPR_DisableScissor (*gEngfuncs.pfnSPR_DisableScissor)
//
#define FillRGBA(x,y,w,h,r,g,b,a) (*gEngfuncs.pfnFillRGBA)(x + gHUD.m_flHudLagOfs[0],y + gHUD.m_flHudLagOfs[1],w,h,r,g,b,a)


// ScreenHeight returns the height of the screen, in pixels
#define ScreenHeight (gHUD.m_scrinfo.iHeight)
// ScreenWidth returns the width of the screen, in pixels
#define ScreenWidth (gHUD.m_scrinfo.iWidth)
// Character Height of the text.
#define CharHeight (gHUD.m_scrinfo.iCharHeight)

#define BASE_XRES 640.f

// use this to project world coordinates to screen coordinates
#define XPROJECT(x)	( (1.0f+(x))*ScreenWidth*0.5f )
#define YPROJECT(y) ( (1.0f-(y))*ScreenHeight*0.5f )

static inline float XRES(float x)
{
	return x * ScreenWidth / 640;
}

static inline float YRES(float y)
{
	return y * ScreenHeight / 480;
}

#define GetScreenInfo (*gEngfuncs.pfnGetScreenInfo)
#define ServerCmd (*gEngfuncs.pfnServerCmd)
#define EngineClientCmd (*gEngfuncs.pfnClientCmd)
#define SetCrosshair (*gEngfuncs.pfnSetCrosshair)
#define AngleVectors (*gEngfuncs.pfnAngleVectors)


// Gets the height & width of a sprite,  at the specified frame
inline int SPR_Height( HSPRITE x, int f )	{ return gEngfuncs.pfnSPR_Height(x, f); }
inline int SPR_Width( HSPRITE x, int f )	{ return gEngfuncs.pfnSPR_Width(x, f); }

template<typename T, size_t N>
char (&ArraySizeHelper(T (&)[N]))[N];
#define ARRAYSIZE(x) sizeof(ArraySizeHelper(x))

#define max(a, b)  (((a) > (b)) ? (a) : (b))
#define min(a, b)  (((a) < (b)) ? (a) : (b))
//#define fabs(x)	   ((x) > 0 ? (x) : 0 - (x))

static size_t count_digits(int n)
{
	size_t result = 0;

	do {
		++result;
	} while ((n /= 10) != 0);

	return result;
}

static size_t get_map_name(char* dest, size_t count)
{
	auto map_path = gEngfuncs.pfnGetLevelName();

	const char* slash = strrchr(map_path, '/');
	if (!slash)
		slash = map_path - 1;

	const char* dot = strrchr(map_path, '.');
	if (!dot)
		dot = map_path + strlen(map_path);

	size_t bytes_to_copy = min(count - 1, static_cast<size_t>(dot - slash - 1));

	strncpy(dest, slash + 1, bytes_to_copy);
	dest[bytes_to_copy] = '\0';

	return bytes_to_copy;
}

static char* get_server_address()
{
	net_status_t netstatus{};
	gEngfuncs.pNetAPI->Status(&netstatus);
	return gEngfuncs.pNetAPI->AdrToString(&netstatus.remote_address);
}

static void sanitize_address(std::string& address)
{
	for (size_t i = 0; i < address.size(); ++i) {
		char c = address[i];
		if ((c >= '0' && c <= '9') || c == '.' || c == ':')
			continue;

		// Invalid character.
		address = address.substr(0, i);
		break;
	}
}

static size_t get_player_count()
{
	size_t player_count = 0;

	for (int i = 0; i < MAX_PLAYERS; ++i) {
		// Make sure the information is up to date.
		GetPlayerInfo(i + 1)->Update();

		// This player slot is empty.
		if (GetPlayerInfo(i + 1) == nullptr)
			continue;

		++player_count;
	}

	return player_count;
}

inline 	client_textmessage_t	*TextMessageGet( const char *pName ) { return gEngfuncs.pfnTextMessageGet( pName ); }
inline 	int						TextMessageDrawChar( int x, int y, int number, int r, int g, int b ) 
{ 
	return gEngfuncs.pfnDrawCharacter( x, y, number, r, g, b ); 
}

inline int DrawConsoleString( int x, int y, char *string )
{
	return gHUD.DrawConsoleStringWithColorTags(x, y, string);
}

inline void GetConsoleStringSize( char *string, int *width, int *height )
{
	gHUD.GetConsoleStringSizeWithColorTags(string, *width, *height);
}

inline int ConsoleStringLen( char *string )
{
	int _width, _height;
	GetConsoleStringSize( string, &_width, &_height );
	return _width;
}

inline void ConsolePrint( const char *string )
{
	gEngfuncs.pfnConsolePrint( color_tags::strip_color_tags_thread_unsafe(string) );
}

inline void CenterPrint( const char *string )
{
	gEngfuncs.pfnCenterPrint( color_tags::strip_color_tags_thread_unsafe(string) );
}


inline char *safe_strcpy( char *dst, const char *src, int len_dst)
{
	if( len_dst <= 0 )
	{
		return NULL; // this is bad
	}

	strncpy(dst,src,len_dst);
	dst[ len_dst - 1 ] = '\0';

	return dst;
}

inline int safe_sprintf( char *dst, int len_dst, const char *format, ...)
{
	if( len_dst <= 0 )
	{
		return -1; // this is bad
	}

	va_list v;

    va_start(v, format);

	_vsnprintf(dst,len_dst,format,v);

	va_end(v);

	dst[ len_dst - 1 ] = '\0';

	return 0;
}

// sound functions
inline void PlaySound( const char *szSound, float vol ) { gEngfuncs.pfnPlaySoundByName( szSound, vol ); }
inline void PlaySound( int iSound, float vol ) { gEngfuncs.pfnPlaySoundByIndex( iSound, vol ); }

void ScaleColors( int &r, int &g, int &b, int a );

float Length(const float *v);

#ifdef _MSC_VER
// disable 'possible loss of data converting float to int' warning message
#pragma warning( disable: 4244 )
// disable 'truncation from 'const double' to 'float' warning message
#pragma warning( disable: 4305 )
#endif

inline void UnpackRGB(int &r, int &g, int &b, unsigned long ulRGB)\
{\
	r = (ulRGB & 0xFF0000) >>16;\
	g = (ulRGB & 0xFF00) >> 8;\
	b = ulRGB & 0xFF;\
}

HSPRITE LoadSprite(const char *pszName);

// frac should always be multiplied by frametime
float lerp(float start, float end, float frac);
double dlerp(double start, double end, double frac);