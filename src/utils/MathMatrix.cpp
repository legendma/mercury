#include <cmath>
#include <cstdio>

#include "Math.hpp"
#include "Utilities.hpp"

void Math_Float2x2Eigenvalues( const Float2x2 *input_matrix, const float tolerance, const uint32_t output_capacity, float *output );
Float2 Math_Float2x2EigenVector( const Float2x2 *input_matrix, const float tolerance, const float eigenvalue );
void Math_Float3x3TransformToSymmetricTridiagonalMatrix( const Float3x3 *input_matrix, Float3x3 *output_matrix );
bool Math_Float3x3DominateEigenValueVector( const Float3x3 *input_matrix, const uint32_t max_iterations, const float tolerance, float *output_eigenvalue, Float3 *output_eigenvector );
void Math_Float3x3NextDominateEigenValueVector( const Float3x3 *input_matrix, const float tolerance, const float dominate_eigenvalue, const Float3 dominate_eigenvector, float *output_eigenvalue, Float3 *output_eigenvector );



/*******************************************************************
*
*   Math_Float3x3MakeScaleFromFloat2()
*
*   DESCRIPTION:
*       Create a scaling matrix from a Float2.
*
*******************************************************************/

void Math_Float3x3MakeScaleFromFloat2( const Float2 scale, Float3x3 *out )
{
clr_struct( out );

out->n._11 = scale.v.x;
out->n._22 = scale.v.y;
out->n._33 = 1.0f;

} /*  Math_Float3x3MakeScaleFromFloat2() */


/*******************************************************************
*
*   Math_Float4x4MakeScaleFromFloat3()
*
*   DESCRIPTION:
*       Create a scaling matrix from a Float3.
*
*******************************************************************/

void Math_Float4x4MakeScaleFromFloat3( const Float3 scale, Float4x4 *out )
{
clr_struct( out );

out->n._11 = scale.v.x;
out->n._22 = scale.v.y;
out->n._33 = scale.v.z;
out->n._44 = 1.0f;

} /*  Math_Float4x4MakeScaleFromFloat3() */


/*******************************************************************
*
*   Math_Float3x3MakeRotation()
*
*   DESCRIPTION:
*       Create a rotation matrix from a radian value.
*
*******************************************************************/

void Math_Float3x3MakeRotation( const float theta, Float3x3 *out )
{
clr_struct( out );
float sine   = sinf( theta );
float cosine = cosf( theta );

out->n._11 = cosine;
out->n._12 = -sine;
out->n._21 = sine;
out->n._21 = cosine;
out->n._33 = 1.0f;

} /*  Math_Float3x3MakeRotation() */


/*******************************************************************
*
*   Math_Float3x3MakeTranslateFromFloat2()
*
*   DESCRIPTION:
*       Create a translation matrix from a Float2.
*
*******************************************************************/

void Math_Float3x3MakeTranslateFromFloat2( const Float2 translation, Float3x3 *out )
{
clr_struct( out );

out->n._13 = translation.v.x;
out->n._23 = translation.v.y;
out->n._33 = 1.0f;

} /*  Math_Float3x3MakeTranslateFromFloat2() */


/*******************************************************************
*
*   Math_Float4x4MakeTranslateFromFloat3()
*
*   DESCRIPTION:
*       Create a translation matrix from a Float3.
*
*******************************************************************/

void Math_Float4x4MakeTranslateFromFloat3( const Float3 translation, Float4x4 *out )
{
clr_struct( out );

out->n._14 = translation.v.x;
out->n._24 = translation.v.y;
out->n._34 = translation.v.z;
out->n._44 = 1.0f;

} /*  Math_Float4x4MakeTranslateFromFloat3() */


/*******************************************************************
*
*   Math_Float3x3MultiplyByFloat3x3()
*
*   DESCRIPTION:
*        Calculate C = A x B
*
*******************************************************************/

void Math_Float3x3MultiplyByFloat3x3( const Float3x3 *a, const Float3x3 *b, Float3x3 *out )
{
Float3x3 temp;
temp.n._11 =   a->n._11 * b->n._11 + a->n._12 * b->n._12 + a->n._13 * b->n._13;
temp.n._12 =   a->n._11 * b->n._21 + a->n._12 * b->n._22 + a->n._13 * b->n._23;
temp.n._13 =   a->n._11 * b->n._31 + a->n._12 * b->n._32 + a->n._13 * b->n._33;

temp.n._21 =   a->n._21 * b->n._11 + a->n._22 * b->n._12 + a->n._23 * b->n._13;
temp.n._22 =   a->n._21 * b->n._21 + a->n._22 * b->n._22 + a->n._23 * b->n._23;
temp.n._23 =   a->n._21 * b->n._31 + a->n._22 * b->n._32 + a->n._23 * b->n._33;

temp.n._31 =   a->n._31 * b->n._11 + a->n._32 * b->n._12 + a->n._33 * b->n._13;
temp.n._32 =   a->n._31 * b->n._21 + a->n._32 * b->n._22 + a->n._33 * b->n._23;
temp.n._33 =   a->n._31 * b->n._31 + a->n._32 * b->n._32 + a->n._33 * b->n._33;

*out = temp;

} /*  Math_Float3x3MultiplyByFloat3x3() */


/*******************************************************************
*
*   Math_Float2x2MultiplyByFloat2()
*
*   DESCRIPTION:
*        Calculate y = A * x
*
*******************************************************************/

Float2 Math_Float2x2MultiplyByFloat2( const Float2x2 *A, const Float2 *x )
{
    Float2 output = {};
    output.v.x = A->n._11 * x->v.x + A->n._12 * x->v.y;
    output.v.y = A->n._21 * x->v.x + A->n._22 * x->v.y;

    return output;

} /*   Math_Float2x2MultiplyByFloat2() */


/*******************************************************************
*
*   Math_Float3x3MultiplyByFloat3()
*
*   DESCRIPTION:
*        Calculate y = A * x
*
*******************************************************************/

Float3 Math_Float3x3MultiplyByFloat3( const Float3x3 *A, const Float3 *x )
{
    Float3 output = {};
    output.v.x = A->n._11 * x->v.x + A->n._12 * x->v.y + A->n._13 * x->v.z;
    output.v.y = A->n._21 * x->v.x + A->n._22 * x->v.y + A->n._23 * x->v.z;
    output.v.z = A->n._31 * x->v.x + A->n._32 * x->v.y + A->n._33 * x->v.z;

    return output;

} /*   Math_Float3x3MultiplyByFloat3() */


/*******************************************************************
*
*   Math_Float4x4MultiplyByFloat4x4()
*
*   DESCRIPTION:
*        Calculate C = A x B
*
*******************************************************************/

void Math_Float4x4MultiplyByFloat4x4( const Float4x4 *a, const Float4x4 *b, Float4x4 *out )
{
Float4x4 temp;
temp.n._11 =   a->n._11 * b->n._11 + a->n._12 * b->n._12 + a->n._13 * b->n._13 + a->n._14 * b->n._14;
temp.n._12 =   a->n._11 * b->n._21 + a->n._12 * b->n._22 + a->n._13 * b->n._23 + a->n._14 * b->n._24;
temp.n._13 =   a->n._11 * b->n._31 + a->n._12 * b->n._32 + a->n._13 * b->n._33 + a->n._14 * b->n._34;
temp.n._14 =   a->n._11 * b->n._41 + a->n._12 * b->n._42 + a->n._13 * b->n._43 + a->n._14 * b->n._44;

temp.n._21 =   a->n._21 * b->n._11 + a->n._22 * b->n._12 + a->n._23 * b->n._13 + a->n._24 * b->n._14;
temp.n._22 =   a->n._21 * b->n._21 + a->n._22 * b->n._22 + a->n._23 * b->n._23 + a->n._24 * b->n._24;
temp.n._23 =   a->n._21 * b->n._31 + a->n._22 * b->n._32 + a->n._23 * b->n._33 + a->n._24 * b->n._34;
temp.n._24 =   a->n._21 * b->n._41 + a->n._22 * b->n._42 + a->n._23 * b->n._43 + a->n._24 * b->n._44;

temp.n._31 =   a->n._31 * b->n._11 + a->n._32 * b->n._12 + a->n._33 * b->n._13 + a->n._34 * b->n._14;
temp.n._32 =   a->n._31 * b->n._21 + a->n._32 * b->n._22 + a->n._33 * b->n._23 + a->n._34 * b->n._24;
temp.n._33 =   a->n._31 * b->n._31 + a->n._32 * b->n._32 + a->n._33 * b->n._33 + a->n._34 * b->n._34;
temp.n._34 =   a->n._31 * b->n._41 + a->n._32 * b->n._42 + a->n._33 * b->n._43 + a->n._34 * b->n._44;

temp.n._41 =   a->n._41 * b->n._11 + a->n._42 * b->n._12 + a->n._43 * b->n._13 + a->n._44 * b->n._14;
temp.n._42 =   a->n._41 * b->n._21 + a->n._42 * b->n._22 + a->n._43 * b->n._23 + a->n._44 * b->n._24;
temp.n._43 =   a->n._41 * b->n._31 + a->n._42 * b->n._32 + a->n._43 * b->n._33 + a->n._44 * b->n._34;
temp.n._44 =   a->n._41 * b->n._41 + a->n._42 * b->n._42 + a->n._43 * b->n._43 + a->n._44 * b->n._44;

*out = temp;

} /*  Math_Float4x4MultiplyByFloat4x4() */


/*******************************************************************
*
*   Math_Float3x3TransformSpin()
*
*   DESCRIPTION:
*       Calculate M = S x R x T
*
*******************************************************************/

void Math_Float3x3TransformSpin( const Float2 translation, const float rotation, const Float2 scale, Float3x3 *out )
{
/* load rotation */
Math_Float3x3MakeRotation( rotation, out );

/* scale */
out->n._11 *= scale.v.x;
out->n._21 *= scale.v.x;

out->n._12 *= scale.v.y;
out->n._22 *= scale.v.y;

/* translate */
out->n._13 = translation.v.x;
out->n._23 = translation.v.y;

} /*  Math_Float3x3TransformSpin() */


/*******************************************************************
*
*   Math_Float4x4TransformSpin()
*
*   DESCRIPTION:
*       Calculate M = S x R x T
*
*******************************************************************/

void Math_Float4x4TransformSpin( const Float3 translation, const Quaternion rotation, const Float3 scale, Float4x4 *out )
{
/* load rotation */
Math_QuaternionToFloat4x4( rotation, out );

/* scale */
out->n._11 *= scale.v.x;
out->n._21 *= scale.v.x;
out->n._31 *= scale.v.x;

out->n._12 *= scale.v.y;
out->n._22 *= scale.v.y;
out->n._32 *= scale.v.y;

out->n._13 *= scale.v.z;
out->n._23 *= scale.v.z;
out->n._33 *= scale.v.z;

/* translate */
out->n._14 = translation.v.x;
out->n._24 = translation.v.y;
out->n._34 = translation.v.z;

} /*  Math_Float4x4TransformSpin() */


/*******************************************************************
*
*   Math_Float2x2MatrixFromVectors()
*
*   DESCRIPTION:
*       Makes a Matrix from two Float2 column vectors
*
*******************************************************************/

void Math_Float2x2MatrixFromVectors( const Float2 vector_A,const Float2 vector_B, Float2x2 *output )
{
    output->n._11 = vector_A.v.x;
    output->n._21 = vector_A.v.y;

    output->n._12 = vector_B.v.x;
    output->n._22 = vector_B.v.y;

} /*  Math_Float2x2MatrixFromVectors() */



/*******************************************************************
*
*   Math_Float3x3MatrixFromVectors()
*
*   DESCRIPTION:
*       Makes a Matrix from three Float3 column vectors
*
*******************************************************************/

void Math_Float3x3MatrixFromVectors( const Float3 vector_A, const Float3 vector_B, const Float3 vector_C, Float3x3 *output )
{
    output->n._11 = vector_A.v.x;
    output->n._21 = vector_A.v.y;
    output->n._31 = vector_A.v.z;

    output->n._12 = vector_B.v.x;
    output->n._22 = vector_B.v.y;
    output->n._32 = vector_B.v.z;

    output->n._13 = vector_C.v.x;
    output->n._23 = vector_C.v.y;
    output->n._33 = vector_C.v.z;

} /*  Math_Float3x3MatrixFromVectors() */


/*******************************************************************
*
*   Math_Float2x2Determinate()
*
*   DESCRIPTION:
*       Calculates the determinate of a 2x2 matrix
*
*******************************************************************/

float Math_Float2x2Determinate( const Float2x2 *matrix )
{
    
  return (matrix->n._11 * matrix->n._22 - matrix->n._12 * matrix->n._21);


} /*  Math_Float2x2Determinate() */


/*******************************************************************
*
*   Math_Float3x3Determinate()
*
*   DESCRIPTION:
*       Calculates the determinate of a 3x3 matrix
*
*******************************************************************/

float Math_Float3x3Determinate( const Float3x3 *matrix )
{

// seperating into 2x2 determinates using the Lapace expansion method
    Float2x2 dummy_matrix = {};

    dummy_matrix.n._11 = matrix->n._22;
    dummy_matrix.n._12 = matrix->n._23;
    dummy_matrix.n._21 = matrix->n._32;
    dummy_matrix.n._22 = matrix->n._33;

    float detA11 = Math_Float2x2Determinate(&dummy_matrix );

    dummy_matrix.n._11 = matrix->n._21;
    dummy_matrix.n._12 = matrix->n._23;
    dummy_matrix.n._21 = matrix->n._31;
    dummy_matrix.n._22 = matrix->n._33;

    float detA12 = Math_Float2x2Determinate( &dummy_matrix );

    dummy_matrix.n._11 = matrix->n._21;
    dummy_matrix.n._12 = matrix->n._22;
    dummy_matrix.n._21 = matrix->n._31;
    dummy_matrix.n._22 = matrix->n._32;

    float detA13 = Math_Float2x2Determinate( &dummy_matrix );

    return (matrix->n._11 * detA11 - matrix->n._12 * detA12 + matrix->n._13 * detA13 );


} /*  Math_Float3x3Determinate() */


/*******************************************************************
*
*   Math_Float2x2Transpose()
*
*   DESCRIPTION:
*       Transposes a 2x2 matrix
*
*******************************************************************/

void Math_Float2x2Transpose( const Float2x2 *input_matrix, Float2x2 *output_matrix )
{
    output_matrix->n._11 = input_matrix->n._11;
    output_matrix->n._12 = input_matrix->n._21;
    output_matrix->n._21 = input_matrix->n._12;
    output_matrix->n._22 = input_matrix->n._22;
 

} /*  Math_Float2x2Transpose() */


/*******************************************************************
*
*   Math_Float3x3Transpose()
*
*   DESCRIPTION:
*       Transposes a 3x3 matrix
*
*******************************************************************/

void Math_Float3x3Transpose( const Float3x3 *input_matrix, Float3x3 *output_matrix )
{
    output_matrix->n._11 = input_matrix->n._11;
    output_matrix->n._12 = input_matrix->n._21;
    output_matrix->n._13 = input_matrix->n._31;
    output_matrix->n._21 = input_matrix->n._12;
    output_matrix->n._22 = input_matrix->n._22;
    output_matrix->n._23 = input_matrix->n._32;
    output_matrix->n._31 = input_matrix->n._13;
    output_matrix->n._32 = input_matrix->n._23;
    output_matrix->n._33 = input_matrix->n._33;

} /*  Math_Float3x3Transpose() */


/*******************************************************************
*
*   Math_Float2x2ScalebyFloat()
*
*   DESCRIPTION:
*       Multiplys a 2x2 matrix by a float
*
*******************************************************************/

void Math_Float2x2ScalebyFloat( const Float2x2 *input_matrix, const float scale, Float2x2 *output_matrix )
{
    output_matrix->n._11 = scale * input_matrix->n._11;
    output_matrix->n._12 = scale * input_matrix->n._12;
    output_matrix->n._21 = scale * input_matrix->n._21;
    output_matrix->n._22 = scale * input_matrix->n._22;


} /*  Math_Float2x2ScalbyFloat() */


/*******************************************************************
*
*   Math_Float3x3ScalebyFloat()
*
*   DESCRIPTION:
*       Multiplys a 3x3 matrix by a float
*
*******************************************************************/

void Math_Float3x3ScalebyFloat( const Float3x3 *input_matrix, const float scale, Float3x3 *output_matrix )
{
    output_matrix->n._11 = scale * input_matrix->n._11;
    output_matrix->n._12 = scale * input_matrix->n._12;
    output_matrix->n._13 = scale * input_matrix->n._13;
    output_matrix->n._21 = scale * input_matrix->n._21;
    output_matrix->n._22 = scale * input_matrix->n._22;
    output_matrix->n._23 = scale * input_matrix->n._23;
    output_matrix->n._31 = scale * input_matrix->n._31;
    output_matrix->n._32 = scale * input_matrix->n._32;
    output_matrix->n._33 = scale * input_matrix->n._33;

} /*  Math_Float3x3ScalebyFloat() */


/*******************************************************************
*
*   Math_Float4x4ScalebyFloat()
*
*   DESCRIPTION:
*       Multiplys a 4x4 matrix by a float
*
*******************************************************************/

void Math_Float4x4ScalebyFloat( const Float4x4 *input_matrix, const float scale, Float4x4 *output_matrix )
{
    output_matrix->n._11 = scale * input_matrix->n._11;
    output_matrix->n._12 = scale * input_matrix->n._12;
    output_matrix->n._13 = scale * input_matrix->n._13;
    output_matrix->n._14 = scale * input_matrix->n._14;
    output_matrix->n._21 = scale * input_matrix->n._21;
    output_matrix->n._22 = scale * input_matrix->n._22;
    output_matrix->n._23 = scale * input_matrix->n._23;
    output_matrix->n._24 = scale * input_matrix->n._24;
    output_matrix->n._31 = scale * input_matrix->n._31;
    output_matrix->n._32 = scale * input_matrix->n._32;
    output_matrix->n._33 = scale * input_matrix->n._33;
    output_matrix->n._34 = scale * input_matrix->n._34;
    output_matrix->n._41 = scale * input_matrix->n._41;
    output_matrix->n._42 = scale * input_matrix->n._42;
    output_matrix->n._43 = scale * input_matrix->n._43;
    output_matrix->n._44 = scale * input_matrix->n._44;

} /*  Math_Float4x4ScalebyFloat() */


/*******************************************************************
*
*   Math_Float2x2Inverse()
*
*   DESCRIPTION:
*       calculates the inverse matrix of a 2x2 matrix
*
*******************************************************************/

void Math_Float2x2Inverse( const Float2x2 *input_matrix, Float2x2 *output_matrix )
{
    float OneOverDet = 1 / Math_Float2x2Determinate(input_matrix);
    Float2x2 transpose = {};
    Math_Float2x2Transpose(input_matrix, &transpose);

    /* calcualte the inverse */
    Math_Float2x2ScalebyFloat(&transpose, OneOverDet,output_matrix );

} /*  Math_Float2x2Inverse() */


/*******************************************************************
*
*   Math_Float3x3Inverse()
*
*   DESCRIPTION:
*       calculates the inverse matrix of a 3x3 matrix
*
*******************************************************************/

void Math_Float3x3Inverse( const Float3x3 *input_matrix, Float3x3 *output_matrix )
{
    float OneOverDet = 1 / Math_Float3x3Determinate( input_matrix );
    Float3x3 transpose = {};
    Math_Float3x3Transpose( input_matrix, &transpose );

    /* calcualte the inverse */
    Math_Float3x3ScalebyFloat( &transpose, OneOverDet, output_matrix );

} /*  Math_Float3x3Inverse() */


/*******************************************************************
*
*   Math_Float2x2Eigenvalues()
*
*   DESCRIPTION:
*       calculates the eigenvalues of a 2x2 matrix. 
*       output is an array of floats with the eigenvalues
*       
*
*******************************************************************/

static void Math_Float2x2Eigenvalues(const Float2x2 *input_matrix,const float tolerance, const uint32_t output_capacity, float* output)
{
    debug_assert( output_capacity >= 2) ;
    
    float b = input_matrix->n._11 + input_matrix->n._22;
    float ac = -4.0f * (input_matrix->n._11 * input_matrix->n._22 - input_matrix->n._12 * input_matrix->n._21); //-4*ac
    float dummy = b * b + ac;
    if( abs( dummy ) < tolerance )
    {
        dummy = 0.0f;
    }
    debug_assert(dummy >= 0.0f);

    output[0] = 0.5f * (b + (float)sqrt( dummy ));
    output[1] = 0.5f * (b - (float)sqrt( dummy ));


}/* Math_Float2x2Eigenvalues()*/


/*******************************************************************
*
*   Math_Float2x2EigenVector()
*
*   DESCRIPTION:
*       calculates the normalized Eigen vector of a 2x2 matrix given an eigenvalue.
*
*
*******************************************************************/

static Float2 Math_Float2x2EigenVector( const Float2x2 *input_matrix,const float tolerance, const float eigenvalue )
{
    Float2 output = {};

  if( abs(input_matrix->n._12) > tolerance )
  {
      output.v.x = 1.0f;
      output.v.y = (eigenvalue - input_matrix->n._11) / input_matrix->n._12;
  }
  else if( abs(input_matrix->n._21) > tolerance )
  {
      output.v.x = (eigenvalue - input_matrix->n._22) / input_matrix->n._21;
      output.v.y = 1.0f;
  }
  else if( (abs(input_matrix->n._22 - eigenvalue)) > tolerance )
  {
      output.v.x = 1.0f;
      output.v.y = -input_matrix->n._21 / (input_matrix->n._22 - eigenvalue);
  }
  else if( (abs(input_matrix->n._11 - eigenvalue)) > tolerance )
  {
      output.v.x = -input_matrix->n._12 / (input_matrix->n._11 - eigenvalue);
      output.v.y = 1.0f;
  }
  else
  {
      output.v.x = 1.0f;
      output.v.y = 0.0f;
  }

  for( uint32_t i = 0; i < 2; i++ )
  {
      if( abs( output.f[i] ) < tolerance )
      {
          output.f[i] = 0.0f;
      }
  }
    
    float normalization = 1 / Math_Float2Magnitude( output );
    output.v.x = output.v.x * normalization;
    output.v.y = output.v.y * normalization;

    return output;

}/* Math_Float2x2Eigenvalues()*/



/*******************************************************************
*
*   Math_Float2x2EigenValuesVectors()
*
*   DESCRIPTION:
*       calculates the Eigen values and noramlized eigens vector of a 2x2 matrix
*
*
*******************************************************************/

void Math_Float2x2EigenValuesVectors( const Float2x2 *input_matrix,const float tolerance, const uint32_t output_capacity, float *output_eigenvalues, Float2 *output_eigenvectors )
{
    debug_assert( output_capacity >= 2 );

    Math_Float2x2Eigenvalues(input_matrix,tolerance,output_capacity,output_eigenvalues);
    for( uint32_t i = 0; i < 2; i++ )
    {
        output_eigenvectors[i] = Math_Float2x2EigenVector(input_matrix,tolerance, output_eigenvalues[i]);
    }

}/* Math_Float2x2EigenValuesVectors()*/



/*******************************************************************
*
*   Math_Float3x3TransformToSymmetricTridiagonalMatrix()
*
*   DESCRIPTION:
*       transforms a symmetric 3X3 matrix into a Tridiagonal symmetric 3x3 matrix
*
*******************************************************************/

static void Math_Float3x3TransformToSymmetricTridiagonalMatrix( const Float3x3 *input_matrix, Float3x3 *output_matrix )
{

    float alpha = 1.0f;
    if( input_matrix->n._21 == 0 )
    {
        alpha = -input_matrix->n._31;
    }

    alpha = (-(float)sqrt( input_matrix->n._21 * input_matrix->n._21 + input_matrix->n._31 * input_matrix->n._31 ) * input_matrix->n._21) / abs( input_matrix->n._21 );

    float RSQ = alpha * alpha - alpha * input_matrix->n._21;

    Float3 v = {};
    Float3 z = {};
    v.v.x = 0.0;
    v.v.y = input_matrix->n._21 - alpha;
    v.v.z = input_matrix->n._31;

    {
        Float3 u = {};
        u.v.x = ((input_matrix->n._12 * v.v.y + input_matrix->n._13 * v.v.z) / RSQ);
        u.v.y = ((input_matrix->n._22 * v.v.y + input_matrix->n._23 * v.v.z) / RSQ);
        u.v.z = ((input_matrix->n._32 * v.v.y + input_matrix->n._33 * v.v.z) / RSQ);

        float dummy = (v.v.y * u.v.y + v.v.z * u.v.z) / (2 * RSQ);
        z.v.x = u.v.x;
        z.v.y = u.v.y - dummy * v.v.y;
        z.v.z = u.v.z - dummy * v.v.z;
    }

    output_matrix->n._11 = input_matrix->n._11;
    output_matrix->n._32 = input_matrix->n._32 - v.v.y * z.v.z - v.v.z * z.v.y;
    output_matrix->n._23 = output_matrix->n._32;
    output_matrix->n._22 = input_matrix->n._22 - 2 * v.v.y * z.v.y;
    output_matrix->n._33 = input_matrix->n._33 - 2 * v.v.z * z.v.z;
    output_matrix->n._13 = 0.0f;
    output_matrix->n._31 = output_matrix->n._13;
    output_matrix->n._21 = input_matrix->n._21 - v.v.y * z.v.x;
    output_matrix->n._12 = output_matrix->n._21;


} /*  Math_Float3x3TransformToSymmetricTridiagonalMatrix() */



/*******************************************************************
*
*   Math_Float3x3DominateEigenValueVector()
*
*   DESCRIPTION:
*       calculates the dominate eigenvalues and eigenvectors for a symmetric 3x3 matrix
*       returns false if it failed to converge
*
*******************************************************************/
static bool Math_Float3x3DominateEigenValueVector(const Float3x3 *input_matrix, const uint32_t max_iterations, const float tolerance,  float *output_eigenvalue, Float3 *output_eigenvector )
{
    debug_assert( tolerance < 1 );
    Float3 y = {};
    Float3 dummy = {};
    float magy = 0.0f;
    float one_over_magy = 1.0f;
    float error = 1.0f;

// this is also a failsafe output incase the algorithm fails to converge
    *output_eigenvalue = input_matrix->n._11;
    output_eigenvector->v.x = 0.9901f;
    output_eigenvector->v.y = output_eigenvector->v.z = 0.099f;


    for( uint32_t j = 0; j < max_iterations; j++ )
    {
        y = Math_Float3x3MultiplyByFloat3(input_matrix,output_eigenvector);
        magy = Math_Float3Magnitude( y );
    
        if( magy < tolerance )
        {
            *output_eigenvalue = 0.0f;
            break;
        }
    
        *output_eigenvalue = output_eigenvector->v.x * y.v.x + output_eigenvector->v.y * y.v.y + output_eigenvector->v.z * y.v.z;
        one_over_magy = 1 / magy;
        dummy = Math_Float3ScalarMultiply(y, one_over_magy );
        error = Math_Float3Magnitude( Math_Float3Subtraction ( *output_eigenvector, dummy));
        *output_eigenvector = dummy;
    
        if( error <= tolerance )
        {
            break;
        }
    
        if( j == (max_iterations - 1) )
        {
            printf( "Failed to find an Eigenvlue & Eigenvector in Math_Float3x3EigenValueVector()" );
            return false;
        }
    }

return true;

}/*Math_Float3x3DominateEigenValueVector() */



/*******************************************************************
*
*   Math_Float3x3NextDominateEigenValueVector()
*
*   DESCRIPTION:
*    Input a 3x3 matrix and the dominate eigenvalue/vector and this function 
*    calculates the next dominate eigenvalue/vector.
*
*******************************************************************/

static void Math_Float3x3NextDominateEigenValueVector(const Float3x3 *input_matrix,const float tolerance, const float dominate_eigenvalue, const Float3 dominate_eigenvector, float *output_eigenvalue, Float3 *output_eigenvector )
{
    debug_if( true,
        for( uint32_t i = 0; i < 3; i++ )
        {
           // debug_assert( abs(dominate_eigenvector.f[i]) != 0.0f );
        }        
        );

    uint32_t index_i = 0;
    Float2x2 iteration_matrix_B = {};

    // find the maxiumum component of the dominate eigen vector
    float dummy = abs( dominate_eigenvector.f[0]);
    for( int i = 1; i < 3; i++ )
    {
        if( (abs( dominate_eigenvector.f[i] ) - dummy) > tolerance )
        {
            index_i = i;
            dummy = abs( dominate_eigenvector.f[i] );
        }
    }

    //set the iteration matrix
    switch( index_i )
    {
        case (0):
            iteration_matrix_B.n._11 = input_matrix->n._22 - (dominate_eigenvector.v.y / dominate_eigenvector.v.x) * input_matrix->n._12;
            iteration_matrix_B.n._12 = input_matrix->n._23 - (dominate_eigenvector.v.y / dominate_eigenvector.v.x) * input_matrix->n._13;
            iteration_matrix_B.n._21 = input_matrix->n._32 - (dominate_eigenvector.v.z / dominate_eigenvector.v.x) * input_matrix->n._12;
            iteration_matrix_B.n._22 = input_matrix->n._33 - (dominate_eigenvector.v.z / dominate_eigenvector.v.x) * input_matrix->n._13;
        break;

        case(1):
            iteration_matrix_B.n._11 = input_matrix->n._11 - (dominate_eigenvector.v.x / dominate_eigenvector.v.y) * input_matrix->n._21;
            iteration_matrix_B.n._12 = input_matrix->n._13 - (dominate_eigenvector.v.x / dominate_eigenvector.v.y) * input_matrix->n._23;
            iteration_matrix_B.n._21 = input_matrix->n._31 - (dominate_eigenvector.v.z / dominate_eigenvector.v.y) * input_matrix->n._21;
            iteration_matrix_B.n._22 = input_matrix->n._33 - (dominate_eigenvector.v.z / dominate_eigenvector.v.y) * input_matrix->n._23;
        break;

        case(2):
            iteration_matrix_B.n._11 = input_matrix->n._11 - (dominate_eigenvector.v.x / dominate_eigenvector.v.z) * input_matrix->n._31;
            iteration_matrix_B.n._12 = input_matrix->n._12 - (dominate_eigenvector.v.x / dominate_eigenvector.v.z) * input_matrix->n._32;
            iteration_matrix_B.n._21 = input_matrix->n._21 - (dominate_eigenvector.v.y / dominate_eigenvector.v.z) * input_matrix->n._31;
            iteration_matrix_B.n._22 = input_matrix->n._22 - (dominate_eigenvector.v.y / dominate_eigenvector.v.z) * input_matrix->n._32;
        break;
    }

    // calculate the dominant eigen value and vector of the 2x2 iteration matrix
    float dummy_eigenvalue[2] = {};
    Float2 dummy_eigenvectors[2] = {};
    Float2 temp_eigenvector1 = {};
    Float3 temp_eigenvector2 = {};
    uint32_t dummy_index = 0;

    Math_Float2x2EigenValuesVectors( &iteration_matrix_B,tolerance, 2, dummy_eigenvalue, dummy_eigenvectors );

    if( dummy_eigenvalue[1] < dummy_eigenvalue[0] )
    {
        dummy_index = 1;
    }

    *output_eigenvalue = dummy_eigenvalue[dummy_index];
    temp_eigenvector1 = dummy_eigenvectors[dummy_index];

    //set the output eigen vector
    switch( index_i )
    {
        case (0):
            temp_eigenvector2.v.x = 0.0f;
            temp_eigenvector2.v.y = temp_eigenvector1.v.x;
            temp_eigenvector2.v.z = temp_eigenvector1.v.y;
        break;

        case(1):
            temp_eigenvector2.v.x = temp_eigenvector1.v.x;
            temp_eigenvector2.v.y = 0.0f;
            temp_eigenvector2.v.z = temp_eigenvector1.v.y;
        break;

        case(2):
            temp_eigenvector2.v.x = temp_eigenvector1.v.x;
            temp_eigenvector2.v.y = temp_eigenvector1.v.y;
            temp_eigenvector2.v.z = 0.0f;
        break;
    }

    dummy = 0.0f;
    for( uint32_t j = 0; j < 3; j++ )
    {
        dummy = dummy + input_matrix->f[index_i][j] * temp_eigenvector2.f[j];
    }

    for( uint32_t k = 0; k < 3; k++ )
    {
        output_eigenvector->f[k] = (*output_eigenvalue - dominate_eigenvalue) * temp_eigenvector2.f[k] + dummy * ( dominate_eigenvector.f[k] / dominate_eigenvector.f[index_i]); 
    }

    //normalize the output eigenvector
    float normalization = 1 / Math_Float3Magnitude( *output_eigenvector );
    for( uint32_t k = 0; k < 3; k++ )
    {
        output_eigenvector->f[k] = normalization * output_eigenvector->f[k];
    }


}/* Math_Float3x3NextDominateEigenValueVector()*/



/*******************************************************************
*
*   Math_Float3x3EigenValuesVectors()
*
*   DESCRIPTION:
*       calculates the eigenvalues and eigenvectors for a symmetric 3x3 matrix
*       the Eigenvalues/vectors are ordered by their dominance. 
*       so the 0 index should be the largest eigenvalue and dominate vector
*       and the 2 index is the smalled. 
*       returns false if it failed to converge
*
*******************************************************************/

bool Math_Float3x3EigenValuesVectors( const Float3x3 *input_matrix, const uint32_t max_iterations, const float tolerance, const uint32_t output_capacity, float *output_eigenvalues, Float3 *output_eigenvectors)
{
    assert(output_capacity >= 3);

    float temp_eigenvalues[3] = {};
    Float3 temp_eigenvectors[3] = {};

    if( !Math_Float3x3DominateEigenValueVector( input_matrix, max_iterations, tolerance, &temp_eigenvalues[0], &temp_eigenvectors[0] ) )
    {
        return false;
    }

    Math_Float3x3NextDominateEigenValueVector(input_matrix,tolerance, temp_eigenvalues[0], temp_eigenvectors[0], &temp_eigenvalues[1], &temp_eigenvectors[1] );
    Math_Float3x3NextDominateEigenValueVector( input_matrix,tolerance, temp_eigenvalues[1], temp_eigenvectors[1], &temp_eigenvalues[2], &temp_eigenvectors[2] );


    //sorting the output into largest eigenvalues first
    uint32_t temp_index = 1;
    if( temp_eigenvalues[2] > temp_eigenvalues[1] )
    {
        temp_index = 2;
    }
   
    output_eigenvalues[0] = temp_eigenvalues[0];
    output_eigenvectors[0] = temp_eigenvectors[0];
    output_eigenvalues[1] = temp_eigenvalues[temp_index];
    output_eigenvectors[1] = temp_eigenvectors[temp_index];

    switch( temp_index )
    {
        case 1:
            output_eigenvalues[2] = temp_eigenvalues[2];
            output_eigenvectors[2] = temp_eigenvectors[2];
        break;

        case 2:
            output_eigenvalues[2] = temp_eigenvalues[1];
            output_eigenvectors[2] = temp_eigenvectors[1];
        break;

    }

    return true;
}/*  Math_Float3x3EigenValuesVectors() */