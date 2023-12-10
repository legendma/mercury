#include "Math.hpp"
#include "Utilities.hpp"


/*******************************************************************
*
*   Math_QuaternionToFloat4x4()
*
*   DESCRIPTION:
*        Convert a quaternion to a 4x4 rotation matrix.
*
*******************************************************************/

void Math_QuaternionToFloat4x4( const Quaternion in, Float4x4 *out )
{
float q0xq0 = in.v.x * in.v.x;
float q0xq1 = in.v.x * in.v.y;
float q0xq2 = in.v.x * in.v.z;
float q0xq3 = in.v.x * in.v.w;

float q1xq1 = in.v.y * in.v.y;
float q1xq2 = in.v.y * in.v.z;
float q1xq3 = in.v.y * in.v.w;

float q2xq2 = in.v.z * in.v.z;
float q2xq3 = in.v.z * in.v.w;

float q3xq3 = in.v.w * in.v.w;

out->n._11 = 2.0f * ( q0xq0 + q1xq1 ) - 1.0f;
out->n._12 = 2.0f * ( q1xq2 - q0xq3 );
out->n._13 = 2.0f * ( q1xq3 + q0xq2 );
out->n._14 = 0.0f;

out->n._21 = 2.0f * ( q1xq2 + q0xq3 );
out->n._22 = 2.0f * ( q0xq0 + q2xq2 ) - 1.0f;
out->n._23 = 2.0f * ( q2xq3 - q0xq1 );
out->n._24 = 0.0f;

out->n._31 = 2.0f * ( q1xq3 - q0xq2 );
out->n._32 = 2.0f * ( q2xq3 + q0xq1 );
out->n._33 = 2.0f * ( q0xq0 + q3xq3 ) - 1.0f;
out->n._34 = 0.0f;

out->n._41 = 0.0f;
out->n._42 = 0.0f;
out->n._43 = 0.0f;
out->n._44 = 1.0f;

} /*  Math_QuaternionToFloat4x4() */