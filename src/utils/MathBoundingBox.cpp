#include <cstdio>

#include "Math.hpp"
#include "Utilities.hpp"


void Math_TightenCapsuleBB2DEndpoints( const Float2 *vertices, const uint32_t num_of_vertices, const Float2 *eigenvectors, const float bounding_sphere_radius, const bool point_A, Float2 *bounding_box_point );

/*******************************************************************
*
*   Math_GenerateAxisAlignedBoundingBox2D()
*
*   DESCRIPTION:
*       Takes as input an array of vertices and generates an
*       AABB bounding box
*
*******************************************************************/

void Math_GenerateAxisAlignedBoundingBox2D(const Float2 *vertices, const int num_of_vertices, BoundingBoxAA2D *bounding_box)
{

    bounding_box->center = {};
    bounding_box->half_extent = {};
        
    float min_x = vertices[0].v.x;
    float max_x = vertices[0].v.x;
    float min_y = vertices[0].v.y;
    float max_y = vertices[0].v.y;

    // crudely find the max and min values for x and y
    for( int i = 1; i < num_of_vertices; i++ )
    {
        if( vertices[i].v.x < min_x )
        {
            min_x = vertices[i].v.x;
         }
        else if( vertices[i].v.x > max_x )
        {
            max_x = vertices[i].v.x;
        }

        if( vertices[i].v.y < min_y )
        {
            min_y = vertices[i].v.y;
        }
        else if( vertices[i].v.y > max_y )
        {
            max_y = vertices[i].v.y;
        }    

    }

    // find midpoint between max and min
    float size_x = 0.5f * (max_x - min_x);
    float size_y = 0.5f * (max_y - min_y);

    //output to the component
    bounding_box->center.v.x = min_x + size_x;
    bounding_box->center.v.y = min_y + size_y;
    bounding_box->half_extent.v.x = size_x;
    bounding_box->half_extent.v.y = size_y;


}/* Math_GenerateAxisAlignedBoundingBox2D()*/



/*******************************************************************
*
*   Math_GenerateAxisAlignedBoundingBox3D()
*
*   DESCRIPTION:
*       takes as input an array of vertices and generates an
*       AABB bounding box
*
*******************************************************************/

void Math_GenerateAxisAlignedBoundingBox3D( const Float3 *vertices, const int num_of_vertices, BoundingBoxAA3D *bounding_box )
{

    bounding_box->center = {};
    bounding_box->half_extent = {};

    float min_x = vertices[0].v.x;
    float max_x = vertices[0].v.x;
    float min_y = vertices[0].v.y;
    float max_y = vertices[0].v.y;
    float min_z = vertices[0].v.z;
    float max_z = vertices[0].v.z;

    // crudely find the max and min values for x and y
    for( int i = 1; i < num_of_vertices; i++ )
    {
        if( vertices[i].v.x < min_x )
        {
            min_x = vertices[i].v.x;
        }
        else if( vertices[i].v.x > max_x )
        {
            max_x = vertices[i].v.x;
        }

        if( vertices[i].v.y < min_y )
        {
            min_y = vertices[i].v.y;
        }
        else if( vertices[i].v.y > max_y )
        {
            max_y = vertices[i].v.y;
        }

        if( vertices[i].v.z < min_z )
        {
            min_y = vertices[i].v.z;
        }
        else if( vertices[i].v.z > max_z )
        {
            max_y = vertices[i].v.z;
        }

    }

    // find midpoint between max and min
    float size_x = 0.5f * (max_x - min_x);
    float size_y = 0.5f * (max_y - min_y);
    float size_z = 0.5f * (max_z - min_z);

    //output to the component
    bounding_box->center.v.x = min_x + size_x;
    bounding_box->center.v.y = min_y + size_y;
    bounding_box->center.v.z = min_z + size_z;
    bounding_box->half_extent.v.x = size_x;
    bounding_box->half_extent.v.y = size_y;
    bounding_box->half_extent.v.z = size_z;


}/* Math_GenerateAxisAlignedBoundingBox3D()*/


/*******************************************************************
*
*   Math_GenerateBoundingBoxSpherefromAABB2D()
*
*   DESCRIPTION:
*       takes as input an AABB bounding box and generates a 
*       bounding sphere from it. note, this will overestimate the
*       bounding sphere and be quite a bit larger then the min possible
*
*******************************************************************/
void Math_GenerateBoundingBoxSpherefromAABB2D( const BoundingBoxAA2D *AABB_bounding_box, BoundingBoxSphere2D *bounding_sphere )
{
    bounding_sphere->center = AABB_bounding_box->center;
    bounding_sphere->radius = Math_Float2Magnitude( AABB_bounding_box->half_extent);

}/* Math_GenerateBoundingBoxSpherefromAABB2D()() */



/*******************************************************************
*
*   Math_GenerateBoundingBoxSpherefromAABB3D()
*
*   DESCRIPTION:
*       takes as input an AABB bounding box and generates a
*       bounding sphere from it. note, this will overestimate the
*       bounding sphere and be quite a bit larger then the min possible
*
*******************************************************************/
void Math_GenerateBoundingBoxSpherefromAABB3D( const BoundingBoxAA3D *AABB_bounding_box, BoundingBoxSphere3D *bounding_sphere )
{
    bounding_sphere->center = AABB_bounding_box->center;
    bounding_sphere->radius = Math_Float3Magnitude( AABB_bounding_box->half_extent );


}/* Math_GenerateBoundingBoxSpherefromAABB3D()() */



/*******************************************************************
*
*   Math_GenerateCapsuleBoundingBox2D()
*
*   DESCRIPTION:
*       takes as input an array of vertices and generates a
*       capsule bounding box
*
*******************************************************************/

void Math_GenerateCapsuleBoundingBox2D( const Float2 *vertices, const uint32_t num_of_vertices,const float tolerance, BoundingBoxCapsule2D *bounding_box )
{
    float eiganvalues[2] = {};
    Float2 eigenvectors[2] = {};
    Float2 center = {};
    Float2 line_segment_AB[2] = {};
    Float2 line_segment_CD[2] = {};


    Math_Float2EigenDecomposition(vertices,num_of_vertices,tolerance,2, eiganvalues,eigenvectors,&center);

    //set the intial values for the OBB dominate and lesser line segments
    line_segment_AB[0] = Math_Float2Subtraction((Math_Float2Addition(center, center)), eigenvectors[0]);
    line_segment_AB[1] = eigenvectors[0];
    line_segment_CD[0] = Math_Float2Subtraction( (Math_Float2Addition( center, center )), eigenvectors[1] );
    line_segment_CD[1] = eigenvectors[1];


    float dummy_distance = 0.0f;
    Float2 temp_new_point = {};

    // calulate the A and B positions of AB that will capture all points when projected onto AB.
    for( uint32_t i = 0; i < num_of_vertices; i++ )
    {
        Math_Float2ProjectionofPointOntoLineSegment( line_segment_AB[0], line_segment_AB[1], vertices[i], &temp_new_point,&dummy_distance);
        
        if( dummy_distance < -0.0f )
        {
            line_segment_AB[0] = temp_new_point;
        }
        else if( dummy_distance > 1.0f )
        {
            line_segment_AB[1] = temp_new_point;
        }
    }

    // calulate the C and D positions of CD that will capture all points when projected onto CD.
    dummy_distance = 0.0f;
    temp_new_point = {};

    for( uint32_t i = 0; i < num_of_vertices; i++ )
    {
        Math_Float2ProjectionofPointOntoLineSegment( line_segment_CD[0], line_segment_CD[1], vertices[i], &temp_new_point, &dummy_distance );

        if( dummy_distance < -0.0f )
        {
            line_segment_CD[0] = temp_new_point;
        }
        else if( dummy_distance > 1.0f)
        {
            line_segment_CD[1] = temp_new_point;
        }
    }

    bounding_box->point_A = line_segment_AB[0];
    bounding_box->point_B = line_segment_AB[1];


    //calculate which side of line_CD is furthest from the intersection point between AB and CD
    Float2 AB_centerpoint = Math_Float2LineSegmentCenterpoint( line_segment_AB[0], line_segment_AB[1] );
    Float2 AB_CD_intersection_point = {};
    Math_Float2LineSegmentIntersection( line_segment_AB[0], line_segment_AB[1], line_segment_CD[0], line_segment_CD[1], tolerance, &AB_CD_intersection_point );
    bounding_box->radius = Math_Float2Magnitude(Math_Float2Subtraction(line_segment_CD[0], AB_CD_intersection_point));
    if( Math_Float2Magnitude( Math_Float2Subtraction( line_segment_CD[1], AB_CD_intersection_point ) ) > bounding_box->radius )
    {
        bounding_box->radius = Math_Float2Magnitude( Math_Float2Subtraction( line_segment_CD[1], AB_CD_intersection_point ));
    }



    // the above code actually generates an OBB. This means the centerpoint of the capsule circle/spheres already contains all the points.  
    //We need to pull this back towards the origin for a tighter fit.
    eigenvectors[0] = Math_Float2Subtraction( eigenvectors[0] , center);
    eigenvectors[1] = Math_Float2Subtraction( eigenvectors[1], center );

    eigenvectors[0] = Math_Float2ScalarMultiply( eigenvectors[0], (1 / Math_Float2Magnitude( eigenvectors[0])));
    eigenvectors[1] = Math_Float2ScalarMultiply( eigenvectors[1], (1 / Math_Float2Magnitude( eigenvectors[1] )) );

    Math_TightenCapsuleBB2DEndpoints( vertices, num_of_vertices, eigenvectors, bounding_box->radius,true, &bounding_box->point_A );
    Math_TightenCapsuleBB2DEndpoints( vertices, num_of_vertices, eigenvectors, bounding_box->radius,false, &bounding_box->point_B );
    
  
}/*Math_GenerateCapsuleBoundingBox2D()*/


/*******************************************************************
*
*   Math_TightenCapsuleBB2DEndpoints()
*
*   DESCRIPTION:
*       takes a capsule bounding box and tigtens the end points 
*       to ensure there is no wasted space
*       note: Eiganvectors need to be normalized and subtracted from the center like 
*      Math_Float2Subtraction( eigenvectors[0] , center);
*
*******************************************************************/

static void Math_TightenCapsuleBB2DEndpoints( const Float2 *vertices, const uint32_t num_of_vertices, const Float2 *eigenvectors, const float bounding_sphere_radius, const bool point_A, Float2 *bounding_box_point )
{

    Float2 P = *bounding_box_point;
    Float2 P_prime = {};
    
    switch( point_A )
    {
        case(true):
            P_prime = Math_Float2Addition( P, Math_Float2ScalarMultiply( eigenvectors[0], bounding_sphere_radius ) );
            break;
        case(false):
            P_prime = Math_Float2Subtraction( P, Math_Float2ScalarMultiply( eigenvectors[0], bounding_sphere_radius ) );
            break;
    }

//generate a list of relevent test points using a dummy AABB around the new/old capsule circle/sphere
Float2 r_e2 = Math_Float2ScalarMultiply( eigenvectors[1], bounding_sphere_radius);
Float2 dummy_AABB[4] =
{
    Math_Float2Addition( P_prime,r_e2 ),
    Math_Float2Subtraction( P_prime,r_e2 ),
    Math_Float2Addition( P,r_e2 ),
    Math_Float2Subtraction( P,r_e2 ),
};

float dummy_max_x = dummy_AABB[0].v.x;
float dummy_min_x = dummy_AABB[0].v.x;
float dummy_max_y = dummy_AABB[0].v.y;
float dummy_min_y = dummy_AABB[0].v.y;
for( uint32_t i = 1; i < 4; i++ )
{
    if( dummy_AABB[i].v.x > dummy_max_x )
    {
        dummy_max_x = dummy_AABB[i].v.x;
    }
    if( dummy_AABB[i].v.x < dummy_min_x )
    {
        dummy_min_x = dummy_AABB[i].v.x;
    }

    if( dummy_AABB[i].v.y > dummy_max_y )
    {
        dummy_max_y = dummy_AABB[i].v.y;
    }
    if( dummy_AABB[i].v.y < dummy_min_y )
    {
        dummy_min_y = dummy_AABB[i].v.y;
    }
}

Float2 *test_verts = (Float2 *)malloc( num_of_vertices * sizeof( Float2 ) );
uint32_t dummy_index = 0;

for( uint32_t i = 0; i < num_of_vertices; i++ )
{
    if( vertices[i].v.x < dummy_min_x )
    {
        continue;
    }
    else if( vertices[i].v.x > dummy_max_x )
    {
        continue;
    }
    else if( vertices[i].v.y < dummy_min_y )
    {
        continue;
    }
    else if( vertices[i].v.y > dummy_max_y )
    {
        continue;
    }
    test_verts[dummy_index] = vertices[i];
    dummy_index++;
}

// find the furthest test_vert from the new capsule sphere center
uint32_t max_index = 0;
float max_dsq = 0.0;
float dum_val = 0.0;
for( uint32_t i = 0; i < dummy_index; i++ )
{
    dum_val = Math_Float2SquareMagnitude( Math_Float2Subtraction( test_verts[i], P_prime ) );
    if( dum_val > max_dsq )
    {
        max_dsq = dum_val;
        max_index = i;
    }
}

if( max_dsq > (bounding_sphere_radius * bounding_sphere_radius) )
{
    float alpha = 0.0;
    float beta = 0.0;

    beta = Math_Float2DotProduct( Math_Float2Subtraction( P_prime, test_verts[max_index] ), eigenvectors[0] );
    alpha = Math_Float2DotProduct( Math_Float2Subtraction( P_prime, test_verts[max_index] ), eigenvectors[1] );

    switch( point_A )
        {
        case(true):
            *bounding_box_point = Math_Float2Subtraction( P_prime, Math_Float2ScalarMultiply( eigenvectors[0], (abs( beta ) - (float)sqrt( bounding_sphere_radius * bounding_sphere_radius - alpha * alpha )) ) );
            break;
        case(false):
            *bounding_box_point = Math_Float2Addition( P_prime, Math_Float2ScalarMultiply( eigenvectors[0], (abs( beta ) - (float)sqrt( bounding_sphere_radius * bounding_sphere_radius - alpha * alpha )) ) );
            break;
    }

}



free( test_verts );

}/* Math_TightenCapsuleBB2DEndpoints() */