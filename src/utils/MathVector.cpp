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

Float2 Math_Float2Negative( const Float2 a )
{
Float2 b;
b.v.x = -a.v.x;
b.v.y = -a.v.y;

return( b );

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

Float3 Math_Float3Negative( const Float3 a )
{
Float3 b;
b.v.x = -a.v.x;
b.v.y = -a.v.y;
b.v.z = -a.v.z;

return( b );

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
Float4 B;
B.v.x = -A.v.x;
B.v.y = -A.v.y;
B.v.z = -A.v.z;
B.v.w = -A.v.w;

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
float angle = Math_Float3AngleBetween (A, B);

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
float angle = Math_Float4AngleBetween( A, B );

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
*   Math_Float2HadamardProduct()
*
*   DESCRIPTION:
*       Calculate the component-wise produt of two vectors. 
*
*******************************************************************/

Float2 Math_Float2HadamardProduct( const Float2 a, const Float2 b )
{
Float2 ret = Math_Float2Make( a.v.x * b.v.x, a.v.y * b.v.y );

return( ret );

} /*  Math_Float2HadamardProduct() */


/*******************************************************************
*
*   Math_Float3HadamardProduct()
*
*   DESCRIPTION:
*       Calculate the component-wise produt of two vectors.
*
*******************************************************************/

Float3 Math_Float3HadamardProduct( const Float3 a, const Float3 b )
{
Float3 ret = Math_Float3Make( a.v.x * b.v.x, a.v.y * b.v.y, a.v.z * b.v.z );

return( ret );

} /*  Math_Float3HadamardProduct() */


/*******************************************************************
*
*   Math_Float4HadamardProduct()
*
*   DESCRIPTION:
*       Calculate the component-wise produt of two vectors.
*
*******************************************************************/

Float4 Math_Float4HadamardProduct( const Float4 a, const Float4 b )
{
Float4 ret = Math_Float4Make( a.v.x * b.v.x, a.v.y * b.v.y, a.v.z * b.v.z, a.v.w * b.v.w );

return( ret );

} /*  Math_Float4HadamardProduct() */


/*******************************************************************
*
*   Math_Float2TripleProduct()
*
*   DESCRIPTION:
*       Calculates the triple product for three vectors A x (B x C) 
*
*******************************************************************/

Float2 Math_Float2TripleProduct( const Float2 A, const Float2 B, const Float2 C )
{
float scalar1 = Math_Float2DotProduct( A, C );
float scalar2 = Math_Float2DotProduct( A, B );
Float2 modB = Math_Float2ScalarMultiply( B, scalar1 );
Float2 modC = Math_Float2ScalarMultiply( C, scalar2);

return ( Math_Float2Subtraction( modB, modC));

} /*  Math_Float2TripleProduct() */


/*******************************************************************
*
*   Math_Float3TripleProduct()
*
*   DESCRIPTION:
*       Calculates the triple product for three vectors A x (B x C)
*
*******************************************************************/

Float3 Math_Float3TripleProduct( const Float3 A, const Float3 B, const Float3 C )
{
float scalar1 = Math_Float3DotProduct( A, C );
float scalar2 = Math_Float3DotProduct( A, B );
Float3 modB = Math_Float3ScalarMultiply( B, scalar1 );
Float3 modC = Math_Float3ScalarMultiply( C, scalar2 );

return (Math_Float3Subtraction( modB, modC ));

} /*  Math_Float3TripleProduct() */


/*******************************************************************
*
*   Math_Float4TripleProduct()
*
*   DESCRIPTION:
*       Calculates the triple product for three vectors A x (B x C)
*
*******************************************************************/

Float4 Math_Float4TripleProduct( const Float4 A, const Float4 B, const Float4 C )
{
float scalar1 = Math_Float4DotProduct( A, C );
float scalar2 = Math_Float4DotProduct( A, B );
Float4 modB = Math_Float4ScalarMultiply( B, scalar1 );
Float4 modC = Math_Float4ScalarMultiply( C, scalar2 );

return (Math_Float4Subtraction( modB, modC ));

} /*  Math_Float4TripleProduct() */


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
*   Math_Float2AngleBetween()
*
*   DESCRIPTION:
*       Calculates the angle between the two vectors
*
*******************************************************************/

float Math_Float2AngleBetween( const Float2 A, const Float2 B )
{
float magA = Math_Float2Magnitude( A );
float magB = Math_Float2Magnitude( B );
float AdotB = Math_Float2DotProduct(  A, B );

return ((float) acos(AdotB/(magA * magB)) );

} /*  Math_Float2AngleBetween() */


/*******************************************************************
*
*   Math_Float3AngleBetween()
*
*   DESCRIPTION:
*       Calculates the angle between the two vectors
*
*******************************************************************/

float Math_Float3AngleBetween( const Float3 A, const Float3 B )
{
    float magA = Math_Float3Magnitude( A );
    float magB = Math_Float3Magnitude( B );
    float AdotB = Math_Float3DotProduct( A, B );

    return ((float)acos( AdotB / (magA * magB) ));

} /*  Math_Float3AngleBetween() */


/*******************************************************************
*
*   Math_Float4AngleBetween()
*
*   DESCRIPTION:
*       Calculates the angle between the two vectors
*
*******************************************************************/

float Math_Float4AngleBetween( const Float4 A, const Float4 B )
{
    float magA = Math_Float4Magnitude( A );
    float magB = Math_Float4Magnitude( B );
    float AdotB = Math_Float4DotProduct( A, B );

    return ((float) acos( AdotB / (magA * magB) ));

} /*  Math_Float4AngleBetween() */


/*******************************************************************
*
*   Math_Float2IsAngleObtuse()
*
*   DESCRIPTION:
*       is the Angle between two vectors > 90 degrees
*
*******************************************************************/

bool Math_Float2IsAngleObtuse( const Float2 A, const Float2 B )
{
    if( Math_Float2DotProduct( A, B ) < 0.0)
    {
    return true;
    }
    return false;

} /*  Math_Float2IsAngleObtuse() */


/*******************************************************************
*
*   Math_Float3IsAngleObtuse()
*
*   DESCRIPTION:
*       is the Angle between two vectors > 90 degrees
*
*******************************************************************/

bool Math_Float3IsAngleObtuse( const Float3 A, const Float3 B )
{
    if( Math_Float3DotProduct( A, B ) < 0.0 )
    {
        return true;
    }
    return false;

} /*  Math_Float3IsAngleObtuse() */


/*******************************************************************
*
*   Math_Float4IsAngleObtuse()
*
*   DESCRIPTION:
*       is the Angle between two vectors > 90 degrees
*
*******************************************************************/

bool Math_Float4IsAngleObtuse( const Float4 A, const Float4 B )
{
    if( Math_Float4DotProduct( A, B ) < 0.0 )
    {
        return true;
    }
    return false;

} /*  Math_Float4IsAngleObtuse() */


/*******************************************************************
*
*   Math_Float2IsAngleAcute()
*
*   DESCRIPTION:
*       is the Angle between two vectors < 90 degrees
*
*******************************************************************/

bool Math_Float2IsAngleAcute( const Float2 A, const Float2 B )
{
    if( Math_Float2DotProduct( A, B ) > 0.0 )
    {
        return true;
    }
    return false;

} /*  Math_Float2IsAngleAcute() */


/*******************************************************************
*
*   Math_Float3IsAngleAcute()
*
*   DESCRIPTION:
*       is the Angle between two vectors < 90 degrees
*
*******************************************************************/

bool Math_Float3IsAngleAcute( const Float3 A, const Float3 B )
{
    if( Math_Float3DotProduct( A, B ) > 0.0 )
    {
        return true;
    }
    return false;

} /*  Math_Float3IsAngleAcute() */


/*******************************************************************
*
*   Math_Float4IsAngleAcute()
*
*   DESCRIPTION:
*       is the Angle between two vectors < 90 degrees
*
*******************************************************************/

bool Math_Float4IsAngleAcute( const Float4 A, const Float4 B )
{
    if( Math_Float4DotProduct( A, B ) > 0.0 )
    {
        return true;
    }
    return false;

} /*  Math_Float4IsAngleAcute() */


/*******************************************************************
*
*   Math_Float2IsAnglePerpendicular()
*
*   DESCRIPTION:
*       Are the two vectors perpendicular?
*
*******************************************************************/

bool Math_Float2IsAnglePerpendicular( const Float2 A, const Float2 B )
{
    if( Math_Float2DotProduct( A, B ) == 0.0 )
    {
        return true;
    }
    return false;

} /*  Math_Float2IsAnglePerpendicular() */


/*******************************************************************
*
*   Math_Float3IsAnglePerpendicular()
*
*   DESCRIPTION:
*        Are the two vectors perpendicular?
*
*******************************************************************/

bool Math_Float3IsAnglePerpendicular( const Float3 A, const Float3 B )
{
    if( Math_Float3DotProduct( A, B ) == 0.0 )
    {
        return true;
    }
    return false;

} /*  Math_Float3IsAnglePerpendicular() */


/*******************************************************************
*
*   Math_Float4IsAnglePerpendicular()
*
*   DESCRIPTION:
*        Are the two vectors perpendicular?
*
*******************************************************************/

bool Math_Float4IsAnglePerpendicular( const Float4 A, const Float4 B )
{
    if( Math_Float4DotProduct( A, B ) == 0.0 )
    {
        return true;
    }
    return false;

} /*  Math_Float4IsAnglePerpendicular() */


/*******************************************************************
*
*   Math_Float2DistanceBetween()
*
*   DESCRIPTION:
*        Calculate the magnitude of the B - A vector
*
*******************************************************************/

float Math_Float2DistanceBetween( const Float2 A, const Float2 B )
{
    Float2 C = {};
    C = Math_Float2Subtraction( B, A );

    return ( Math_Float2Magnitude( C ));

} /*  Math_Float2DistanceBetween() */


/*******************************************************************
*
*   Math_Float3DistanceBetween()
*
*   DESCRIPTION:
*        Calculate the magnitude of the B - A vector
*
*******************************************************************/

float Math_Float3DistanceBetween( const Float3 A, const Float3 B )
{
    Float3 C = {};
    C = Math_Float3Subtraction( B, A );

    return (Math_Float3Magnitude( C ));

} /*  Math_Float3DistanceBetween() */


/*******************************************************************
*
*   Math_Float4DistanceBetween()
*
*   DESCRIPTION:
*        Calculate the magnitude of the B - A vector
*
*******************************************************************/

float Math_Float4DistanceBetween( const Float4 A, const Float4 B )
{
    Float4 C = {};
    C = Math_Float4Subtraction( B, A );

    return (Math_Float4Magnitude( C ));

} /*  Math_Float4DistanceBetween() */


/*******************************************************************
*
*   Math_Float2SquaredDistanceBetween()
*
*   DESCRIPTION:
*        Calculate the magnitude^2 of the B - A vector
*
*******************************************************************/

float Math_Float2SquaredDistanceBetween( const Float2 A, const Float2 B )
{
    Float2 C = {};
    C = Math_Float2Subtraction( B, A );

    return ( Math_Float2SquareMagnitude( C ));

} /*  Math_Float2SquaredDistanceBetween() */


/*******************************************************************
*
*   Math_Float3SquaredDistanceBetween()
*
*   DESCRIPTION:
*        Calculate the magnitude^2 of the B - A vector
*
*******************************************************************/

float Math_Float3SquaredDistanceBetween( const Float3 A, const Float3 B )
{
    Float3 C = {};
    C = Math_Float3Subtraction( B, A );

    return (Math_Float3SquareMagnitude( C ));

} /*  Math_Float3SquaredDistanceBetween() */


/*******************************************************************
*
*   Math_Float4SquaredDistanceBetween()
*
*   DESCRIPTION:
*        Calculate the magnitude^2 of the B - A vector
*
*******************************************************************/

float Math_Float4SquaredDistanceBetween( const Float4 A, const Float4 B )
{
    Float4 C = {};
    C = Math_Float4Subtraction( B, A );

    return (Math_Float4SquareMagnitude( C ));

} /*  Math_Float4SquaredDistanceBetween() */



/*******************************************************************
*
*   Math_Float2ProjectionofPointOntoLineSegment()
*
*   DESCRIPTION:
*       Projects a point onto a line segment AB and caculates an
*       output point on the AB line as well as the distance of the
*       output point from line segment point A.
*
*******************************************************************/
void Math_Float2ProjectionofPointOntoLineSegment( const Float2 line_segmentA, const Float2 line_segmentB, const Float2 test_point, Float2 *output_point, float *distance_from_A )
{
   // Float2 lineAB = Math_Float2Subtraction( line_segmentB, line_segmentA );
   // float magAB = Math_Float2Magnitude(lineAB);
   // lineAB = Math_Float2ScalarMultiply(lineAB,magAB);
   //
   // *distance_from_A = Math_Float2DotProduct( Math_Float2Subtraction( test_point, line_segmentA ), lineAB ) / magAB;
   // *output_point = Math_Float2Addition( line_segmentA, Math_Float2ScalarMultiply( Math_Float2Subtraction( line_segmentB, line_segmentA ), *distance_from_A ) );

   Float2 lineAB = Math_Float2Subtraction( line_segmentB, line_segmentA );
   Float2 test_point_vec = Math_Float2Subtraction( test_point, line_segmentA );
   *distance_from_A = Math_Float2DotProduct(lineAB, test_point_vec) / Math_Float2DotProduct(lineAB,lineAB);;
   *output_point = Math_Float2Addition(line_segmentA, Math_Float2ScalarMultiply(lineAB,*distance_from_A));


}/*Math_Float2ProjectionofPointOntoLineSegment() */


/*******************************************************************
*
*   Math_Float3ProjectionofPointOntoLineSegment()
*
*   DESCRIPTION:
*       Projects a point onto a line segment AB and caculates an
*       output point on the AB line as well as the distance of the
*       output point from line segment point A.
*
*******************************************************************/
void Math_Float3ProjectionofPointOntoLineSegment( const Float3 line_segmentA, const Float3 line_segmentB, const Float3 test_point, Float3 *output_point, float *distance_from_A )
{
    Float3 lineAB = Math_Float3Subtraction( line_segmentB, line_segmentA );
    *distance_from_A = Math_Float3DotProduct( Math_Float3Subtraction( test_point, line_segmentA ), lineAB ) ;
    *output_point = Math_Float3Addition( line_segmentA, Math_Float3ScalarMultiply( lineAB, *distance_from_A ) );

}/*Math_Float3ProjectionofPointOntoLineSegment() */


/*******************************************************************
*
*   Math_Float2LineSegmentCenterpoint()
*
*   DESCRIPTION:
*       Calculates the centerpoint, C,  of a line segment AB.
*
*******************************************************************/

Float2 Math_Float2LineSegmentCenterpoint( const Float2 A, const Float2 B )
{
    Float2 C = {};

    C.v.x = 0.5f * (A.v.x + B.v.x);
    C.v.y = 0.5f * (A.v.y + B.v.y);

    return C;
}/* Math_Float2LineSegmentCenterpoint()*/


/*******************************************************************
*
*   Math_Float3LineSegmentCenterpoint()
*
*   DESCRIPTION:
*       Calculates the centerpoint, C,  of a line segment AB.
*
*******************************************************************/

Float3 Math_Float3LineSegmentCenterpoint( const Float3 A, const Float3 B )
{
    Float3 C = {};

    C.v.x = 0.5f * (A.v.x + B.v.x);
    C.v.y = 0.5f * (A.v.y + B.v.y);
    C.v.z = 0.5f * (A.v.z + B.v.z);

    return C;
}/* Math_Float2LineSegmentCenterpoint()*/


/*******************************************************************
*
*   Math_Float2LineSegmentIntersection()
*
*   DESCRIPTION:
*       Calculates if 2 line segments (AB and CD) intersect and at what point.
*       
*
*******************************************************************/

bool Math_Float2LineSegmentIntersection( const Float2 A, const Float2 B, const Float2 C, const Float2 D, const float tolerance, Float2 *output_intersection_point )
{
    Float2 AB_vector = Math_Float2Subtraction(B,A);
    Float2 CD_vector = Math_Float2Subtraction(D,C);
    float ABxCD = Math_Float2PseudoCrossProduct(AB_vector,CD_vector);

    //check if the two lines are parrallel
    if( abs( ABxCD ) <= tolerance )
    {
        if( abs( Math_Float2PseudoCrossProduct( Math_Float2Subtraction( A, C ), AB_vector ) ) <= tolerance )
        {
            // the two lines are co-linear. Next we test if they overlap
            float normal_factor = Math_Float2DotProduct(AB_vector,AB_vector);
            float ref_interval[2] = {};
            float test_interval[2] = {};
            ref_interval[0] = tolerance;
            ref_interval[1] = normal_factor;

            if( Math_Float2DotProduct( CD_vector, AB_vector ) < 0.0f )
            {
                test_interval[1] = Math_Float2DotProduct( Math_Float2Subtraction( C, A ), AB_vector );
                test_interval[0] = test_interval[1] + Math_Float2DotProduct( CD_vector, AB_vector );
            }
            test_interval[0] = Math_Float2DotProduct(Math_Float2Subtraction(C,A), AB_vector);
            test_interval[1] = test_interval[0] + Math_Float2DotProduct(CD_vector,AB_vector);

            //overlap test
            if( test_interval[0] > ref_interval[1] )
            {
                return false;  //the two lines are co-linear but do not overlap
            }
            else if( test_interval[1] < ref_interval[0] )
            {
                return false; //the two lines are co-linear but do not overlap
            }
            
            if( test_interval[0] <= ref_interval[0] )
            {
                *output_intersection_point = A;
                return true;  // the two lines overlap over point A so return A as the intersection point.
            }
            else if( test_interval[0] <= ref_interval[1] )
            {
                *output_intersection_point = B;
                return true;  // the two lines overlap over point B so return B as the intersection point.
            }
        }
        return false;  // the two lines are parrallel and do not intersect
    }

    ABxCD = 1.0f / ABxCD;
    float intersection_position =  ABxCD * Math_Float2PseudoCrossProduct( Math_Float2Subtraction( C, A ), CD_vector );
    if( intersection_position < -0.0f )
    {
        return false;  //intersection point lins outside the line segment vector AB. < point A
    }
    else if( intersection_position > 1.0f )
    {
        return false;  //intersection point lins outside the line segment  vector AB. > point B
    }

    intersection_position =  ABxCD * Math_Float2PseudoCrossProduct( Math_Float2Subtraction( C, A ), AB_vector );
    if( intersection_position < -0.0f )
    {
        return false;  //intersection point lins outside the line segment vector CD. < point C
    }
    else if( intersection_position > 1.0f )
    {
        return false;  //intersection point lins outside the line segment vector CD. > point D
    }

    *output_intersection_point = Math_Float2Addition(C,Math_Float2ScalarMultiply(CD_vector, intersection_position ));
    return true;
    
}/* Math_Float2LineSegmentIntersection()*/


/*******************************************************************
*
*   Math_Float3LineSegmentIntersection()
*
*   DESCRIPTION:
*       Calculates if 2 line segments (AB and CD) intersect and at what point.
*       Note: will return false and not produce an intersection point if the two lines are co-linear.
*
*******************************************************************/

bool Math_Float3LineSegmentIntersection( const Float3 A, const Float3 B, const Float3 C, const Float3 D, const float tolerance, Float3 *output_intersection_point )
{
    Float3 AB_vector = Math_Float3Subtraction( B, A );
    Float3 CD_vector = Math_Float3Subtraction( D, C );
    Float3 ABxCD = Math_Float3CrossProduct( AB_vector, CD_vector );
    float sqmag_ABxCD = Math_Float3SquareMagnitude(ABxCD);
    Float3x3 intersection_matrix = {};
    float det = 0.0f;

    // check if the two lines are parallel
    if( sqmag_ABxCD < tolerance )
    {
        if( Math_Float3SquareMagnitude(Math_Float3CrossProduct( Math_Float3Subtraction( A, C ), AB_vector ) ) <= tolerance)
        {
            // the two lines are co-linear. Next we test if they overlap
            float normal_factor = Math_Float3DotProduct( AB_vector, AB_vector );
            float ref_interval[2] = {};
            float test_interval[2] = {};
            ref_interval[0] = tolerance;
            ref_interval[1] = normal_factor;

            if( Math_Float3DotProduct( CD_vector, AB_vector ) < 0.0f )
            {
                test_interval[1] = Math_Float3DotProduct( Math_Float3Subtraction( C, A ), AB_vector );
                test_interval[0] = test_interval[1] + Math_Float3DotProduct( CD_vector, AB_vector );
            }
            test_interval[0] = Math_Float3DotProduct( Math_Float3Subtraction( C, A ), AB_vector );
            test_interval[1] = test_interval[0] + Math_Float3DotProduct( CD_vector, AB_vector );

            //overlap test
            if( test_interval[0] > ref_interval[1] )
            {
                return false;  //the two lines are co-linear but do not overlap
            }
            else if( test_interval[1] < ref_interval[0] )
            {
                return false; //the two lines are co-linear but do not overlap
            }

            if( test_interval[0] <= ref_interval[0] )
            {
                *output_intersection_point = A;
                return true;  // the two lines overlap over point A so return A as the intersection point.
            }
            else if( test_interval[0] <= ref_interval[1] )
            {
                *output_intersection_point = B;
                return true;  // the two lines overlap over point B so return B as the intersection point.
            }
        }
        return false;  // the two lines are parrallel and do not intersect
    }

    Float3 intersection_line[2] = {};
    //point of closest approach on line AB
    Math_Float3x3MatrixFromVectors(Math_Float3Subtraction(C,A), CD_vector, ABxCD, &intersection_matrix);
    det = Math_Float3x3Determinate(&intersection_matrix);
    float normalization = 1.0f / sqmag_ABxCD;
    float intersection_position = det * normalization;

    if( intersection_position < -0.0f )
    {
        return false;  //intersection point lins outside the line segment vector AB. < point A
    }
    else if( intersection_position > 1.0f )
    {
        return false;  //intersection point lins outside the line segment  vector AB. > point B
    }

    intersection_line[0] = Math_Float3Addition(A, Math_Float3ScalarMultiply(AB_vector, intersection_position ));
    
    // find closest approach on line CD
    Math_Float3x3MatrixFromVectors( Math_Float3Subtraction( C, A ), AB_vector, ABxCD, &intersection_matrix );
    det = Math_Float3x3Determinate( &intersection_matrix );
    intersection_position = det * normalization;

    if( intersection_position < -0.0f )
    {
        return false;  //intersection point lins outside the line segment vector CD. < point C
    }
    else if( intersection_position > 1.0f )
    {
        return false;  //intersection point lins outside the line segment  vector CD. > point D
    }

    intersection_line[1] = Math_Float3Addition( C, Math_Float3ScalarMultiply( CD_vector, intersection_position ) );

    // calculate the midpoint of the intersection line to get the intersection point
    *output_intersection_point = Math_Float3LineSegmentCenterpoint(intersection_line[0], intersection_line[1]);

    return true;
}/* Math_Float3LineSegmentIntersection()*/