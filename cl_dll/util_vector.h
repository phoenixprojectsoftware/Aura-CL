/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
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
//  Legacy_Vector.h
// A subset of the extdll.h in the project HL Entity DLL
//

// Misc C-runtime library headers
#include "stdio.h"
#include "stdlib.h"
#include "math.h"

// Header file containing definition of globalvars_t and entvars_t
typedef unsigned int	func_t;		//
typedef unsigned int	string_t;	// from engine's pr_comp.h;
typedef float vec_t;				// needed before including progdefs.h

//=========================================================
// 2DVector - used for many pathfinding and many other 
// operations that are treated as planar rather than 3d.
//=========================================================
class Legacy_Vector2D
{
public:
	inline Legacy_Vector2D(void)									{ }
	inline Legacy_Vector2D(float X, float Y)						{ x = X; y = Y; }
	inline Legacy_Vector2D operator+(const Legacy_Vector2D& v)	const	{ return Legacy_Vector2D(x+v.x, y+v.y);	}
	inline Legacy_Vector2D operator-(const Legacy_Vector2D& v)	const	{ return Legacy_Vector2D(x-v.x, y-v.y);	}
	inline Legacy_Vector2D operator*(float fl)				const	{ return Legacy_Vector2D(x*fl, y*fl);	}
	inline Legacy_Vector2D operator/(float fl)				const	{ return Legacy_Vector2D(x/fl, y/fl);	}
	
	inline float Length(void)						const	{ return (float)sqrt(x*x + y*y );		}

	inline Legacy_Vector2D Normalize ( void ) const
	{
		Legacy_Vector2D vec2;

		float flLen = Length();
		if ( flLen == 0 )
		{
			return Legacy_Vector2D( (float)0, (float)0 );
		}
		else
		{
			flLen = 1 / flLen;
			return Legacy_Vector2D( x * flLen, y * flLen );
		}
	}

	vec_t	x, y;
};

#undef DotProduct
inline float DotProduct(const Legacy_Vector2D& a, const Legacy_Vector2D& b) { return( a.x*b.x + a.y*b.y ); }
inline Legacy_Vector2D operator*(float fl, const Legacy_Vector2D& v)	{ return v * fl; }

//=========================================================
// 3D Legacy_Vector
//=========================================================
class Legacy_Vector						// same data-layout as engine's vec3_t,
{								//		which is a vec_t[3]
public:
	// Construction/destruction
	inline Legacy_Vector(void)								{ }
	inline Legacy_Vector(float X, float Y, float Z)		{ x = X; y = Y; z = Z;	}
	// Sabian: commented out the extra declarations. This seems to work.
	//inline Legacy_Vector(double X, double Y, double Z)		{ x = (float)X; y = (float)Y; z = (float)Z;	}
	//inline Legacy_Vector(int X, int Y, int Z)				{ x = (float)X; y = (float)Y; z = (float)Z;	}
	inline Legacy_Vector(const Legacy_Vector& v)					{ x = v.x; y = v.y; z = v.z;				} 
	inline Legacy_Vector(float rgfl[3])					{ x = rgfl[0]; y = rgfl[1]; z = rgfl[2];	}

	// Operators
	inline Legacy_Vector operator-(void) const				{ return Legacy_Vector(-x,-y,-z);				}
	inline int operator==(const Legacy_Vector& v) const	{ return x==v.x && y==v.y && z==v.z;	}
	inline int operator!=(const Legacy_Vector& v) const	{ return !(*this==v);					}
	inline Legacy_Vector operator+(const Legacy_Vector& v) const	{ return Legacy_Vector(x+v.x, y+v.y, z+v.z);	}
	inline Legacy_Vector operator-(const Legacy_Vector& v) const	{ return Legacy_Vector(x-v.x, y-v.y, z-v.z);	}
	inline Legacy_Vector operator*(float fl) const			{ return Legacy_Vector(x*fl, y*fl, z*fl);		}
	inline Legacy_Vector operator/(float fl) const			{ return Legacy_Vector(x/fl, y/fl, z/fl);		}
	
	// Methods
	inline void CopyToArray(float* rgfl) const		{ rgfl[0] = x, rgfl[1] = y, rgfl[2] = z; }
	inline float Length(void) const					{ return (float)sqrt(x*x + y*y + z*z); }
	inline float LengthSqr(void) const				{ return (float)(x*x + y*y + z*z); }
	operator float *()								{ return &x; } // Vectors will now automatically convert to float * when needed
	operator const float *() const					{ return &x; } // Vectors will now automatically convert to float * when needed
	inline Legacy_Vector Normalize(void) const
	{
		float flLen = Length();
		if (flLen == 0) return Legacy_Vector(0,0,1); // ????
		flLen = 1 / flLen;
		return Legacy_Vector(x * flLen, y * flLen, z * flLen);
	}

	inline Legacy_Vector2D Make2D ( void ) const
	{
		Legacy_Vector2D	Vec2;

		Vec2.x = x;
		Vec2.y = y;

		return Vec2;
	}
	inline float Length2D(void) const					{ return (float)sqrt(x*x + y*y); }

	// Members
	vec_t x, y, z;
};
inline Legacy_Vector operator*(float fl, const Legacy_Vector& v)	{ return v * fl; }
inline float DotProduct(const Legacy_Vector& a, const Legacy_Vector& b) { return(a.x*b.x+a.y*b.y+a.z*b.z); }
inline Legacy_Vector CrossProduct(const Legacy_Vector& a, const Legacy_Vector& b) { return Legacy_Vector( a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x ); }

#define InvPitch(x) Legacy_Vector(-x[0], x[1], x[2])


#ifndef DID_VEC3_T_DEFINE
#define DID_VEC3_T_DEFINE
#define vec3_t Legacy_Vector
#endif