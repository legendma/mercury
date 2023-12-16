#include <cmath>

#include "Math.hpp"
#include "Utilities.hpp"


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