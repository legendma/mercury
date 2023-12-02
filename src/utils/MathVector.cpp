#include <cstdio>
#include <math.h>

#include "Math.hpp"
#include "Utilities.hpp"


/*******************************************************************
*
*   Math_Float2Addition()
*
*   DESCRIPTION:
*       Adds two 2D vectors together. 
*
*******************************************************************/

Float2 Math_Float2Addition( const Float2 A, const Float2 B )
{
    Float2 C = {};
    for( uint32_t i = 0; i < cnt_of_array( C.f ); i++ )
    {
      C.f[i] = A.f[i] + B.f[i];
    }
    return C;

} /*  Math_Float2Addition() */


/*******************************************************************
*
*   Math_Float2Subtraction()
*
*   DESCRIPTION:
*       subtract two 2D vectors together.
*
*******************************************************************/

Float2 Math_Float2Subtraction( const Float2 A, const Float2 B)
{
    Float2 C = {};
    for( uint32_t i = 0; i < cnt_of_array( C.f ); i++ )
    {
        C.f[i] = A.f[i] - B.f[i];
    }
    return C;

} /*  Math_Float2Subtraction() */


/*******************************************************************
*
*   Math_Float3Addition()
*
*   DESCRIPTION:
*       Adds two 3D vectors together.
*
*******************************************************************/

Float3 Math_Float3Addition( const Float3 A, const Float3 B)
{
    Float3 C = {};
    for( uint32_t i = 0; i < cnt_of_array( C.f ); i++ )
    {
        C.f[i] = A.f[i] + B.f[i];
    }
    return C;

} /*  Math_Float3Addition() */


/*******************************************************************
*
*   Math_Float3Subtraction()
*
*   DESCRIPTION:
*       subtract two 3D vectors together.
*
*******************************************************************/

Float3 Math_Float3Subtraction( const Float3 A, const Float3 B)
{
    Float3 C = {};
    for( uint32_t i = 0; i < cnt_of_array( C.f ); i++ )
    {
        C.f[i] = A.f[i] - B.f[i];
    }
    return C;

} /*  Math_Float3Subtraction() */


/*******************************************************************
*
*   Math_Float4Addition()
*
*   DESCRIPTION:
*       Adds two 4D vectors together.
*
*******************************************************************/

Float4 Math_Float4Addition( const Float4 A, const Float4 B )
{
    Float4 C = {};
    for( uint32_t i = 0; i < cnt_of_array( C.f ); i++ )
    {
        C.f[i] = A.f[i] + B.f[i];
    }
    return C;

} /*  Math_Float4Addition() */


/*******************************************************************
*
*   Math_Float4Subtraction()
*
*   DESCRIPTION:
*       subtract two 4D vectors together.
*
*******************************************************************/

Float4 Math_Float4Subtraction( const Float4 A, const Float4 B)
{
    Float4 C = {};
    for( uint32_t i = 0; i < cnt_of_array( C.f ); i++ )
    {
        C.f[i] = A.f[i] - B.f[i];
    }
    return C;

} /*  Math_Float4Subtraction() */


/*******************************************************************
*
*   Math_Float2ScalarMultiply()
*
*   DESCRIPTION:
*       Applys a scaler to the vector
*
*******************************************************************/

Float2 Math_Float2ScalarMultiply(const Float2 A, const float scalar )
{
    Float2 B = {};
    for( uint32_t i = 0; i < cnt_of_array( B.f ); i++ )
    {
       B.f[i] = A.f[i] * scalar;
    }
    return B;

} /*  Math_Float2ScalarMultiply() */


/*******************************************************************
*
*   Math_Float3ScalarMultiply()
*
*   DESCRIPTION:
*       Applys a scaler to the vector
*
*******************************************************************/

Float3 Math_Float3ScalarMultiply( const Float3 A, const float scalar )
{
    Float3 B = {};
    for( uint32_t i = 0; i < cnt_of_array( B.f ); i++ )
    {
        B.f[i] = A.f[i] * scalar;
    }
    return B;

} /*  Math_Float3ScalarMultiply() */


/*******************************************************************
*
*   Math_Float4ScalarMultiply()
*
*   DESCRIPTION:
*       Applys a scaler to the vector
*
*******************************************************************/

Float4 Math_Float4ScalarMultiply( const Float4 A, const float scalar )
{
    Float4 B = {};
    for( uint32_t i = 0; i < cnt_of_array( B.f ); i++ )
    {
        B.f[i] = A.f[i] * scalar;
    }
    return B;

} /*  Math_Float4ScalarMultiply() */


/*******************************************************************
*
*   Math_Float2Negative()
*
*   DESCRIPTION:
*       Calculates the negative of the vector.
*       This is the vector going in the oposite direction
*
*******************************************************************/

Float2 Math_Float2Negative( const Float2 A )
{
    Float2 B = {};
    for( uint32_t i = 0; i < cnt_of_array( B.f ); i++ )
    {
        B.f[i] = -A.f[i];
    }
    return B;

} /*  Math_Float2Negative() */


/*******************************************************************
*
*   Math_Float3Negative()
*
*   DESCRIPTION:
*       Calculates the negative of the vector.
*       This is the vector going in the oposite direction
*
*******************************************************************/

Float3 Math_Float3Negative( const Float3 A )
{
    Float3 B = {};
    for( uint32_t i = 0; i < cnt_of_array( B.f ); i++ )
    {
        B.f[i] = -A.f[i];
    }
    return B;

} /*  Math_Float3Negative() */


/*******************************************************************
*
*   Math_Float4Negative()
*
*   DESCRIPTION:
*       Calculates the negative of the vector.  
*       This is the vector going in the oposite direction
*
*******************************************************************/

Float4 Math_Float4Negative( const Float4 A )
{
    Float4 B = {};
    for( uint32_t i = 0; i < cnt_of_array( B.f ); i++ )
    {
        B.f[i] = -A.f[i];
    }
    return B;

} /*  Math_Float4Negative() */


/*******************************************************************
*
*   Math_Float2DotProduct()
*
*   DESCRIPTION:
*       Calculates the dot product between the two vectors
*
*******************************************************************/

float Math_Float2DotProduct( const Float2 A, const Float2 B )
{
    return (A.v.x * B.v.x + A.v.y * B.v.y);
} /*  Math_Float2DotProduct() */


/*******************************************************************
*
*   Math_Float3DotProduct()
*
*   DESCRIPTION:
*       Calculates the dot product between the two vectors
*
*******************************************************************/

float Math_Float3DotProduct( const Float3 A, const Float3 B )
{
    return (A.v.x * B.v.x + A.v.y * B.v.y + A.v.z * B.v.z);
} /*  Math_Float3DotProduct() */


/*******************************************************************
*
*   Math_Float4DotProduct()
*
*   DESCRIPTION:
*       Calculates the dot product between the two vectors
*
*******************************************************************/

float Math_Float4DotProduct( const Float4 A, const Float4 B )
{
    return (A.v.x * B.v.x + A.v.y * B.v.y + A.v.z * B.v.z + A.v.w * B.v.w);
} /*  Math_Float3DotProduct() */


/*******************************************************************
*
*   Math_Float2PseudoCrossProduct()
*
*   DESCRIPTION:
*       Calculates the pseudo cross product between the two vectors in 2D.
*       The result corresponds to the signed area of the parallegram 
*       formed from vectors A and B.
*
*******************************************************************/

float Math_Float2PseudoCrossProduct( const Float2 A, const Float2 B )
{
    Float2 C = {};
    C.v.x = -B.v.y;
    C.v.y = B.v.x;

    return (Math_Float2DotProduct(C,A));
} /*  Math_Float2PseudoCrossProduct() */


/*******************************************************************
*
*   Math_Float3CrossProduct()
*
*   DESCRIPTION:
*       Calculates the cross product between the two vectors
*
*******************************************************************/

Float3 Math_Float3CrossProduct( const Float3 A, const Float3 B )
{
    Float3 C = {};
    float t1 = A.v.x - B.v.y;
    float t2 = B.v.y + B.v.z;
    float t3 = A.v.x * B.v.z;
    float t4 = t1*t2 - t3;
    C.v.x = B.v.y * (t1 - A.v.z) - t4;
    C.v.y = A.v.z * B.v.x - t3;
    C.v.z = t4 - A.v.y * ( B.v.x - t2);
    return C;

} /*  Math_Float3CrossProduct() */


/*******************************************************************
*
*   Math_Float3CrossProductMagnitude()
*
*   DESCRIPTION:
*       Calculates the Magnitude of the cross product of the two vectors
*
*******************************************************************/

float Math_Float3CrossProductMagnitude( const Float3 A, const Float3 B )
{
    float magA = Math_Float3Magnitude( A );
    float magB = Math_Float3Magnitude( B );
    float angle = Math_AngleBetweenFloat3 (A, B);

    return (magA * magB * (float) sin( angle ));

} /*  Math_Float3CrossProductMagnitude() */


/*******************************************************************
*
*   Math_Float4CrossProductMagnitude()
*
*   DESCRIPTION:
*       Calculates the Magnitude of the cross product of the two vectors
*
*******************************************************************/

float Math_Float4CrossProductMagnitude( const Float4 A, const Float4 B )
{
    float magA = Math_Float4Magnitude( A );
    float magB = Math_Float4Magnitude( B );
    float angle = Math_AngleBetweenFloat4( A, B );

    return (magA * magB * (float)sin( angle ));

} /*  Math_Float4CrossProductMagnitude() */


/*******************************************************************
*
*   Math_Float3BoxProduct()
*
*   DESCRIPTION:
*       Calculates the triple scalar product between three vectors
*       in 3D space (A x B) * C. The value corresonds to the signed 
*       volume of the parallelpiped formed by the three vectors
*
*******************************************************************/

float Math_Float3BoxProduct( const Float3 A, const Float3 B, const Float3 C )
{
    Float3 AxB = {};
    AxB = Math_Float3CrossProduct(A, B);

    return (Math_Float3DotProduct(AxB, C));

} /*  Math_Float3BoxProduct() */


/*******************************************************************
*
*   Math_Float2TrippleProduct()
*
*   DESCRIPTION:
*       Calculates the triple product for three vectors A x (B x C) 
*
*******************************************************************/

Float2 Math_Float2TrippleProduct( const Float2 A, const Float2 B, const Float2 C )
{
    float scalar1 = Math_Float2DotProduct( A, C );
    float scalar2 = Math_Float2DotProduct( A, B );
    Float2 modB = Math_Float2ScalarMultiply( B, scalar1 );
    Float2 modC = Math_Float2ScalarMultiply( C, scalar2);

    return ( Math_Float2Subtraction( modB, modC));

} /*  Math_Float2TrippleProduct() */


/*******************************************************************
*
*   Math_Float3TrippleProduct()
*
*   DESCRIPTION:
*       Calculates the triple product for three vectors A x (B x C)
*
*******************************************************************/

Float3 Math_Float3TrippleProduct( const Float3 A, const Float3 B, const Float3 C )
{
    float scalar1 = Math_Float3DotProduct( A, C );
    float scalar2 = Math_Float3DotProduct( A, B );
    Float3 modB = Math_Float3ScalarMultiply( B, scalar1 );
    Float3 modC = Math_Float3ScalarMultiply( C, scalar2 );

    return (Math_Float3Subtraction( modB, modC ));

} /*  Math_Float3TrippleProduct() */


/*******************************************************************
*
*   Math_Float4TrippleProduct()
*
*   DESCRIPTION:
*       Calculates the triple product for three vectors A x (B x C)
*
*******************************************************************/

Float4 Math_Float4TrippleProduct( const Float4 A, const Float4 B, const Float4 C )
{
    float scalar1 = Math_Float4DotProduct( A, C );
    float scalar2 = Math_Float4DotProduct( A, B );
    Float4 modB = Math_Float4ScalarMultiply( B, scalar1 );
    Float4 modC = Math_Float4ScalarMultiply( C, scalar2 );

    return (Math_Float4Subtraction( modB, modC ));

} /*  Math_Float3TrippleProduct() */


/*******************************************************************
*
*   Math_Float2Magnitude()
*
*   DESCRIPTION:
*       calculates the magnetude or length of the vector
*
*******************************************************************/

float Math_Float2Magnitude( const Float2 A )
{
    return ((float) sqrt( Math_Float2DotProduct( A, A )));

} /*  Math_Float2Magnitude */


/*******************************************************************
*
*   Math_Float3Magnitude()
*
*   DESCRIPTION:
*       calculates the magnetude or length of the vector
*
*******************************************************************/

float Math_Float3Magnitude( const Float3 A )
{
    return ((float)sqrt( Math_Float3DotProduct( A, A ) ));

} /*  Math_Float3Magnitude */


/*******************************************************************
*
*   Math_Float4Magnitude()
*
*   DESCRIPTION:
*       calculates the magnetude or length of the vector
*
*******************************************************************/

float Math_Float4Magnitude( const Float4 A )
{
    return ((float) sqrt( Math_Float4DotProduct( A, A ) ));

} /*  Math_Float4Magnitude */


/*******************************************************************
*
*   Math_Float2SquareMagnitude()
*
*   DESCRIPTION:
*       calculates the magnetude^2 of the vector
*
*******************************************************************/

float Math_Float2SquareMagnitude( const Float2 A )
{
    return ( Math_Float2DotProduct( A, A ) );

} /*  Math_Float2SquareMagnitude */


/*******************************************************************
*
*   Math_Float3SquareMagnitude()
*
*   DESCRIPTION:
*       calculates the magnetude^2 of the vector
*
*******************************************************************/

float Math_Float3SquareMagnitude( const Float3 A )
{
    return (Math_Float3DotProduct( A, A ) );

} /*  Math_Float3SquareMagnitude */


/*******************************************************************
*
*   Math_Float4SquareMagnitude()
*
*   DESCRIPTION:
*       calculates the magnetude^2 of the vector
*
*******************************************************************/

float Math_Float4SquareMagnitude( const Float4 A )
{
    return ( Math_Float4DotProduct( A, A ) );

} /*  Math_Float4SquareMagnitude */


/*******************************************************************
*
*   Math_AngleBetweenFloat2()
*
*   DESCRIPTION:
*       Calculates the angle between the two vectors
*
*******************************************************************/

float Math_AngleBetweenFloat2( const Float2 A, const Float2 B )
{
    float magA = Math_Float2Magnitude( A );
    float magB = Math_Float2Magnitude( B );
    float AdotB = Math_Float2DotProduct(  A, B );

    return ((float) acos(AdotB/(magA * magB)) );

} /*  Math_AngleBetweenFloat2() */


/*******************************************************************
*
*   Math_AngleBetweenFloat3()
*
*   DESCRIPTION:
*       Calculates the angle between the two vectors
*
*******************************************************************/

float Math_AngleBetweenFloat3( const Float3 A, const Float3 B )
{
    float magA = Math_Float3Magnitude( A );
    float magB = Math_Float3Magnitude( B );
    float AdotB = Math_Float3DotProduct( A, B );

    return ((float)acos( AdotB / (magA * magB) ));

} /*  Math_AngleBetweenFloat3() */


/*******************************************************************
*
*   Math_AngleBetweenFloat4()
*
*   DESCRIPTION:
*       Calculates the angle between the two vectors
*
*******************************************************************/

float Math_AngleBetweenFloat4( const Float4 A, const Float4 B )
{
    float magA = Math_Float4Magnitude( A );
    float magB = Math_Float4Magnitude( B );
    float AdotB = Math_Float4DotProduct( A, B );

    return ((float) acos( AdotB / (magA * magB) ));

} /*  Math_AngleBetweenFloat4() */


/*******************************************************************
*
*   Math_IsAngleObtuseFloat2()
*
*   DESCRIPTION:
*       is the Angle between two vectors > 90 degrees
*
*******************************************************************/

bool Math_IsAngleObtuseFloat2( const Float2 A, const Float2 B )
{
    if( Math_Float2DotProduct( A, B ) < 0.0)
    {
    return true;
    }
    return false;

} /*  Math_IsAngleObtuseFloat2() */


/*******************************************************************
*
*   Math_IsAngleObtuseFloat3()
*
*   DESCRIPTION:
*       is the Angle between two vectors > 90 degrees
*
*******************************************************************/

bool Math_IsAngleObtuseFloat3( const Float3 A, const Float3 B )
{
    if( Math_Float3DotProduct( A, B ) < 0.0 )
    {
        return true;
    }
    return false;

} /*  Math_IsAngleObtuseFloat3() */


/*******************************************************************
*
*   Math_IsAngleObtuseFloat4()
*
*   DESCRIPTION:
*       is the Angle between two vectors > 90 degrees
*
*******************************************************************/

bool Math_IsAngleObtuseFloat4( const Float4 A, const Float4 B )
{
    if( Math_Float4DotProduct( A, B ) < 0.0 )
    {
        return true;
    }
    return false;

} /*  Math_IsAngleObtuseFloat4() */


/*******************************************************************
*
*   Math_IsAngleAcuteFloat2()
*
*   DESCRIPTION:
*       is the Angle between two vectors < 90 degrees
*
*******************************************************************/

bool Math_IsAngleAcuteFloat2( const Float2 A, const Float2 B )
{
    if( Math_Float2DotProduct( A, B ) > 0.0 )
    {
        return true;
    }
    return false;

} /*  Math_IsAngleAcuteFloat2() */


/*******************************************************************
*
*   Math_IsAngleAcuteFloat3()
*
*   DESCRIPTION:
*       is the Angle between two vectors < 90 degrees
*
*******************************************************************/

bool Math_IsAngleAcuteFloat3( const Float3 A, const Float3 B )
{
    if( Math_Float3DotProduct( A, B ) > 0.0 )
    {
        return true;
    }
    return false;

} /*  Math_IsAngleAcuteFloat3() */


/*******************************************************************
*
*   Math_IsAngleAcuteFloat4()
*
*   DESCRIPTION:
*       is the Angle between two vectors < 90 degrees
*
*******************************************************************/

bool Math_IsAngleAcuteFloat4( const Float4 A, const Float4 B )
{
    if( Math_Float4DotProduct( A, B ) > 0.0 )
    {
        return true;
    }
    return false;

} /*  Math_IsAngleAcuteFloat4() */


/*******************************************************************
*
*   Math_IsAnglePerpendicularFloat2()
*
*   DESCRIPTION:
*       Are the two vectors perpendicular?
*
*******************************************************************/

bool Math_IsAnglePerpendicularFloat2( const Float2 A, const Float2 B )
{
    if( Math_Float2DotProduct( A, B ) == 0.0 )
    {
        return true;
    }
    return false;

} /*  Math_IsAnglePerpendicularFloat2() */


/*******************************************************************
*
*   Math_IsAnglePerpendicularFloat3()
*
*   DESCRIPTION:
*        Are the two vectors perpendicular?
*
*******************************************************************/

bool Math_IsAnglePerpendicularFloat3( const Float3 A, const Float3 B )
{
    if( Math_Float3DotProduct( A, B ) == 0.0 )
    {
        return true;
    }
    return false;

} /*  Math_IsAnglePerpendicularFloat3() */


/*******************************************************************
*
*   Math_IsAnglePerpendicularFloat4()
*
*   DESCRIPTION:
*        Are the two vectors perpendicular?
*
*******************************************************************/

bool Math_IsAnglePerpendicularFloat4( const Float4 A, const Float4 B )
{
    if( Math_Float4DotProduct( A, B ) == 0.0 )
    {
        return true;
    }
    return false;

} /*  Math_IsAnglePerpendicularFloat4() */


/*******************************************************************
*
*   Math_DistanceBetweenFloat2()
*
*   DESCRIPTION:
*        Calculate the magnitude of the B - A vector
*
*******************************************************************/

float Math_DistanceBetweenFloat2( const Float2 A, const Float2 B )
{
    Float2 C = {};
    C = Math_Float2Subtraction( B, A );

    return ( Math_Float2Magnitude( C ));

} /*  Math_DistanceBetweenFloat2() */


/*******************************************************************
*
*   Math_DistanceBetweenFloat3()
*
*   DESCRIPTION:
*        Calculate the magnitude of the B - A vector
*
*******************************************************************/

float Math_DistanceBetweenFloat3( const Float3 A, const Float3 B )
{
    Float3 C = {};
    C = Math_Float3Subtraction( B, A );

    return (Math_Float3Magnitude( C ));

} /*  Math_DistanceBetweenFloat3() */


/*******************************************************************
*
*   Math_DistanceBetweenFloat4()
*
*   DESCRIPTION:
*        Calculate the magnitude of the B - A vector
*
*******************************************************************/

float Math_DistanceBetweenFloat4( const Float4 A, const Float4 B )
{
    Float4 C = {};
    C = Math_Float4Subtraction( B, A );

    return (Math_Float4Magnitude( C ));

} /*  Math_DistanceBetweenFloat4() */


/*******************************************************************
*
*   Math_SquaredDistanceBetweenFloat2()
*
*   DESCRIPTION:
*        Calculate the magnitude^2 of the B - A vector
*
*******************************************************************/

float Math_SquaredDistanceBetweenFloat2( const Float2 A, const Float2 B )
{
    Float2 C = {};
    C = Math_Float2Subtraction( B, A );

    return ( Math_Float2SquareMagnitude( C ));

} /*  Math_SquaredDistanceBetweenFloat2() */


/*******************************************************************
*
*   Math_SquaredDistanceBetweenFloat3()
*
*   DESCRIPTION:
*        Calculate the magnitude^2 of the B - A vector
*
*******************************************************************/

float Math_SquaredDistanceBetweenFloat3( const Float3 A, const Float3 B )
{
    Float3 C = {};
    C = Math_Float3Subtraction( B, A );

    return (Math_Float3SquareMagnitude( C ));

} /*  Math_SquaredDistanceBetweenFloat3() */


/*******************************************************************
*
*   Math_SquaredDistanceBetweenFloat4()
*
*   DESCRIPTION:
*        Calculate the magnitude^2 of the B - A vector
*
*******************************************************************/

float Math_SquaredDistanceBetweenFloat4( const Float4 A, const Float4 B )
{
    Float4 C = {};
    C = Math_Float4Subtraction( B, A );

    return (Math_Float4SquareMagnitude( C ));

} /*  Math_SquaredDistanceBetweenFloat4() */
