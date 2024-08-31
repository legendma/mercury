#include <cstdio>
#include <cstdlib>

#include "Math.hpp"


/*******************************************************************
*
*   Math_Float2CollisionPointToLineDistance()
*
*   DESCRIPTION:
*       Takes as input line normal vector and orgin and compares
*       to a test point. returns the distance from the point to the line
*
*******************************************************************/

float Math_Float2CollisionPointToLineDistance( const Float2 line_origin, const Float2 line_normal_vector, const Float2 test_point )
{
    Float2 Pline_vector = Math_Float2Subtraction( test_point, line_origin );
    return(Math_Float2DotProduct( line_normal_vector, Pline_vector ));

}/* Math_Float2CollisionPointToLineDistance()*/


/*******************************************************************
*
*   Math_Float3CollisionPointToPlaneDistance()
*
*   DESCRIPTION:
*       Takes as input a plane origin and normal vector and compares
*       to a test point. returns the distance from the point to the plane 
*
*******************************************************************/

float Math_Float3CollisionPointToPlaneDistance( const Float3 plane_origin, const Float3 plane_normal_vector, const Float3 test_point )
{
    Float3 Pplane_vector = Math_Float3Subtraction(test_point, plane_origin);
    return(Math_Float3DotProduct(plane_normal_vector, Pplane_vector ));

}/* Math_Float3CollisionPointToPlaneDistance()*/



/*******************************************************************
*
*   Math_Float2CollisionPointToLineNearestPointOnLine()
*
*   DESCRIPTION:
*       Takes as input line normal vector and orgin and compares
*       to a test point. returns a the point on the line that is nearest
*       to the test point.
*
*******************************************************************/

Float2 Math_Float2CollisionPointToLineNearestPointOnLine( const Float2 line_origin, const Float2 line_normal_vector, const Float2 test_point )
{
    float distance = Math_Float2CollisionPointToLineDistance(line_origin, line_normal_vector, test_point);
    Float2 scaled_normal = Math_Float2ScalarMultiply(line_normal_vector, distance);
    return( Math_Float2Subtraction(test_point, scaled_normal));

}/* Math_Float2CollisionPointToLineNearestPointOnLine()*/



/*******************************************************************
*
*   Math_Float3CollisionPointToPlaneNearestPointOnPlane()
*
*   DESCRIPTION:
*       Takes as input plane normal vector and orgin and compares
*       to a test point. returns a the point on the plane that is nearest
*       to the test point.
*
*******************************************************************/

Float3 Math_Float3CollisionPointToPlaneNearestPointOnPlane( const Float3 plane_origin, const Float3 plane_normal_vector, const Float3 test_point )
{
    float distance = Math_Float3CollisionPointToPlaneDistance( plane_origin, plane_normal_vector, test_point );
    Float3 scaled_normal = Math_Float3ScalarMultiply( plane_normal_vector, distance );
    return(Math_Float3Subtraction( test_point, scaled_normal ));

}/* Math_Float3CollisionPointToPlaneNearestPointOnPlane()*/



/*******************************************************************
*
*   Math_Float2CollisionPointToAABB()
*
*   DESCRIPTION:
*       Is the test point inside the given Axis aligned bounding box
*
*******************************************************************/

bool Math_Float2CollisionPointToAABB( const Float2 test_point, const BoundingBoxAA2D bounding_box )
{
    if( test_point.v.x > (bounding_box.center.v.x + bounding_box.half_extent.v.x) )
    {
        return false;
    }
    else if( test_point.v.x < (bounding_box.center.v.x - bounding_box.half_extent.v.x) )
    {
        return false;
    }
    else if( test_point.v.y > ( bounding_box.center.v.y + bounding_box.half_extent.v.y ) )
    {
        return false;
    }
    else if( test_point.v.y < (bounding_box.center.v.y - bounding_box.half_extent.v.y) )
    {
        return false;
    }

    return true;
}/* Math_Float2CollisionPointToAABB() */



/*******************************************************************
*
*   Math_Float3CollisionPointToAABB()
*
*   DESCRIPTION:
*       Is the test point inside the given Axis aligned bounding box
*
*******************************************************************/

bool Math_Float3CollisionPointToAABB( const Float3 test_point, const BoundingBoxAA3D bounding_box )
{
    if( test_point.v.x > (bounding_box.center.v.x + bounding_box.half_extent.v.x) )
    {
        return false;
    }
    else if( test_point.v.x < (bounding_box.center.v.x - bounding_box.half_extent.v.x) )
    {
        return false;
    }
    else if( test_point.v.y > ( bounding_box.center.v.y + bounding_box.half_extent.v.y ) )
    {
        return false;
    }
    else if( test_point.v.y < (bounding_box.center.v.y - bounding_box.half_extent.v.y) )
    {
        return false;
    }
    else if( test_point.v.z > ( bounding_box.center.v.z + bounding_box.half_extent.v.z ) )
    {
        return false;
    }
    else if( test_point.v.z < (bounding_box.center.v.z - bounding_box.half_extent.v.z) )
    {
        return false;
    }


    return true;
}/* Math_Float3CollisionPointToAABB() */


/*******************************************************************
*
*   Math_Float2CollisionPointToSphereBB()
*
*   DESCRIPTION:
*       Is the test point inside the given bounding sphere
*
*******************************************************************/

bool Math_Float2CollisionPointToSphereBB( const Float2 test_point, const BoundingBoxSphere2D bounding_box )
{
    float sphere_radius_squared = bounding_box.radius * bounding_box.radius;
    Float2 delta_to_sphere_origin = Math_Float2Subtraction(test_point, bounding_box.center);

    if( Math_Float2SquareMagnitude( delta_to_sphere_origin ) > sphere_radius_squared )
    {
        return false;
    }
    
    return true;

}/* Math_Float2CollisionPointToSphereBB() */



/*******************************************************************
*
*   Math_Float3CollisionPointToSphereBB()
*
*   DESCRIPTION:
*       Is the test point inside the given bounding sphere
*
*******************************************************************/

bool Math_Float3CollisionPointToSphereBB( const Float3 test_point, const BoundingBoxSphere3D bounding_box )
{
    float sphere_radius_squared = bounding_box.radius * bounding_box.radius;
    Float3 delta_to_sphere_origin = Math_Float3Subtraction( test_point, bounding_box.center );

    if( Math_Float3SquareMagnitude( delta_to_sphere_origin ) > sphere_radius_squared )
    {
        return false;
    }

    return true;

}/* Math_Float3CollisionPointToSphereBB() */


/*******************************************************************
*
*   Math_Float2CollisionAABBToAABB()
*
*   DESCRIPTION:
*       Do the given AABB and AABBs intersect?
*
*******************************************************************/

bool Math_Float2CollisionAABBToAABB( const BoundingBoxAA2D bounding_box_A, const BoundingBoxAA2D bounding_box_B )
{
    for( int i = 0; i < 2; i++ )
    {
        if( abs( bounding_box_A.center.f[i] - bounding_box_B.center.f[i]) >(bounding_box_A.half_extent.f[i] + bounding_box_B.half_extent.f[i]) )
        {
            return false;
        }
    }

    return true;

}/*Math_Float2CollisionAABBToAABB() */


/*******************************************************************
*
*   Math_Float3CollisionAABBToAABB()
*
*   DESCRIPTION:
*       Do the given AABB and AABBs intersect?
*
*******************************************************************/

bool Math_Float3CollisionAABBToAABB( const BoundingBoxAA3D bounding_box_A, const BoundingBoxAA3D bounding_box_B )
{
    for( int i = 0; i < 3; i++ )
    {
        if( abs( bounding_box_A.center.f[i] - bounding_box_B.center.f[i] ) > ( bounding_box_A.half_extent.f[i] + bounding_box_B.half_extent.f[i] ) )
        {
            return false;
        }
    }

    return true;
}/*Math_Float3CollisionAABBToAABB() */



/*******************************************************************
*
*   Math_Float2CollisionAABBToSphereBB()
*
*   DESCRIPTION:
*       Do the given AABB and sphere BBs intersect?
*
*******************************************************************/

bool Math_Float2CollisionAABBToSphereBB( const BoundingBoxAA2D bounding_box, const BoundingBoxSphere2D bounding_sphere )
{
    for( int i = 0; i < 2; i++ )
    { 
        if( abs( bounding_box.center.f[i] - bounding_sphere.center.f[i]) >(bounding_box.half_extent.f[i] + bounding_sphere.radius) )
        {
            return false;
        }
    }

    return true;

}/* Math_Float2CollisionAABBToSphereBB() */


/*******************************************************************
*
*   Math_Float3CollisionAABBToSphereBB()
*
*   DESCRIPTION:
*       Do the given AABB and sphere BBs intersect?
*
*******************************************************************/

bool Math_Float3CollisionAABBToSphereBB( const BoundingBoxAA3D bounding_box, const BoundingBoxSphere3D bounding_sphere )
{
    for( int i = 0; i < 3; i++ )
    {
        if( abs( bounding_box.center.f[i] - bounding_sphere.center.f[i] ) > ( bounding_box.half_extent.f[i] + bounding_sphere.radius ) )
        {
            return false;
        }
    }

    return true;

}/* Math_Float3CollisionAABBToSphereBB() */


/*******************************************************************
*
*   Math_Float2CollisionSphereBBToSphereBB()
*
*   DESCRIPTION:
*       Do the given Sphere BB and sphere BBs intersect?
*
*******************************************************************/

bool Math_Float2CollisionAABBToSphereBBDoTheyIntersect( const BoundingBoxSphere2D bounding_sphere_A, const BoundingBoxSphere2D bounding_sphere_B )
{
    for( int i = 0; i < 2; i++ )
    { 
        if( abs( bounding_sphere_A.center.f[i] - bounding_sphere_B.center.f[i]) > (bounding_sphere_A.radius + bounding_sphere_B.radius) )
        {
            return false;
        }
    }

    return true;

}/* Math_Float2CollisionAABBToSphereBBDoTheyIntersect() */



/*******************************************************************
*
*   Math_Float3CollisionSphereBBToSphereBB()
*
*   DESCRIPTION:
*       Do the given Sphere BB and sphere BBs intersect?
*
*******************************************************************/

bool Math_Float3CollisionAABBToSphereBBDoTheyIntersect( const BoundingBoxSphere3D bounding_sphere_A, const BoundingBoxSphere3D bounding_sphere_B )
{
    for( int i = 0; i < 3; i++ )
    {
        if( abs( bounding_sphere_A.center.f[i] - bounding_sphere_B.center.f[i] ) > ( bounding_sphere_A.radius + bounding_sphere_B.radius ) )
        {
            return false;
        }
    }

    return true;

}/* Math_Float3CollisionAABBToSphereBBDoTheyIntersect() */