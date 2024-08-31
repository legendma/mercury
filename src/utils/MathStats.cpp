#include <math.h>
#include <cstdlib>

#include "Math.hpp"
#include "Utilities.hpp"

/*******************************************************************
*
*   Math_floatMean()
*
*   DESCRIPTION:
*       takes as input an array of floats and outputs the mean value
*       pass in 0 for stride to read in the array of floats as normal
*       pass in the size of FloatX otherwise
*
*******************************************************************/

float Math_floatMean( const float *array_of_floats, const int num_of_floats, const size_t stride )
{
    size_t the_stride = stride;
    if( the_stride == 0 )
        {
        the_stride = sizeof(*array_of_floats);
        }

    float count = 0;
    char *as_byte = (char*)array_of_floats;

    for( int i = 0; i < num_of_floats; i++ )
        {
        float *value = (float*)as_byte;
        count = count + *value;
        as_byte += the_stride;
        }

    return (count / num_of_floats);

}/*Math_floatMean() */


/*******************************************************************
*
*   Math_floatStandardDeviation()
*
*   DESCRIPTION:
*       takes as input an array of floats and mean value and outputs
*       the standard deviation.
*       pass in 0 for stride to read in the array of floats as normal
*       pass in the size of FloatX otherwise
*
*******************************************************************/

float Math_floatStandardDeviation( const float *array_of_floats, const int num_of_floats, const float mean_value, const size_t stride )
{
    size_t the_stride = stride;
    if( the_stride == 0 )
    {
        the_stride = sizeof( *array_of_floats );
    }


    float deviation = 0;
    char *as_byte = (char *)array_of_floats;
    for( int i = 0; i < num_of_floats; i++ )
    {
        float *value = (float *)as_byte;
        deviation = deviation + (*value * *value);
        as_byte += the_stride;
    }

    return ((float)sqrt( (deviation / num_of_floats) - mean_value * mean_value ));

}/*Math_floatStandardDeviation() */



/*******************************************************************
*
*   Math_Float2CoVarianceMatrix2x2()
*
*   DESCRIPTION:
*       takes as input an array of Float2 points and calculates the
*       2x2 covariance matrix

*
*******************************************************************/

void Math_Float2CoVarianceMatrix2x2( const Float2 *points, const uint32_t num_of_points, Float2x2 *output )
{
  
    float mean_x = Math_floatMean( &points->v.x, num_of_points, sizeof(*points) );
    float mean_y = Math_floatMean( &points->v.y, num_of_points, sizeof(*points) );

    float sumX_variance = 0.0f;
    float sumY_variance = 0.0f;
    float sum_covariance = 0.0f;

    for( uint32_t k = 0; k < num_of_points; k++ )
    {
        sumX_variance = sumX_variance + (points[k].v.x - mean_x) * (points[k].v.x - mean_x);
        sumY_variance = sumY_variance + (points[k].v.y - mean_y) * (points[k].v.y - mean_y);
        sum_covariance = sum_covariance + (points[k].v.x - mean_x) * (points[k].v.y - mean_y);
    }

    float scale = 1 / ((float)num_of_points - 1.0f);

    sumX_variance  =  scale * sumX_variance;
    sumY_variance  =  scale * sumY_variance;
    sum_covariance = scale * sum_covariance;

    output->n._11 = sumX_variance;
    output->n._22 = sumY_variance;
    output->n._12 = sum_covariance;
    output->n._21 = sum_covariance;

}/*Math_Float2CoVarianceMatrix2x2() */


/*******************************************************************
*
*   Math_Float3CoVarianceMatrix3x3()
*
*   DESCRIPTION:
*       takes as input an array of Float3 points and calculates the
*       3x3 covariance matrix

*
*******************************************************************/

void Math_Float3CoVarianceMatrix3x3( const Float3 *points, const int num_of_points, Float3x3 *output )
{

    float mean_x = Math_floatMean( &points->v.x, num_of_points, sizeof( *points ) );
    float mean_y = Math_floatMean( &points->v.y, num_of_points, sizeof( *points ) );
    float mean_z = Math_floatMean( &points->v.z, num_of_points, sizeof( *points ) );

    float sumX_variance = 0.0f;
    float sumY_variance = 0.0f;
    float sumZ_variance = 0.0f;
    float sum_XYcovariance = 0.0f;
    float sum_XZcovariance = 0.0f;
    float sum_YZcovariance = 0.0f;

    for( int k = 0; k < num_of_points; k++ )
    {
        sumX_variance    = sumX_variance    + (points[k].v.x - mean_x) * (points[k].v.x - mean_x);
        sumY_variance    = sumY_variance    + (points[k].v.y - mean_y) * (points[k].v.y - mean_y);
        sumZ_variance    = sumZ_variance    + (points[k].v.z - mean_z) * (points[k].v.y - mean_z);
        sum_XYcovariance = sum_XYcovariance + (points[k].v.x - mean_x) * (points[k].v.y - mean_y);
        sum_XZcovariance = sum_XZcovariance + (points[k].v.x - mean_x) * (points[k].v.z - mean_z);
        sum_YZcovariance = sum_YZcovariance + (points[k].v.y - mean_y) * (points[k].v.z - mean_z);
    }

    float scale = 1 / ((float)num_of_points - 1.0f);

    sumX_variance    = scale * sumX_variance;
    sumY_variance    = scale * sumY_variance;
    sum_XYcovariance = scale * sum_XYcovariance;
    sum_XZcovariance = scale * sum_XZcovariance;
    sum_YZcovariance = scale * sum_YZcovariance;

    output->n._11 = sumX_variance;
    output->n._22 = sumY_variance;
    output->n._33 = sumZ_variance;

    output->n._12 = sum_XYcovariance;
    output->n._21 = sum_XYcovariance;

    output->n._13 = sum_XZcovariance;
    output->n._31 = sum_XZcovariance;

    output->n._23 = sum_YZcovariance;
    output->n._32 = sum_YZcovariance;


}/*Math_Float3CoVarianceMatrix3x3() */





/*******************************************************************
*
*   Math_Float2EigenDecomposition()
*
*   DESCRIPTION:
*       Takes as input an array of Float2 points and calculates
*       The scaled, orthogonal EigenVectors that can represent the data.
*       Output is an Array of Float2 eigenvectors ordered by their dominance

*
*******************************************************************/

void Math_Float2EigenDecomposition( const Float2 *array_of_points, const uint32_t number_of_points, const float tolerance, const uint32_t size_of_output_arrays, float *output_eigenvalues,Float2 *output_eigenvectors, Float2 *output_vector_origin )
{
    debug_assert(size_of_output_arrays >=2);
    Float2x2 covariance_matrix = {};

    Math_Float2CoVarianceMatrix2x2(array_of_points,number_of_points,&covariance_matrix);
    Math_Float2x2EigenValuesVectors(&covariance_matrix,tolerance, size_of_output_arrays, output_eigenvalues, output_eigenvectors );

    // calculate the array of points origin
    for( uint32_t i = 0; i < 2; i++ )
    {
        output_vector_origin->f[i] = Math_floatMean( &array_of_points->f[i], number_of_points, sizeof( *array_of_points ) );
    }

    // apply the origin to the eiganvector
    for( uint32_t i = 0; i < 2; i++ )
    {
        output_eigenvectors[i] = Math_Float2Addition( output_eigenvectors[i], *output_vector_origin );
    }


}/* Math_Float2EigenDecomposition()*/


/*******************************************************************
*
*   Math_Float3EigenDecomposition()
*
*   DESCRIPTION:
*       Takes as input an array of Float3 points and calculates
*       The scaled, orthogonal EigenVectors that can represent the data.
*       Output is an Array of Float23 eigenvectors ordered by their dominance

*
*******************************************************************/

bool Math_Float3EigenDecomposition( const Float3 *array_of_points, const uint32_t number_of_points, const float tolerance,const uint32_t max_iterations, const uint32_t size_of_output_arrays, float *output_eigenvalues, Float3 *output_eigenvectors, Float3 *output_vector_origin )
{
    // recomended settings are: max_iterations = 40, tolerance = 0.01f. Typically it will only need 10-20 iterations to converge to a solution.
    // a tigher tolerance will need more iterations to converge

    debug_assert( size_of_output_arrays >= 3 );
    Float3x3 covariance_matrix = {};

    Math_Float3CoVarianceMatrix3x3( array_of_points, number_of_points, &covariance_matrix );
    if(!Math_Float3x3EigenValuesVectors( &covariance_matrix,max_iterations, tolerance, size_of_output_arrays, output_eigenvalues, output_eigenvectors ))
    {
        return false;
    }

    // scale the eigenvectors by the eigenvalues. (2 sigma)  
    for( uint32_t j = 0; j < 3; j++ )
    {
        for( uint32_t i = 0; i < 3; i++ )
        {
            output_eigenvectors[j].f[i] = output_eigenvectors[j].f[i] * 2.0f * (float)sqrt( output_eigenvalues[j] );
        }
    }

    // calculate the array of points origin
    for( uint32_t i = 0; i < 3; i++ )
    {
        output_vector_origin->f[i] = Math_floatMean(&array_of_points->f[i], number_of_points, sizeof(*array_of_points));
    }
    
    // apply the origin to the eiganvector
    for( uint32_t i = 0; i < 3; i++ )
    {
        output_eigenvectors[i] = Math_Float3Addition( output_eigenvectors[i], *output_vector_origin );
    }

    return true;
}/* Math_Float3EigenDecomposition()*/
