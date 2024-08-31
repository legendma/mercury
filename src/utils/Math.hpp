#pragma once
#include <climits>
#include <cstdint>


#define NUM_BITS_PER_BYTE \
    CHAR_BIT

typedef union _Float2
    {
    float f[ 2 ];
    struct
        {
        float           x;
        float           y;
        } v;
    } Float2;

typedef union _Float3
    {
    float f[ 3 ];
    struct
        {
        float           x;
        float           y;
        float           z;
        } v;
    } Float3;

typedef union _Float4
    {
    float f[ 4 ];
    struct
        {
        float           x;
        float           y;
        float           z;
        float           w;
        } v;
    } Float4;

typedef Float4 Quaternion;


typedef union _Float2x2
{
    float f[2][2];
    struct
    {
        float           _11;
        float           _12;
        float           _21;
        float           _22;
    } n;
} Float2x2;


typedef union _Float3x3
    {
    float f[ 3 ][ 3 ];
    struct
        {
        float           _11;
        float           _12;
        float           _13;
        float           _21;
        float           _22;
        float           _23;
        float           _31;
        float           _32;
        float           _33;
        } n;
    } Float3x3;

typedef union _Float4x4
    {
    float f[ 4 ][ 4 ];
    struct
        {
        float           _11;
        float           _12;
        float           _13;
        float           _14;
        float           _21;
        float           _22;
        float           _23;
        float           _24;
        float           _31;
        float           _32;
        float           _33;
        float           _34;
        float           _41;
        float           _42;
        float           _43;
        float           _44;
        } n;
    } Float4x4;

static const Quaternion QUATERNION_IDENTITY =
    {
    0.0f, 0.0f, 0.0f, 1.0f
    };

static const Float3x3 FLOAT3x3_IDENTITY =
    {
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f
    };

static const Float4x4 FLOAT4x4_IDENTITY =
    {
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f
    };

/**************************/
/* Vector Math Functions*/
/**************************/

/*******************************************************************
*
*   Math_Float2Make()
*
*******************************************************************/

static inline Float2 Math_Float2Make( const float x, const float y )
{
Float2 ret;
ret.v.x = x;
ret.v.y = y;

return( ret );

} /* Math_Float2Make() */


/*******************************************************************
*
*   Math_Float3Make()
*
*******************************************************************/

static inline Float3 Math_Float3Make( const float x, const float y, const float z )
{
Float3 ret;
ret.v.x = x;
ret.v.y = y;
ret.v.z = z;

return( ret );

} /* Math_Float3Make() */


/*******************************************************************
*
*   Math_Float4Make()
*
*******************************************************************/

static inline Float4 Math_Float4Make( const float x, const float y, const float z, const float w )
{
Float4 ret;
ret.v.x = x;
ret.v.y = y;
ret.v.z = z;
ret.v.w = w;

return( ret );

} /* Math_Float4Make() */


typedef struct _BoundingBoxAA2D
{
    Float2   center;
    Float2   half_extent;

}BoundingBoxAA2D;


typedef struct _BoundingBoxAA3D
{
    Float3   center;
    Float3   half_extent;

}BoundingBoxAA3D;

typedef struct _BoundingBoxSphere2D
{
    Float2   center;
    float    radius;

}BoundingBoxSphere2D;


typedef struct _BoundingBoxSphere3D
{
    Float3   center;
    float    radius;

}BoundingBoxSphere3D;

typedef struct _BoundingBoxCapsule2D
{
    Float2   point_A;
    Float2   point_B;
    float    radius;

}BoundingBoxCapsule2D;





/**************************/
/* Statistics Math Functions*/
/**************************/

float Math_floatMean( const float *array_of_floats, const int num_of_floats, const size_t stride );
float Math_floatStandardDeviation( const float *array_of_floats, const int num_of_floats, const float mean_value, const size_t stride );
void Math_Float2CoVarianceMatrix2x2( const Float2 *points, const uint32_t num_of_points, Float2x2 *output );
void Math_Float3CoVarianceMatrix3x3( const Float3 *points, const int num_of_points, Float3x3 *output );
void Math_Float2EigenDecomposition( const Float2 *array_of_points, const uint32_t number_of_points, const float tolerance, const uint32_t size_of_output_arrays, float *output_eigenvalues, Float2 *output_eigenvectors, Float2 *output_vector_origin );
bool Math_Float3EigenDecomposition( const Float3 *array_of_points, const uint32_t number_of_points, const float tolerance, const uint32_t max_iterations, const uint32_t size_of_output_arrays, float *output_eigenvalues, Float3 *output_eigenvectors, Float3 *output_vector_origin );


/**************************/
/* Vector Math Functions*/
/**************************/

Float2 Math_Float2Addition( const Float2 A, const Float2 B );
Float3 Math_Float3Addition( const Float3 A, const Float3 B );
Float4 Math_Float4Addition( const Float4 A, const Float4 B );

Float2 Math_Float2Subtraction( const Float2 A, const Float2 B );
Float3 Math_Float3Subtraction( const Float3 A, const Float3 B );
Float4 Math_Float4Subtraction( const Float4 A, const Float4 B );

Float2 Math_Float2ScalarMultiply( const Float2 A, const float scalar );
Float3 Math_Float3ScalarMultiply( const Float3 A, const float scalar );
Float4 Math_Float4ScalarMultiply( const Float4 A, const float scalar );

Float2 Math_Float2Negative( const Float2 A );
Float3 Math_Float3Negative( const Float3 A );
Float4 Math_Float4Negative( const Float4 A );

float Math_Float2DotProduct( const Float2 A, const Float2 B );
float Math_Float3DotProduct( const Float3 A, const Float3 B );
float Math_Float4DotProduct( const Float4 A, const Float4 B );


Float3 Math_Float3CrossProduct( const Float3 A, const Float3 B );
/* TODO Add 4D cross product function */

float Math_Float2PseudoCrossProduct( const Float2 A, const Float2 B );
float Math_Float3CrossProductMagnitude( const Float3 A, const Float3 B );
float Math_Float4CrossProductMagnitude( const Float4 A, const Float4 B );

float Math_Float3BoxProduct( const Float3 A, const Float3 B, const Float3 C );
/* TODO Add 4D box product */

Float2 Math_Float2HadamardProduct( const Float2 a, const Float2 b );
Float3 Math_Float3HadamardProduct( const Float3 a, const Float3 b );
Float4 Math_Float4HadamardProduct( const Float4 a, const Float4 b );

Float2 Math_Float2TripleProduct( const Float2 A, const Float2 B, const Float2 C );
Float3 Math_Float3TripleProduct( const Float3 A, const Float3 B, const Float3 C );
Float4 Math_Float4TripleProduct( const Float4 A, const Float4 B, const Float4 C );

float Math_Float2Magnitude( const Float2 A );
float Math_Float3Magnitude( const Float3 A );
float Math_Float4Magnitude( const Float4 A );

float Math_Float2SquareMagnitude( const Float2 A );
float Math_Float3SquareMagnitude( const Float3 A );
float Math_Float4SquareMagnitude( const Float4 A );

float Math_Float2AngleBetween( const Float2 A, const Float2 B );
float Math_Float3AngleBetween( const Float3 A, const Float3 B );
float Math_Float4AngleBetween( const Float4 A, const Float4 B );

bool Math_Float2IsAngleObtuse( const Float2 A, const Float2 B );
bool Math_Float3IsAngleObtuse( const Float3 A, const Float3 B );
bool Math_Float4IsAngleObtuse( const Float4 A, const Float4 B );

bool Math_Float2IsAngleAcute( const Float2 A, const Float2 B );
bool Math_Float3IsAngleAcute( const Float3 A, const Float3 B );
bool Math_Float4IsAngleAcute( const Float4 A, const Float4 B );

bool Math_Float2IsAnglePerpendicular( const Float2 A, const Float2 B );
bool Math_Float3IsAnglePerpendicular( const Float3 A, const Float3 B );
bool Math_Float4IsAnglePerpendicular( const Float4 A, const Float4 B );

float Math_Float2DistanceBetween( const Float2 A, const Float2 B );
float Math_Float3DistanceBetween( const Float3 A, const Float3 B );
float Math_Float4DistanceBetween( const Float4 A, const Float4 B );

float Math_Float2SquaredDistanceBetween( const Float2 A, const Float2 B );
float Math_Float3SquaredDistanceBetween( const Float3 A, const Float3 B );
float Math_Float4SquaredDistanceBetween( const Float4 A, const Float4 B );

void Math_Float2ProjectionofPointOntoLineSegment( const Float2 line_segmentA, const Float2 line_segmentB, const Float2 test_point, Float2 *output_point, float *distance_from_A );
void Math_Float3ProjectionofPointOntoLineSegment( const Float3 line_segmentA, const Float3 line_segmentB, const Float3 test_point, Float3 *output_point, float *distance_from_A );

Float2 Math_Float2LineSegmentCenterpoint( const Float2 A, const Float2 B );
Float3 Math_Float3LineSegmentCenterpoint( const Float3 A, const Float3 B );

bool Math_Float2LineSegmentIntersection( const Float2 A, const Float2 B, const Float2 C, const Float2 D, const float tolerance, Float2 *output_intersection_point );
bool Math_Float3LineSegmentIntersection( const Float3 A, const Float3 B, const Float3 C, const Float3 D, const float tolerance, Float3 *output_intersection_point );


/*****************************/
/* Quaternion Math Functions */
/*****************************/
void Math_QuaternionToFloat4x4( const Quaternion in, Float4x4 *out );


/*****************************/
/* Matrix Math Functions */
/*****************************/
void Math_Float3x3MakeScaleFromFloat2( const Float2 scale, Float3x3 *out );
void Math_Float4x4MakeScaleFromFloat3( const Float3 scale, Float4x4 *out );

void Math_Float3x3MakeTranslateFromFloat2( const Float2 translation, Float3x3 *out );
void Math_Float4x4MakeTranslateFromFloat3( const Float3 translation, Float4x4 *out );

void Math_Float3x3MakeRotation( const float theta, Float3x3 *out );

void Math_Float3x3MultiplyByFloat3x3( const Float3x3 *a, const Float3x3 *b, Float3x3 *out );
void Math_Float4x4MultiplyByFloat4x4( const Float4x4 *a, const Float4x4 *b, Float4x4 *out );

void Math_Float3x3TransformSpin( const Float2 translation, const float rotation, const Float2 scale, Float3x3 *out );
void Math_Float4x4TransformSpin( const Float3 translation, const Quaternion rotation, const Float3 scale, Float4x4 *out );
void Math_Float2x2ScalebyFloat( const Float2x2 *input_matrix, const float scale, Float2x2 *output_matrix );
void Math_Float3x3ScalebyFloat( const Float3x3 *input_matrix, const float scale, Float3x3 *output_matrix );
void Math_Float4x4ScalebyFloat( const Float4x4 *input_matrix, const float scale, Float4x4 *output_matrix );

float Math_Float2x2Determinate( const Float2x2 *matrix );
float Math_Float3x3Determinate( const Float3x3 *matrix );

void Math_Float2x2Transpose( const Float2x2 *input_matrix, Float2x2 *output_matrix );
void Math_Float3x3Transpose( const Float3x3 *input_matrix, Float3x3 *output_matrix );

void Math_Float2x2Inverse( const Float2x2 *input_matrix, Float2x2 *output_matrix );
void Math_Float3x3Inverse( const Float3x3 *input_matrix, Float3x3 *output_matrix );

void Math_Float3x3MultiplyByFloat3x3( const Float3x3 *a, const Float3x3 *b, Float3x3 *out );
Float2 Math_Float2x2MultiplyByFloat2( const Float2x2 *A, const Float2 *x );
Float3 Math_Float3x3MultiplyByFloat3( const Float3x3 *A, const Float3 *x );

void Math_Float2x2MatrixFromVectors( const Float2 vector_A, const Float2 vector_B, Float2x2 *output );
void Math_Float3x3MatrixFromVectors( const Float3 vector_A, const Float3 vector_B, const Float3 vector_C, Float3x3 *output );

void Math_Float2x2EigenValuesVectors( const Float2x2 *input_matrix, const float tolerance, const uint32_t output_capacity, float *output_eigenvalues, Float2 *output_eigenvectors );
bool Math_Float3x3EigenValuesVectors( const Float3x3 *input_matrix, const uint32_t max_iterations, const float tolerance, const uint32_t output_capacity, float *output_eigenvalues, Float3 *output_eigenvectors );

/*************************/
/* Bounding Box Functions */
/*************************/

void Math_GenerateAxisAlignedBoundingBox2D(const Float2 *vertices, const int num_of_vertices, BoundingBoxAA2D *bounding_box );
void Math_GenerateAxisAlignedBoundingBox3D( const Float3 *vertices, const int num_of_vertices, BoundingBoxAA3D *bounding_box );
void Math_GenerateBoundingBoxSpherefromAABB2D( const BoundingBoxAA2D *AABB_bounding_box, BoundingBoxSphere2D *bounding_sphere );
void Math_GenerateBoundingBoxSpherefromAABB3D( const BoundingBoxAA3D *AABB_bounding_box, BoundingBoxSphere3D *bounding_sphere );
void Math_GenerateCapsuleBoundingBox2D( const Float2 *vertices, const uint32_t num_of_vertices, const float tolerance, BoundingBoxCapsule2D *bounding_box );

extern Float2 test_vertices[ 20 ];
/*************************/
/*  Bit Array Functions  */
/*************************/


#define NUM_BITS_PER_BYTE \
    CHAR_BIT

/*******************************************************************
*
*   MATH_BITARRAY_COUNT()
*
*   DESCRIPTION:
*       Get the array count needed to store the given number of
*       bits, with the given type.
*
*******************************************************************/

#define MATH_BITARRAY_COUNT( _type, _bit_count ) \
    ( 1 + ( ( (_bit_count) - 1 ) / ( NUM_BITS_PER_BYTE * sizeof( _type ) ) ) )


/*******************************************************************
*
*   math_bitarray_bit()
*
*   DESCRIPTION:
*       Get the bit mask for the given bit and bit array.
*
*******************************************************************/

#define math_bitarray_bit( _bit_array, _bit ) \
    ( 1 << ( _bit % ( NUM_BITS_PER_BYTE * sizeof( *(_bit_array) ) ) ) )


/*******************************************************************
*
*   math_bitarray_index()
*
*   DESCRIPTION:
*       Get the array index for the given bit.
*
*******************************************************************/

#define math_bitarray_index( _bit_array, _bit ) \
    ( _bit / ( NUM_BITS_PER_BYTE * sizeof( *(_bit_array) ) ) )


/*******************************************************************
*
*   Math_BitArrayClear()
*
*   DESCRIPTION:
*       Clear the requested bit on the bit array.
*
*******************************************************************/

#define Math_BitArrayClear( _bit_array, _bit ) \
    (_bit_array)[ math_bitarray_index( _bit_array, _bit ) ] &= ~math_bitarray_bit( _bit_array, _bit )


/*******************************************************************
*
*   Math_BitArraySet()
*
*   DESCRIPTION:
*       Set the requested bit on the bit array.
*
*******************************************************************/

#define Math_BitArraySet( _bit_array, _bit ) \
    (_bit_array)[ math_bitarray_index( _bit_array, _bit ) ] |= math_bitarray_bit( _bit_array, _bit )


/*******************************************************************
*
*   Math_BitArrayIsSet()
*
*   DESCRIPTION:
*       Test if the requested bit is set on the array.
*
*******************************************************************/

#define Math_BitArrayIsSet( _bit_array, _bit ) \
    ( ( (_bit_array)[ math_bitarray_index( _bit_array, _bit ) ] & math_bitarray_bit( _bit_array, _bit ) ) != 0 )


/*******************************************************************
*
*   Utilities_ClampToMaxU32()
*
*   DESCRIPTION:
*       Return the value clamped to the maximum.
*
*******************************************************************/

static inline uint32_t Utilities_ClampToMaxU32( const uint32_t value, const uint32_t maximum )
{
return( value > maximum ? maximum : value );

} /* Utilities_ClampToMaxU32() */


/*******************************************************************
*
*   Utilities_ClampToMinU32()
*
*   DESCRIPTION:
*       Return the value clamped to the minimum.
*
*******************************************************************/

static inline uint32_t Utilities_ClampToMinU32( const uint32_t value, const uint32_t minimum )
{
return( value < minimum ? minimum : value );

} /* Utilities_ClampToMinU32() */