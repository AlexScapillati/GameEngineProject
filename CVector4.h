#pragma once
#include <CVector3.h>
#include <CVector2.h>
#include <corecrt_math.h>
#include <GraphicsHelpers.cpp>

class CVector4
{
	
// Concrete class - public access
public:

	/*-----------------------------------------------------------------------------------------
		Constructors/Destructors
	-----------------------------------------------------------------------------------------*/

	// Default constructor - leaves values uninitialised (for performance)
	CVector4() {}

	// Construct by value
	CVector4
	(
		const float xIn,
		const float yIn,
		const float zIn,
		const float wIn
	) : x( xIn ), y( yIn ), z( zIn ), w( wIn )
	{}

	// Construct through pointer to four floats
	explicit CVector4( const float* pfElts )
	{

		x = pfElts[0];
		y = pfElts[1];
		z = pfElts[2];
		w = pfElts[3];

	}
	// 'explicit' disallows implicit conversion:  float* pf; CVector4 v = pf;
	// Need to use this constructor explicitly:   float* pf; CVector4 v = CVector3(pf);
	// Only applies to constructors that can take one parameter, used to avoid confusing code


	// Construct as vector between two 3D points (p1 to p2) and a w value (defaults to 0)
	CVector4
	(
		const CVector3& p1,
		const CVector3& p2,
		const float wIn = 0.0f
	) : x( p2.x - p1.x ), y( p2.y - p1.y ), z( p2.z - p1.z ), w( wIn )
	{}


	// Construct from a CVector2 and z & w values (default to 0)
	explicit CVector4
	(
		const CVector2& v,
		const float zIn = 0.0f,
		const float wIn = 0.0f
	) : x( v.x ), y( v.y ), z( zIn ), w( wIn )
	{}
	// Require explicit conversion from CVector2 (see above)

	// Construct from a CVector3 and a w value (defaults to 0)
	explicit CVector4
	(
		const CVector3& v,
		const float wIn = 0.0f
	) : x( v.x ), y( v.y ), z( v.z ), w( wIn )
	{}
	// Require explicit conversion from CVector3 (see above)


	// Copy constructor
    CVector4( const CVector4& v ) : x( v.x ), y( v.y ), z( v.z ), w( v.w )
	{}

	// Assignment operator
    CVector4& operator=( const CVector4& v )
	{
		if ( this != &v )
		{
			x = v.x;
			y = v.y;
			z = v.z;
			w = v.w;
		}
		return *this;
	}


	/*-----------------------------------------------------------------------------------------
		Setters
	-----------------------------------------------------------------------------------------*/

	// Set all four vector components
    void Set
	(
		const float xIn,
		const float yIn,
		const float zIn,
		const float wIn
	)
	{
		x = xIn;
		y = yIn;
		z = zIn;
		w = wIn;
	}

	// Set the vector through a pointer to four floats
    void Set( const float* pfElts )
	{
		x = pfElts[0];
		y = pfElts[1];
		z = pfElts[2];
		w = pfElts[3];
	}

	// Set as vector between two 3D points (p1 to p2) and a w value (defaults to 0)
    void Set
	(
		const CVector3& p1,
		const CVector3& p2,
		const float wIn = 0.0f
	)
	{
		x = p2.x - p1.x;
		y = p2.y - p1.y;
		z = p2.z - p1.z;
		w = wIn;
	}

	// Set the vector to (0,0,0,0)
    void SetZero()
	{
		x = y = z = w = 0.0f;
	}


	/*-----------------------------------------------------------------------------------------
		Array access
	-----------------------------------------------------------------------------------------*/

	// Access the x, y, z & w components in array style (i.e. v[0], v[1],... same as v.x, v.y,...)
    float& operator[]( const int index )
	{
		return (&x)[index];
	}

	// Access the x, y, z & w elements in array style - const result
	const float& operator[]( const int index ) const
	{
		return (&x)[index];
	}


	/*-----------------------------------------------------------------------------------------
		Member Operators
	-----------------------------------------------------------------------------------------*/
	// Non-member versions defined after the class definition

	///////////////////////////////
	// Reinterpretation

	// Reinterpreting vector types is not guaranteed to be portable - but improves
	// efficiency and is highly convenient

	// Return reference to x & y components as CVector2.  Efficient but non-portable
	CVector2& Vector2()
	{
		return *reinterpret_cast<CVector2*>(&x);
	}

	// Return const reference to x & y components as CVector2. Efficient but non-portable
	const CVector2& Vector2() const
	{
		return *reinterpret_cast<const CVector2*>(&x);
	}


	// Return reference to x, y & z components as a CVector3. Efficient but non-portable
	CVector3& Vector3()
	{
		return *reinterpret_cast<CVector3*>(&x);
	}

	// Return const reference to x, y & z components as CVector3. Efficient but non-portable
	const CVector3& Vector3() const
	{
		return *reinterpret_cast<const CVector3*>(&x);
	}


	///////////////////////////////
	// Addition / subtraction

	// Add another vector to this vector
    CVector4& operator+=( const CVector4& v )
	{
		x += v.x;
		y += v.y;
		z += v.z;
		w += v.w;
		return *this;
	}

	// Subtract another vector from this vector
    CVector4& operator-=( const CVector4& v )
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		w -= v.w;
		return *this;
	}


	////////////////////////////////////
	// Scalar multiplication & division

	// Multiply this vector by a scalar
	CVector4& operator*=( const float s )
	{
		x *= s;
		y *= s;
		z *= s;
		w *= s;
		return *this;
	}

	// Divide this vector by a scalar
    CVector4& operator/=( const float s )
	{
		x /= s;
		y /= s;
		z /= s;
		w /= s;
		return *this;

	}


	////////////////////////////////////
	// Other operations

	// Dot product of this with another vector
    float Dot( const CVector4& v ) const
	{
	    return x*v.x + y*v.y + z*v.z + w*v.w;
	}
	
	
	// Cross product of this with another vector
    CVector4 Cross(	const CVector4& v ) const
	{
		return CVector4(y*v.z - z*v.y, z*v.w - w*v.z,
		                w*v.x - x*v.w, x*v.y - y*v.x);
	}


	/*-----------------------------------------------------------------------------------------
		Length operations
	-----------------------------------------------------------------------------------------*/
	// Non-member versions defined after the class definition

	// Return length of this vector
	float Length() const
	{
		return sqrtf( x*x + y*y + z*z + w*w );
	}

	// Return squared length of this vector
	// More efficient than Length when exact value is not required (e.g. for comparisons)
	// Use InvSqrt( LengthSquared(...) ) to calculate 1 / length more efficiently
	float LengthSquared() const
	{
		return x*x + y*y + z*z + w*w;
	}

	// Reduce vector to unit length
    void Normalise();


	/*---------------------------------------------------------------------------------------------
		Data
	---------------------------------------------------------------------------------------------*/
    
    // Vector components
    float x;
	float y;
	float z;
	float w;

	// Standard vectors
	static const CVector4 kZero;
	static const CVector4 kOne;
	static const CVector4 kOrigin;
	static const CVector4 kXAxis;
	static const CVector4 kYAxis;
	static const CVector4 kZAxis;
	static const CVector4 kWAxis;
};


/*-----------------------------------------------------------------------------------------
	Non-member Operators
-----------------------------------------------------------------------------------------*/

///////////////////////////////
// Comparison

// Vector equality
// Uses BaseMath.h float approximation function 'AreEqual' with default margin of error
inline bool operator==
(
	const CVector4& v1,
	const CVector4& v2
)
{
	return AreEqual( v1.x, v2.x ) && AreEqual( v1.y, v2.y ) &&
	       AreEqual( v1.z, v2.z ) && AreEqual( v1.w, v2.w );
}

// Vector inequality
// Uses BaseMath.h float approximation function 'AreEqual' with default margin of error
inline bool operator!=
(
	const CVector4& v1,
	const CVector4& v2
)
{
	return !AreEqual( v1.x, v2.x ) || !AreEqual( v1.y, v2.y ) ||
		   !AreEqual( v1.z, v2.z ) || !AreEqual( v1.w, v2.w );
}


///////////////////////////////
// Addition / subtraction

// Vector addition
inline CVector4 operator+
(
	const CVector4& v1,
	const CVector4& v2
)
{
	return CVector4(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w);
}

// Vector subtraction
inline CVector4 operator-
(
	const CVector4& v1,
	const CVector4& v2
)
{
	return CVector4(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w);
}

// Unary positive (i.e. a = +v, included for completeness)
inline CVector4 operator+( const CVector4& v )
{
	return v;
}

// Unary negation (i.e. a = -v)
inline CVector4 operator-( const CVector4& v )
{
	return CVector4(-v.x, -v.y, -v.z, -v.w);
}


////////////////////////////////////
// Scalar multiplication & division

// Vector multiplied by scalar
inline CVector4 operator*
(
	const CVector4& v,
	const float  s
)
{
	return CVector4(v.x*s, v.y*s, v.z*s, v.w*s);
}

// Scalar multiplied by vtor
inline CVector4 operator*
(
	const float  s,
	const CVector4& v
)
{
	return CVector4(v.x*s, v.y*s, v.z*s, v.w*s);
}

// Vector divided by scalar
inline CVector4 operator/
(
	const CVector4& v,
	const float  s
)
{

	return CVector4(v.x/s, v.y/s, v.z/s, v.w/s);

}


////////////////////////////////////
// Other operations

// Dot product of two given vectors (order not important) - non-member version
inline float Dot
(
	const CVector4& v1,
	const CVector4& v2
)
{
    return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z + v1.w*v2.w;
}

// Cross product of two given vectors (order is important) - non-member version
inline CVector4 Cross
(
	const CVector4& v1,
	const CVector4& v2
)
{
	return CVector4(v1.y*v2.z - v1.z*v2.y, v1.z*v2.w - v1.w*v2.z,
	                v1.w*v2.x - v1.x*v2.w, v1.x*v2.y - v1.y*v2.x);
}


/*-----------------------------------------------------------------------------------------
	Non-Member Length operations
-----------------------------------------------------------------------------------------*/

// Return length of given vector
inline float Length( const CVector4& v )
{
	return sqrtf( v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w );
}

// Return squared length of given vector
// More efficient than Length when exact value is not required (e.g. for comparisons)
// Use InvSqrt( LengthSquared(...) ) to calculate 1 / length more efficiently
inline float LengthSquared( const CVector4& v )
{
	return v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w;
}

// Return unit length vector in the same direction as given one
CVector4 Normalise( const CVector4& v );



