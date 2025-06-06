//========= Copyright 1996-2022, Valve LLC, All rights reserved. ============
// C COMPATIBLE VERSION
// Purpose: Defines types used by the Steam API
//          This file is intended to be included in C code bases.

#ifndef STEAMTYPES_H
#define STEAMTYPES_H

#define S_CALLTYPE __cdecl
// WARNING: __cdecl is potentially #defined away in steam_api_common.h

// Steam-specific types. Defined here so this header file can be included in other code bases.
#ifndef WCHARTYPES_H
typedef unsigned char uint8;
#endif

#ifdef __GNUC__
#if __GNUC__ < 4
#error "Steamworks requires GCC 4.X (4.2 or 4.4 have been tested)"
#endif
#endif

#if defined(__LP64__) || defined(__x86_64__) || defined(_WIN64) || defined(__aarch64__) || defined(__s390x__)
#define X64BITS
#endif

#if !defined(VALVE_BIG_ENDIAN)
#if defined( __GNUC__ ) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define VALVE_BIG_ENDIAN 1
#endif
#endif

typedef unsigned char uint8;
typedef signed char int8;

#if defined( _WIN32 ) && !defined( __GNUC__ )

typedef __int16 int16;
typedef unsigned __int16 uint16;
typedef __int32 int32;
typedef unsigned __int32 uint32;
typedef __int64 int64;
typedef unsigned __int64 uint64;

typedef int64 lint64;
typedef uint64 ulint64;

#ifdef X64BITS
typedef __int64 intp;				// intp is an integer that can accommodate a pointer
typedef unsigned __int64 uintp;
#else
typedef __int32 intp;
typedef unsigned __int32 uintp;
#endif

#else // _WIN32

typedef short int16;
typedef unsigned short uint16;
typedef int int32;
typedef unsigned int uint32;
typedef long long int64;
typedef unsigned long long uint64;

typedef long int lint64;
typedef unsigned long int ulint64;

#ifdef X64BITS
typedef long long intp;
typedef unsigned long long uintp;
#else
typedef int intp;
typedef unsigned int uintp;
#endif

#endif // else _WIN32

typedef uint32 AppId_t;
typedef uint32 DepotId_t;
typedef uint32 RTime32;
typedef uint64 SteamAPICall_t;
typedef uint32 AccountID_t;
typedef uint64 PartyBeaconID_t;

#ifdef __cplusplus
const AppId_t k_uAppIdInvalid = 0x0;
const DepotId_t k_uDepotIdInvalid = 0x0;
const SteamAPICall_t k_uAPICallInvalid = 0x0;
const AccountID_t k_uAccountIdInvalid = 0;
const PartyBeaconID_t k_ulPartyBeaconIdInvalid = 0;
#endif

#ifdef __cplusplus
enum ESteamIPType
{
	k_ESteamIPTypeIPv4 = 0,
	k_ESteamIPTypeIPv6 = 1,
};

#pragma pack( push, 1 )

struct SteamIPAddress_t
{
	union {
		uint32			m_unIPv4;		// Host order
		uint8			m_rgubIPv6[16];	// Network order
		uint64			m_ipv6Qword[2];	// Big endian
	};
	ESteamIPType m_eType;
#endif

#ifdef __cplusplus
	bool IsSet() const
	{
		if (k_ESteamIPTypeIPv4 == m_eType)
			return m_unIPv4 != 0;
		else
			return m_ipv6Qword[0] != 0 || m_ipv6Qword[1] != 0;
	}

	static SteamIPAddress_t IPv4Any()
	{
		SteamIPAddress_t ipOut;
		ipOut.m_eType = k_ESteamIPTypeIPv4;
		ipOut.m_unIPv4 = 0;
		return ipOut;
	}

	static SteamIPAddress_t IPv6Any()
	{
		SteamIPAddress_t ipOut;
		ipOut.m_eType = k_ESteamIPTypeIPv6;
		ipOut.m_ipv6Qword[0] = 0;
		ipOut.m_ipv6Qword[1] = 0;
		return ipOut;
	}

	static SteamIPAddress_t IPv4Loopback()
	{
		SteamIPAddress_t ipOut;
		ipOut.m_eType = k_ESteamIPTypeIPv4;
		ipOut.m_unIPv4 = 0x7f000001;
		return ipOut;
	}

	static SteamIPAddress_t IPv6Loopback()
	{
		SteamIPAddress_t ipOut;
		ipOut.m_eType = k_ESteamIPTypeIPv6;
		ipOut.m_ipv6Qword[0] = 0;
		ipOut.m_ipv6Qword[1] = 0;
		ipOut.m_rgubIPv6[15] = 1;
		return ipOut;
	}
};
#endif // __cplusplus

#pragma pack( pop )

#endif // STEAMTYPES_H
