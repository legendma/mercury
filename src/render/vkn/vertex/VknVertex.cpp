#include <cstring>

#include "Global.hpp"
#include "Utilities.hpp"

#include "VknCommon.hpp"
#include "VknVertex.hpp"
#include "vknVertexTypes.hpp"


/*********************************************************************
*
*   PROCEDURE NAME:
*       alloc_attribute
*
*********************************************************************/

static __inline VKN_vertex_attribute_type * alloc_attribute
    (
    VKN_vertex_build_type
                       *builder     /* vertex builder               */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_vertex_attribute_type
                       *ret;        /* return new attribute         */

if( !builder->state.free_attributes )
    {
    debug_assert_always();
    return( NULL );
    }

ret = builder->state.free_attributes;
builder->state.free_attributes = ret->next;
ret->next = NULL;

if( !builder->state.head_attribute )
    {
    builder->state.head_attribute = ret;
    builder->state.tail_attribute = ret;
    }
else
    {
    builder->state.tail_attribute->next = ret;
    builder->state.tail_attribute       = ret;
    }

return( ret );

}   /* alloc_attribute() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       alloc_binding
*
*********************************************************************/

static __inline VKN_vertex_binding_type * alloc_binding
    (
    VKN_vertex_build_type
                       *builder     /* vertex builder               */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_vertex_binding_type
                       *ret;        /* return new binding           */

if( !builder->state.free_bindings )
    {
    debug_assert_always();
    return( NULL );
    }

ret = builder->state.free_bindings;
builder->state.free_bindings = ret->next;
ret->next = NULL;

if( !builder->state.head_binding )
    {
    builder->state.head_binding = ret;
    builder->state.tail_binding = ret;
    }
else
    {
    builder->state.tail_binding->next = ret;
    builder->state.tail_binding       = ret;
    }

return( ret );

}   /* alloc_binding() */


static VKN_vertex_build_add_attribute_proc_type add_attribute;

static bool add_attribute_safe
    (
    const u32           location,   /* attribute location           */
    const VkFormat      format,     /* attribute data type          */
    const u32           offset,     /* element binding offset       */
    VKN_vertex_build_type
                       *builder     /* vertex builder               */
    );

static VKN_vertex_build_add_binding_proc_type add_binding;

static bool add_binding_safe
    (
    u32                 binding,    /* binding index                */
    u32                 stride,     /* consecutive element stride   */
    VkVertexInputRate   rate,       /* addressing mode              */
    VKN_vertex_build_type
                       *builder     /* vertex builder               */
    );

static VKN_vertex_build_build_proc_type build;

static u32 fingerprint
    (
    VKN_vertex_type    *vertex      /* vertex to fingerprint        */
    );

static VKN_vertex_binding_type * get_binding_make_current
    (
    u32                 binding,    /* binding index                */
    VKN_vertex_build_type
                       *builder     /* vertex builder               */
    );

static void sort_attributes
    (
    VKN_vertex_type    *vertex      /* vertex to sort attributes    */
    );

static void sort_bindings
    (
    VKN_vertex_type    *vertex      /* vertex to sort bindings      */
    );


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_vertex_init_builder
*
*   DESCRIPTION:
*       Initialize a vertex builder.
*
*********************************************************************/

VKN_VERTEX_CONFIG_API VKN_vertex_init_builder
    (
    const u32           attribute_capacity,
                                    /* num elements attribute pool  */
    const u32           binding_capacity,
                                    /* num elements in binding pool */
    VKN_vertex_attribute_type
                       *attributes, /* attribute pool               */
    VKN_vertex_binding_type
                       *bindings,   /* binding pool                 */
    VKN_vertex_build_type
                       *builder     /* vertex builder               */
    )
{
/*----------------------------------------------------------
Local constants
----------------------------------------------------------*/
static const VKN_vertex_build_config_type CONFIG =
    {
    add_attribute,
    add_binding,
    build
    };

/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     i;          /* loop counter                 */

/*----------------------------------------------------------
Initialize
----------------------------------------------------------*/
clr_struct( builder );
builder->config = &CONFIG;

builder->state.free_attributes = attributes;
builder->state.free_bindings   = bindings;

memset( attributes, 0, attribute_capacity * sizeof( *attributes ) );
for( i = 1; i < attribute_capacity; i++ )
    {
    attributes[ i - 1 ].next = &attributes[ i ];
    }

memset( bindings, 0, binding_capacity * sizeof( *bindings ) );
for( i = 1; i < binding_capacity; i++ )
    {
    bindings[ i - 1 ].next = &bindings[ i ];
    }

return( builder->config );

}   /* VKN_vertex_init_builder() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       add_attribute
*
*********************************************************************/

static VKN_VERTEX_CONFIG_API add_attribute
    (
    const u32           location,   /* attribute location           */
    const VkFormat      format,     /* attribute data type          */
    const u32           offset,     /* element binding offset       */
    struct _VKN_vertex_build_type
                       *builder     /* vertex builder               */
    )
{
(void)add_attribute_safe( location, format, offset, builder );

return( builder->config );

}   /* add_attribute() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       add_attribute_safe
*
*********************************************************************/

static bool add_attribute_safe
    (
    const u32           location,   /* attribute location           */
    const VkFormat      format,     /* attribute data type          */
    const u32           offset,     /* element binding offset       */
    VKN_vertex_build_type
                       *builder     /* vertex builder               */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_vertex_attribute_type
                       *attribute;  /* working attribute            */
u32                     binding;    /* binding index                */

/*----------------------------------------------------------
We use the working binding
----------------------------------------------------------*/
if( !builder->state.tail_binding )
    {
    debug_assert_always();
    return( FALSE );
    }

binding = builder->state.tail_binding->binding.binding;
    
/*----------------------------------------------------------
Ensure we don't duplicate attributes
----------------------------------------------------------*/
for( attribute = builder->state.head_attribute; attribute; attribute = attribute->next )
    {
    if( attribute->attribute.binding == binding
     && attribute->attribute.location == location )
        {
        break;
        }
    }

if( !attribute )
    {
    attribute = alloc_attribute( builder );
    }

/*----------------------------------------------------------
Fill out attribute
----------------------------------------------------------*/
if( attribute )
    {
    attribute->attribute.location = location;
    attribute->attribute.binding  = binding;
    attribute->attribute.format   = format;
    attribute->attribute.offset   = offset;
    }

return( attribute != NULL );

}   /* add_attribute_safe() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       add_binding
*
*********************************************************************/

static VKN_VERTEX_CONFIG_API add_binding
    (
    u32                 binding,    /* binding index                */
    u32                 stride,     /* consecutive element stride   */
    VkVertexInputRate   rate,       /* addressing mode              */
    struct _VKN_vertex_build_type
                       *builder     /* vertex builder               */
    )
{
(void)add_binding_safe( binding, stride, rate, builder );
return( builder->config );

}   /* add_binding() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       add_binding_safe
*
*********************************************************************/

static bool add_binding_safe
    (
    u32                 binding,    /* binding index                */
    u32                 stride,     /* consecutive element stride   */
    VkVertexInputRate   rate,       /* addressing mode              */
    VKN_vertex_build_type
                       *builder     /* vertex builder               */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_vertex_binding_type
                       *working;    /* working binding              */

/*----------------------------------------------------------
Ensure we don't duplicate bindings
----------------------------------------------------------*/
working = get_binding_make_current( binding, builder );
if( !working )
    {
    working = alloc_binding( builder );
    }

/*----------------------------------------------------------
Fill out binding
----------------------------------------------------------*/
if( working )
    {
    working->binding.binding   = binding;
    working->binding.stride    = stride;
    working->binding.inputRate = rate;
    }

return( working != NULL );

}   /* add_binding_safe() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       build
*
*********************************************************************/

static VKN_vertex_type build
    (
    struct _VKN_vertex_build_type
                       *builder     /* vertex builder               */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_vertex_type         ret;        /* return vertex                */

clr_struct( &ret );
ret.attributes = builder->state.head_attribute;
ret.bindings   = builder->state.head_binding;

builder->state.head_attribute = NULL;
builder->state.head_binding   = NULL;
builder->state.tail_attribute = NULL;
builder->state.tail_binding   = NULL;

sort_attributes( &ret );
sort_bindings( &ret );
ret.fingerprint = fingerprint( &ret );

return( ret );

}   /* build() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       fingerprint
*
*********************************************************************/

static u32 fingerprint
    (
    VKN_vertex_type    *vertex      /* vertex to fingerprint        */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_vertex_attribute_type
                       *attribute;  /* working attribute            */
VKN_vertex_binding_type
                       *binding;    /* working binding              */
u32                     ret;        /* return fingerprint           */

ret = VKN_HASH_SEED;
for( binding = vertex->bindings; binding; binding = binding->next )
    {
    ret = VKN_hash_blob( ret, &binding->binding, sizeof( binding->binding ) );
    }

for( attribute = vertex->attributes; attribute; attribute = attribute->next )
    {
    ret = VKN_hash_blob( ret, &attribute->attribute, sizeof( attribute->attribute ) );
    }

return( ret );

}   /* fingerprint() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       get_binding_make_current
*
*********************************************************************/

static VKN_vertex_binding_type * get_binding_make_current
    (
    u32                 binding,    /* binding index                */
    VKN_vertex_build_type
                       *builder     /* vertex builder               */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_vertex_binding_type
                       *found;      /* found binding                */
VKN_vertex_binding_type
                      **head;       /* current binding head         */

found = NULL;
for( head = &builder->state.head_binding; *head; head = &(*head)->next )
    {
    if( (*head)->binding.binding == binding )
        {
        found = (*head);
        break;
        }
    }

if( found
 && builder->state.tail_binding != found )
    {
    /*------------------------------------------------------
    Move this binding to the back of the list
    ------------------------------------------------------*/
    *head = found->next;
    builder->state.tail_binding->next = found;
    builder->state.tail_binding       = found;
    }

return( found );

}   /* get_binding_make_current() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       sort_attributes
*
*********************************************************************/

static void sort_attributes
    (
    VKN_vertex_type    *vertex      /* vertex to sort attributes    */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_vertex_attribute_type
                       *attribute;  /* working attribute            */
VKN_vertex_attribute_type
                      **head;       /* current attribute head       */
VKN_vertex_attribute_type
                       *sorted;     /* sorted list                  */

sorted = NULL;
while( vertex->attributes )
    {
    attribute = vertex->attributes;
    vertex->attributes = attribute->next;
    attribute->next = NULL;

    if( !sorted )
        {
        sorted = attribute;
        continue;
        }

    for( head = &sorted; *head; head = &(*head)->next )
        {
        if( (*head)->attribute.binding > attribute->attribute.binding
         || ( (*head)->attribute.binding == attribute->attribute.binding
           && (*head)->attribute.location > attribute->attribute.location ) )
           {
           break;
           }
        }

    attribute->next = *head;
    *head = attribute;
    }

vertex->attributes = sorted;

}   /* sort_attributes() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       sort_bindings
*
*********************************************************************/

static void sort_bindings
    (
    VKN_vertex_type    *vertex      /* vertex to sort bindings      */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_vertex_binding_type
                       *binding;    /* working binding              */
VKN_vertex_binding_type
                      **head;       /* current binding head         */
VKN_vertex_binding_type
                       *sorted;     /* sorted list                  */

sorted = NULL;
while( vertex->bindings)
    {
    binding = vertex->bindings;
    vertex->bindings = binding->next;
    binding->next = NULL;

    if( !sorted )
        {
        sorted = binding;
        continue;
        }

    for( head = &sorted; *head; head = &(*head)->next )
        {
        if( (*head)->binding.binding > binding->binding.binding )
           {
           break;
           }
        }

    binding->next = *head;
    *head = binding;
    }

vertex->bindings = sorted;

}   /* sort_bindings() */
