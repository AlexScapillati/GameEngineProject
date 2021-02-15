#include "CVector4.h"

/*-----------------------------------------------------------------------------------------
	Length operations
-----------------------------------------------------------------------------------------*/

// Reduce vector to unit length - member function
void CVector4::Normalise()
{
	TFloat32 lengthSq = x*x + y*y + z*z + w*w;

	// Ensure vector is not zero length (use BaseMath.h float approx. fn with default epsilon)
	if ( IsZero( lengthSq ) )
	{
		x = y = z = w = 0.0f;
	}
	else
	{
		TFloat32 invLength = InvSqrt( lengthSq );
		x *= invLength;
		y *= invLength;
		z *= invLength;
		w *= invLength;
	}
}


// Return unit length vector in the same direction as given one - non-member function
CVector4 Normalise( const CVector4& v )
{
	TFloat32 lengthSq = v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w;

	// Ensure vector is not zero length (use BaseMath.h float approx. fn with default epsilon)
	if (IsZero( lengthSq ) )
	{
		return CVector4(0.0f, 0.0f, 0.0f, 0.0f);
	}
	else
	{
		TFloat32 invLength = InvSqrt( lengthSq );
		return CVector4(v.x * invLength, v.y * invLength, v.z * invLength, v.w * invLength);
	}
}


/*---------------------------------------------------------------------------------------------
	Static constants
---------------------------------------------------------------------------------------------*/

// Standard vectors
const CVector4 CVector4::kZero(0.0f, 0.0f, 0.0f, 0.0f);
const CVector4 CVector4::kOne(1.0f, 1.0f, 1.0f, 1.0f);
const CVector4 CVector4::kOrigin(0.0f, 0.0f, 0.0f, 0.0f);
const CVector4 CVector4::kXAxis(1.0f, 0.0f, 0.0f, 0.0f);
const CVector4 CVector4::kYAxis(0.0f, 1.0f, 0.0f, 0.0f);
const CVector4 CVector4::kZAxis(0.0f, 0.0f, 1.0f, 0.0f);
const CVector4 CVector4::kWAxis(0.0f, 0.0f, 0.0f ,1.0f);

