#include <climits>
#include <cstring>

#include "spirv.h"

#include "Global.hpp"
#include "Utilities.hpp"

#include "VknCommon.hpp"
#include "VknShaderReflect.hpp"
#include "VknShaderReflectTypes.hpp"


#define _lcl_SpvOpTypeAccelerationStructureKHR \
                                    ( 5341 )

#define UNPACK_WORD_MASK_U16        ( 0x0000ffff )
#define UNPACK_WORD_SHIFT_NIBBLE_LOW \
                                    ( 0 )
#define UNPACK_WORD_SHIFT_NIBBLE_HIGH \
                                    ( 16 )
#define INVALID_ENDIAN              ( 0xff )
#define SPIRV_ALIGNMENT             ( 4 * sizeof( u32 ) )

typedef enum
    {
    PARSE_FLAGS_NONE = 0,
    PARSE_FLAGS_PARENT_RUNTIME_ARRAY = shift_bits( 1, 0 )
    } parse_flags_type;

typedef enum
    {
    /* order dependent */
    SECTION_NAME_INSTRUCTIONS,
    SECTION_NAME_DEBUG_INFORMATION,
    SECTION_NAME_ANNOTATIONS,
    SECTION_NAME_TYPES_VARIABLES_CONSTANTS,
    SECTION_NAME_FUNCTIONS,
    /* count */
    SECTION_NAME_CNT
    } section_name_type;

typedef enum
    {
    PRIMITIVE_INVALID,
    PRIMITIVE_UINT32,
    PRIMITIVE_FLOAT32,
    PRIMITIVE_CNT
    } primitive_type;

typedef enum
    {
    IMAGE_DEPTH_FLAG_NON_DEPTH,
    IMAGE_DEPTH_FLAG_DEPTH,
    IMAGE_DEPTH_FLAG_UNKNOWN
    } image_depth_flag_type;

typedef enum
    {
    IMAGE_ARRAYED_FLAG_NON_ARRAYED,
    IMAGE_ARRAYED_FLAG_ARRAYED
    } image_arrayed_flag_type;

typedef enum
    {
    IMAGE_MULTISAMPLE_FLAG_NON_MULTISAMPLED,
    IMAGE_MULTISAMPLE_FLAG_MULTISAMPLED
    } image_multisample_flag_type;

typedef enum
    {
    IMAGE_SAMPLED_FLAG_UNKNOWN,
    IMAGE_SAMPLED_FLAG_SAMPLER,
    IMAGE_SAMPLED_FLAG_READ_WRITE
    } image_sampled_flag_type;


typedef struct
    {
    u32                 magic;      /* magic number                 */
    u32                 version;    /* spir-v version               */
    u32                 generator;  /* ID of spir-v compiler        */
    u32                 bound;      /* opcode ID maximum            */
    u32                 reserved;   /* dummy word                   */
    } header_type;

typedef struct
    {
    u8                  should_endian;
                                    /* should we endian correct?    */
    const u32          *binary;     /* binary being parsed          */
    u32                 word_cnt;   /* number words in binary       */
    u32                 section_start[ SECTION_NAME_CNT ];
                                    /* first word of section        */
    VKN_shader_reflect_descriptor_binding_type
                       *out_bindings;
                                    /* output descriptor bindings   */
    u32                 binding_capacity;
                                    /* output binding array capacity*/
    u32                 binding_cnt;/* number of output bindings    */
    VKN_shader_reflect_push_constant_type
                       *out_constants;
                                    /* output push constants        */
    u32                 constant_capacity;
                                    /* out constant array capacity  */
    u32                 constant_cnt;
                                    /* number of output constants   */
    } parser_common_type;

typedef struct
    {
    section_name_type   section;    /* parser section               */
    u32                 caret;      /* current word in binary       */
    u32                 section_end;/* one word past end of section */
    u32                 remain_instruction_word;
                                    /* remain words in instruction  */
    parser_common_type *common;     /* common parser state          */
    } parser_state_type;

typedef struct
    {
    u32                 result_id;  /* variable's result ID         */
    SpvStorageClass     storage_class;
                                    /* variable's storage class     */
    u32                 type_id;    /* variable's type id           */
    } variable_type;

typedef struct
    {
    u32                 result_id;  /* constant's result ID         */
    primitive_type      kind;       /* primitive type               */
    union
        {
        u32             u32;        /* PRIMITIVE_UINT32             */
        float           f32;        /* PRIMITIVE_FLOAT32            */
        } u;
    } constant_type;

typedef struct
    {
    u32                 type_id;    /* element type                 */
    u32                 length;     /* number of elements in array  */
    } array_type;

typedef struct
    {
    u32                 type_id;    /* element type                 */
    } runtime_array_type;

typedef struct
    {
    u32                 bit_width;  /* number of bits wide          */
    } int_type;

typedef struct
    {
    u32                 type_id;    /* component type               */
    u32                 components; /* number of components         */
    } vector_type;

typedef struct
    {
    SpvStorageClass     storage_class;
                                    /* pointer's storage class      */
    u32                 type_id;    /* type pointed to              */
    } pointer_type;

typedef struct
    {
    u32                 sample_type_id;
                                    /* data type when sampling      */
    SpvDim              dimensions; /* image dimensions             */
    image_depth_flag_type
                        depth;      /* depth usage indication       */
    image_arrayed_flag_type
                        arrayed;    /* arrayed storage indication   */
    image_multisample_flag_type
                        multisample;/* multisampling indication     */
    image_sampled_flag_type
                        sampled;    /* sampler use indication       */
    } image_type;

typedef union
    {
    array_type          as_array;   /* array type                   */
    runtime_array_type  as_runtime_array;
                                    /* runtime array type           */
    image_type          as_image;   /* image type                   */
    int_type            as_int;     /* integer type                 */
    vector_type         as_vector;  /* vectory type                 */
    pointer_type        as_pointer; /* pointer type                 */
    } type_type;


/*********************************************************************
*
*   PROCEDURE NAME:
*       ceiling
*
*   DESCRIPTION:
*       Take the ceiling of the given number to the given multiple.
*
*********************************************************************/

static __inline u32 ceiling
    (
    const u32           value,      /* value to take ceiling        */
    const u32           multiple    /* multiple of this             */
    )
{
return( multiple * ( ( value + ( multiple - 1 ) ) / multiple ) );

}   /* ceiling() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       clamp_max_u32
*
*   DESCRIPTION:
*       Take the ceiling of the given number to the given multiple.
*
*********************************************************************/

static __inline u32 clamp_max_u32
    (
    const u32           value,      /* value to clamp max limit     */
    const u32           maximum     /* maximum limit                */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     ret;        /* return value                 */

ret = value;
if( ret > maximum )
    {
    ret = maximum;
    }

return( ret );

}   /* clamp_max_u32() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       correct_endian
*
*   DESCRIPTION:
*       Correct the given word for endianess.
*
*********************************************************************/

static __inline u32 correct_endian
    (
    const parser_state_type
                       *parser,     /* parser state                 */
    const u32           the_word    /* word to correct              */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     ret;        /* return corrected word        */

if( !parser->common->should_endian )
    {
    return( the_word );
    }

ret = ( ( the_word & 0xff000000 ) >> 24 )
    | ( ( the_word & 0x00ff0000 ) >>  8 )
    | ( ( the_word & 0x0000ff00 ) <<  8 )
    | ( ( the_word & 0x000000ff ) << 24 );

return( ret );

}   /* correct_endian() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       is_constant
*
*   DESCRIPTION:
*       Given an opcode is the instruction a constant?
*
*********************************************************************/

static __inline bool is_constant
    (
    const SpvOp         opcode      /* instruction opcode           */
    )
{
return( opcode == SpvOpConstantTrue
     || opcode == SpvOpConstantFalse
     || opcode == SpvOpConstant
     || opcode == SpvOpConstantComposite
     || opcode == SpvOpConstantSampler
     || opcode == SpvOpConstantNull
     || opcode == SpvOpSpecConstantTrue
     || opcode == SpvOpSpecConstantFalse
     || opcode == SpvOpSpecConstant
     || opcode == SpvOpSpecConstantComposite
     || opcode == SpvOpSpecConstantOp );

}   /* is_constant() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       is_descriptor_binding
*
*   DESCRIPTION:
*       Given an opcode and storage class, is the instruction a
*       descriptor set binding?
*
*********************************************************************/

static __inline bool is_descriptor_binding
    (
    const SpvOp         opcode,     /* instruction opcode           */
    const SpvStorageClass           /* instruction storage class    */
                        storage_class
    )
{
return( opcode == SpvOpVariable
     && ( storage_class == SpvStorageClassUniform
       || storage_class == SpvStorageClassUniformConstant
       || storage_class == SpvStorageClassStorageBuffer ) );

}   /* is_descriptor_binding() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       is_push_constant
*
*   DESCRIPTION:
*       Given an opcode and storage class, is the instruction a push
*       constant?
*
*********************************************************************/

static __inline bool is_push_constant
    (
    const SpvOp         opcode,     /* instruction opcode           */
    const SpvStorageClass           /* instruction storage class    */
                        storage_class
    )
{
return( opcode == SpvOpVariable
     && storage_class == SpvStorageClassPushConstant );

}   /* is_push_constant() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       is_section_annotations
*
*   DESCRIPTION:
*       Given an opcode, is the instruction found in the annotation
*       section?
*
*********************************************************************/

static __inline bool is_section_annotations
    (
    const SpvOp         opcode      /* instruction opcode           */
    )
{
return( opcode == SpvOpDecorate
     || opcode == SpvOpMemberDecorate
     || opcode == SpvOpDecorationGroup
     || opcode == SpvOpGroupDecorate
     || opcode == SpvOpGroupMemberDecorate
     || opcode == SpvOpDecorateId
     || opcode == SpvOpDecorateStringGOOGLE
     || opcode == SpvOpMemberDecorateStringGOOGLE );

}   /* is_section_annotations() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       is_section_debug_info
*
*   DESCRIPTION:
*       Given an opcode, is the instruction found in the debug info
*       section?
*
*********************************************************************/

static __inline bool is_section_debug_info
    (
    const SpvOp         opcode      /* instruction opcode           */
    )
{
return( opcode == SpvOpName
     || opcode == SpvOpMemberName );

}   /* is_section_debug_info() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       is_section_functions
*
*   DESCRIPTION:
*       Given an opcode, is the instruction found in the functions
*       section?
*
*********************************************************************/

static __inline bool is_section_functions
    (
    const SpvOp         opcode      /* instruction opcode           */
    )
{
return( opcode == SpvOpTypeFunction );

}   /* is_section_functions() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       is_type
*
*   DESCRIPTION:
*       Given an opcode is the instruction a type?
*
*********************************************************************/

static __inline bool is_type
    (
    const SpvOp         opcode      /* instruction opcode           */
    )
{
return( opcode == SpvOpTypeStruct
     || opcode == SpvOpTypeVoid
     || opcode == SpvOpTypeBool
     || opcode == SpvOpTypeInt
     || opcode == SpvOpTypeFloat
     || opcode == SpvOpTypeVector
     || opcode == SpvOpTypeMatrix
     || opcode == SpvOpTypeSampler
     || opcode == SpvOpTypeOpaque
     || opcode == SpvOpTypeFunction
     || opcode == SpvOpTypeEvent
     || opcode == SpvOpTypeDeviceEvent
     || opcode == SpvOpTypeReserveId
     || opcode == SpvOpTypeQueue
     || opcode == SpvOpTypePipe
     || opcode == SpvOpTypeImage
     || opcode == SpvOpTypeSampledImage
     || opcode == SpvOpTypeArray
     || opcode == SpvOpTypeRuntimeArray
     || opcode == SpvOpTypePointer
     || opcode == SpvOpTypeForwardPointer
     || opcode == _lcl_SpvOpTypeAccelerationStructureKHR );

}   /* is_push_constant() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       needs_endian
*
*   DESCRIPTION:
*       Does the parser need to correct for endianess?
*
*********************************************************************/

static __inline bool needs_endian
    (
    const u32           magic       /* spir-v magic word            */
    )
{
/*----------------------------------------------------------
Local literals
----------------------------------------------------------*/
#define HOST_BIG_ENDIAN             ( 0x01020304 )
#define HOST_LITTLE_ENDIAN          ( 0x04030201 )
#define SPIRV_MAGIC_WORD            ( 0x07230203 )

/*----------------------------------------------------------
Local macros
----------------------------------------------------------*/
#define get_byte( _binary, _byte_num ) \
    ( 0x000000ff & ( (_binary) >> ( (_byte_num) * CHAR_BIT ) ) )

#define is_binary_le( _arr )                              \
    (  ( (_arr)[ 0 ] == get_byte( SPIRV_MAGIC_WORD, 0 ) ) \
    && ( (_arr)[ 1 ] == get_byte( SPIRV_MAGIC_WORD, 1 ) ) \
    && ( (_arr)[ 2 ] == get_byte( SPIRV_MAGIC_WORD, 2 ) ) \
    && ( (_arr)[ 3 ] == get_byte( SPIRV_MAGIC_WORD, 3 ) ) \
    )

#define is_binary_be( _arr )                              \
    (  ( (_arr)[ 0 ] == get_byte( SPIRV_MAGIC_WORD, 3 ) ) \
    && ( (_arr)[ 1 ] == get_byte( SPIRV_MAGIC_WORD, 2 ) ) \
    && ( (_arr)[ 2 ] == get_byte( SPIRV_MAGIC_WORD, 1 ) ) \
    && ( (_arr)[ 3 ] == get_byte( SPIRV_MAGIC_WORD, 0 ) ) \
    )

/*----------------------------------------------------------
Local types
----------------------------------------------------------*/
typedef union
    {
    struct
        {
        char            a[ sizeof( u32 ) ];
        } as_bytes;
    u32                 as_word;
    } telltale_type;

/*----------------------------------------------------------
Local constants
----------------------------------------------------------*/
static const telltale_type HOST_TELLTALE = { 1, 2, 3, 4 };

/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
telltale_type           binary_telltale;
                                    /* telltale for binary stream   */
bool                    is_host_le; /* host little-endian?          */
bool                    is_binary_le;
                                    /* binary little-endian?        */

/*----------------------------------------------------------
Host
----------------------------------------------------------*/
switch( HOST_TELLTALE.as_word )
    {
    case HOST_BIG_ENDIAN:
        is_host_le = FALSE;
        break;

    case HOST_LITTLE_ENDIAN:
        is_host_le = TRUE;
        break;

    default:
        debug_assert_always();
        is_host_le = TRUE;
        break;
    }

/*----------------------------------------------------------
Binary
----------------------------------------------------------*/
binary_telltale.as_word = magic;
if( is_binary_be( binary_telltale.as_bytes.a ) )
    {
    is_binary_le = FALSE;
    }
else if( is_binary_le( binary_telltale.as_bytes.a ) )
    {
    is_binary_le = TRUE;
    }
else
    {
    debug_assert_always();
    return( FALSE );
    }

return( is_host_le ^ is_binary_le );

#undef get_byte
#undef is_binary_le
#undef is_binary_be
#undef HOST_BIG_ENDIAN
#undef HOST_LITTLE_ENDIAN
#undef SPIRV_MAGIC_WORD
}   /* needs_endian() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       next_instruction
*
*   DESCRIPTION:
*       Fast-foward to the next instruction.
*
*********************************************************************/

static __inline bool next_instruction
    (
    parser_state_type  *parser      /* active parser                */
    )
{
parser->caret += parser->remain_instruction_word;
parser->remain_instruction_word = 0;

return( parser->caret < parser->section_end );

}   /* next_instruction() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       unpack_word
*
*   DESCRIPTION:
*       Read bits from a word given the mask and shift.
*
*********************************************************************/

static __inline u32 unpack_word
    (
    const u32           the_word,   /* word to unpack               */
    const u32           the_mask,   /* mask to apply                */
    const u32           the_shift   /* shift to apply               */
    )
{
return( ( the_word >> the_shift ) & the_mask );

}   /* unpack_word() */


static u32 get_array_length
    (
    const u32           const_id,   /* spir-v name of constant type */
    const parser_state_type
                       *context     /* parser state                 */
    );

static u32 get_array_stride
    (
    const u32           type_id,    /* spir-v name of array type    */
    const parser_state_type
                       *context     /* parser state                 */
    );

static bool get_binding_override_descriptor_type
    (
    const u32           name,       /* string name hash             */
    const VKN_shader_reflect_spirv_override_type
                       *overrides,  /* override descriptions        */
    const u32           override_cnt,
                                    /* number of overrides          */
    const parser_state_type
                       *context,    /* parser state                 */
    VkDescriptorType   *out_descriptor
                                    /* override descriptor type     */
    );

static u32 get_binding_set_index
    (
    const u32           variable_id,/* variable name                */
    const parser_state_type
                       *context     /* parser state                 */
    );

static VkDescriptorType get_block_descriptor_type
    (
    const u32           type_id,    /* spir-v name of the type      */
    const parser_state_type
                       *context     /* parser state                 */
    );

static constant_type get_constant
    (
    const u32           const_id,   /* spir-v name of constant type */
    const parser_state_type
                       *context     /* parser state                 */
    );

static u32 get_descriptor_binding_descriptor_count
    (
    const u32           type_id,    /* spir-v name of the type      */
    const parser_state_type
                       *context     /* parser state                 */
    );

static VkDescriptorType get_descriptor_binding_descriptor_type
    (
    const u32           type_id,    /* spir-v name of the type      */
    const parser_state_type
                       *context     /* parser state                 */
    );

static u32 get_descriptor_binding_index
    (
    const u32           variable_id,/* spir-v name of the variable  */
    const parser_state_type
                       *context     /* parser state                 */
    );

static u32 get_descriptor_type_offset
    (
    const u32           type_id,    /* spir-v name of the type      */
    const parser_state_type
                       *context     /* parser state                 */
    );

static u32 get_member_offset
    (
    const u32           type_id,    /* spir-v name of structure type*/
    const u32           index,      /* member index                 */
    const parser_state_type
                       *context     /* parser state                 */
    );

static SpvOp get_type_opcode
    (
    const u32           type_id,    /* spir-v name of structure type*/
    const parser_state_type
                       *context     /* parser state                 */
    );

static u32 get_type_size
    (
    const u32           type_id,    /* spir-v name of the type      */
    const parse_flags_type
                        flags,      /* parsing flags                */
    const parser_state_type
                       *context     /* parser state                 */
    );

static const char * get_variable_name_string
    (
    const u32           variable_id,/* variable name                */
    const u32           variable_type_id,
                                    /* type id of variable          */
    const parser_state_type
                       *context,    /* parser state                 */
    u32                *out_length  /* output str length (inc null) */
    );

static void initialize_parser
    (
    const u32          *binary,     /* spir-v bytecode              */
    const u32           binary_sz,  /* spir-v bytecode byte size    */
    VKN_shader_reflect_descriptor_binding_type
                       *descriptor_bindings,
                                    /* descriptor set bindings      */
    u32                *descriptor_binding_cnt,
                                    /* descriptor set binding count */
    VKN_shader_reflect_push_constant_type
                       *push_constants,
                                    /* push constants               */
    u32                *push_constant_cnt,
                                    /* push constant count          */
    parser_common_type *common,     /* common parser state          */
    parser_state_type  *parser      /* parser state to initialize   */
    );

static bool is_section_types_variable_constants
    (
    const SpvOp         opcode,     /* instruction opcode           */
    const SpvStorageClass           /* instruction storage class    */
                        storage_class
    );

static bool parse_descriptor_binding
    (
    const variable_type
                       *variable,   /* variable properties          */
    const VKN_shader_reflect_spirv_override_type
                       *overrides,  /* override descriptions        */
    const u32           override_cnt,
                                    /* number of overrides          */
    parser_state_type  *parser      /* parser state                 */
    );

static bool parse_header
    (
    parser_state_type  *parser,     /* parser state                 */
    header_type        *header      /* output parsed header         */
    );

static bool parse_layout
    (
    const parser_state_type
                       *context,    /* parser state                 */
    const VKN_shader_reflect_spirv_override_type
                       *overrides,  /* override descriptions        */
    const u32           override_cnt/* number of overrides          */
    );

static bool parse_push_constant
    (
    const variable_type
                       *variable,   /* variable properties          */
    parser_state_type  *parser      /* parser state                 */
    );

static void parse_sections
    (
    const parser_state_type
                       *context     /* parser state                 */
    );

static bool read_instruction_header
    (
    parser_state_type  *parser,     /* parser state                 */
    SpvOp              *out_opcode, /* instruction's opcode         */
    u16                *out_word_cnt/* instructions's word count    */
    );

static const char * read_string
    (
    parser_state_type  *parser,     /* parser state                 */
    u32                *length      /* length of string (inc NULL)  */
    );

static u32 read_word
    (
    parser_state_type  *parser      /* parser state                 */
    );

static void remove_duplicate_descriptor_bindings
    (
    parser_state_type  *parser      /* parser state                 */
    );

static void rewind_parser
    (
    const section_name_type
                        section,    /* desired parser section       */
    const parser_state_type
                       *context,    /* parser state                 */
    parser_state_type  *parser      /* rewound parser               */
    );

static void sort_descriptor_bindings
    (
    parser_state_type  *parser      /* parser state                 */
    );


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_shader_reflect_make_override
*
*   DESCRIPTION:
*       Fill out an override request.
*
*********************************************************************/

void VKN_shader_reflect_make_override
    (
    const char         *name,       /* binding string name          */
    const VkDescriptorType
                        descriptor, /* desired descriptor type      */
    VKN_shader_reflect_spirv_override_type
                       *out_override/* output override              */
    )
{
/*----------------------------------------------------------
Local literals
----------------------------------------------------------*/
#define MAX_STRING_LENGTH           ( 150 )

clr_struct( out_override );
out_override->name       = VKN_hash_blob( VKN_HASH_SEED, name, (u32)strnlen( name, MAX_STRING_LENGTH ) );
out_override->descriptor = descriptor;

#undef MAX_STRING_LENGTH
}   /* VKN_shader_reflect_make_override() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_shader_reflect_spirv
*
*   DESCRIPTION:
*       Reflect the descriptor sets and push constants of the given
*       SPIR-V binary.
*
*********************************************************************/

bool VKN_shader_reflect_spirv
    (
    const u32          *binary,     /* spir-v bytecode              */
    const u32           binary_sz,  /* spir-v bytecode byte size    */
    VKN_shader_reflect_descriptor_binding_type
                       *descriptor_bindings,
                                    /* descriptor set bindings      */
    u32                *descriptor_binding_cnt,
                                    /* descriptor set binding count */
    VKN_shader_reflect_push_constant_type
                       *push_constants,
                                    /* push constants               */
    u32                *push_constant_cnt
                                    /* push constant count          */
    )
{
return( VKN_shader_reflect_spirv_w_overrides( binary,
                                              binary_sz,
                                              NULL,
                                              0,
                                              descriptor_bindings,
                                              descriptor_binding_cnt,
                                              push_constants,
                                              push_constant_cnt ) );

}   /* VKN_shader_reflect_spirv() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VKN_shader_reflect_spirv_w_overrides
*
*   DESCRIPTION:
*       Reflect the descriptor sets and push constants of the given
*       SPIR-V binary.  Descriptor sets can have their types
*       overridden.
*
*********************************************************************/

bool VKN_shader_reflect_spirv_w_overrides
    (
    const u32          *binary,     /* spir-v bytecode              */
    const u32           binary_sz,  /* spir-v bytecode byte size    */
    const VKN_shader_reflect_spirv_override_type
                       *overrides,  /* override descriptions        */
    const u32           override_cnt,
                                    /* number of overrides          */
    VKN_shader_reflect_descriptor_binding_type
                       *descriptor_bindings,
                                    /* descriptor set bindings      */
    u32                *descriptor_binding_cnt,
                                    /* descriptor set binding count */
    VKN_shader_reflect_push_constant_type
                       *push_constants,
                                    /* push constants               */
    u32                *push_constant_cnt
                                    /* push constant count          */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
parser_common_type      common;     /* common parser state          */
header_type             header;     /* header block                 */
parser_state_type       parser;     /* parser state                 */

/*----------------------------------------------------------
Initialize the parser state
----------------------------------------------------------*/
initialize_parser( binary,
                   binary_sz,
                   descriptor_bindings,
                   descriptor_binding_cnt,
                   push_constants,
                   push_constant_cnt,
                   &common,
                   &parser );

/*----------------------------------------------------------
Header
----------------------------------------------------------*/
clr_struct( &header );
if( !parse_header( &parser, &header ) )
    {
    return( FALSE );
    }

/*----------------------------------------------------------
Instructions
----------------------------------------------------------*/
parser.common->section_start[ SECTION_NAME_INSTRUCTIONS ] = parser.caret;
parse_sections( &parser );
parse_layout( &parser, overrides, override_cnt );
sort_descriptor_bindings( &parser );
remove_duplicate_descriptor_bindings( &parser );

/*----------------------------------------------------------
Output
----------------------------------------------------------*/
if( descriptor_binding_cnt )
    {
    *descriptor_binding_cnt = parser.common->binding_cnt;
    }

if( push_constant_cnt )
    {
    *push_constant_cnt = parser.common->constant_cnt;
    }

return( TRUE );

}   /* VKN_shader_reflect_spirv_w_overrides() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       get_array_length
*
*   DESCRIPTION:
*       Discover an array's length.
*
*********************************************************************/

static u32 get_array_length
    (
    const u32           const_id,   /* spir-v name of constant type */
    const parser_state_type
                       *context     /* parser state                 */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
constant_type           constant;   /* constant value               */

constant = get_constant( const_id, context );
debug_assert( constant.kind == PRIMITIVE_UINT32 );

return( constant.u.u32 );

}   /* get_array_length() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       get_array_stride
*
*   DESCRIPTION:
*       Discover an array's stride.
*
*********************************************************************/

static u32 get_array_stride
    (
    const u32           type_id,    /* spir-v name of array type    */
    const parser_state_type
                       *context     /* parser state                 */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     array_type_id;
                                    /* type ID of array             */
SpvDecoration           decoration; /* decoration type              */
bool                    found;      /* did we find it?              */
SpvOp                   opcode;     /* instruction's opcode         */
parser_state_type       parser;     /* rewound parser               */
u32                     ret;        /* return byte offset           */

ret = 0;
found = FALSE;
rewind_parser( SECTION_NAME_ANNOTATIONS, context, &parser );
while( next_instruction( &parser ) )
    {
    /*------------------------------------------------------
    Read the instruction header
    ------------------------------------------------------*/
    if( !read_instruction_header( &parser, &opcode, NULL ) )
        {
        return( 0 );
        }

    /*------------------------------------------------------
    Looking for non-members
    ------------------------------------------------------*/
    if( opcode != SpvOpDecorate )
        {
        continue;
        }

    /*------------------------------------------------------
    Of a particular array type
    ------------------------------------------------------*/
    array_type_id = read_word( &parser );
    if( type_id != array_type_id )
        {
        continue;
        }

    /*------------------------------------------------------
    And we want the stride decoration
    ------------------------------------------------------*/
    decoration = (SpvDecoration)read_word( &parser );
    if( decoration != SpvDecorationArrayStride )
        {
        continue;
        }

    /*------------------------------------------------------
    Found it
    ------------------------------------------------------*/
    found = TRUE;
    ret = read_word( &parser );
    break;
    }

/*----------------------------------------------------------
Return the stride
----------------------------------------------------------*/
(void)found;
debug_assert( found );

return( ret );

}   /* get_array_stride() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       get_binding_override_descriptor_type
*
*   DESCRIPTION:
*       Get the override descriptor type for the given binding.
*       Returns TRUE if override was found.
*
*********************************************************************/

static bool get_binding_override_descriptor_type
    (
    const u32           name,       /* string name hash             */
    const VKN_shader_reflect_spirv_override_type
                       *overrides,  /* override descriptions        */
    const u32           override_cnt,
                                    /* number of overrides          */
    const parser_state_type
                       *context,    /* parser state                 */
    VkDescriptorType   *out_descriptor
                                    /* override descriptor type     */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     i;          /* loop counter                 */
bool                    ret;        /* did we return an override?   */

*out_descriptor = VK_DESCRIPTOR_TYPE_MAX_ENUM;
if( name == 0
 || !override_cnt )
    {
    /*------------------------------------------------------
    No overrides, don't bother
    ------------------------------------------------------*/
    return( FALSE );
    }

/*----------------------------------------------------------
Attempt to match this variable to an override
----------------------------------------------------------*/
ret = FALSE;
for( i = 0; i < override_cnt; i++ )
    {
    if( overrides[ i ].name == name )
        {
        /*--------------------------------------------------
        Found an override for this variable
        --------------------------------------------------*/
        ret = TRUE;
        *out_descriptor = overrides[ i ].descriptor;
        break;
        }
    }

return( ret );

}   /* get_binding_override_descriptor_type() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       get_binding_set_index
*
*   DESCRIPTION:
*       Get a descriptor binding's set index.
*
*********************************************************************/

static u32 get_binding_set_index
    (
    const u32           variable_id,/* variable name                */
    const parser_state_type
                       *context     /* parser state                 */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
SpvDecoration           decoration; /* decoration type              */
bool                    found;      /* did we find it?              */
SpvOp                   opcode;     /* instruction's opcode         */
parser_state_type       parser;     /* rewound parser               */
u32                     result_id;  /* instruction's name id        */
u32                     ret;        /* return set index             */

found = FALSE;
rewind_parser( SECTION_NAME_ANNOTATIONS, context, &parser );

ret = max_uint_value( ret );
while( next_instruction( &parser ) )
    {
    /*------------------------------------------------------
    Read the instruction header
    ------------------------------------------------------*/
    if( !read_instruction_header( &parser, &opcode, NULL ) )
        {
        break;
        }

    /*------------------------------------------------------
    Looking for non-members
    ------------------------------------------------------*/
    if( opcode != SpvOpDecorate )
        {
        continue;
        }

    /*------------------------------------------------------
    That matches our given id
    ------------------------------------------------------*/
    result_id = read_word( &parser );
    if( variable_id != result_id )
        {
        continue;
        }

    /*------------------------------------------------------
    And contains our set id
    ------------------------------------------------------*/
    decoration = (SpvDecoration)read_word( &parser );
    if( decoration != SpvDecorationDescriptorSet )
        {
        continue;
        }

    /*------------------------------------------------------
    Found it
    ------------------------------------------------------*/
    found = TRUE;
    ret = read_word( &parser );
    break;
    }

(void)found;
debug_assert( found );

return( ret );

}   /* get_binding_set_index() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       get_block_descriptor_type
*
*   DESCRIPTION:
*       Get the specific descriptor block for the given type.
*
*********************************************************************/

static VkDescriptorType get_block_descriptor_type
    (
    const u32           type_id,    /* spir-v name of the type      */
    const parser_state_type
                       *context     /* parser state                 */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     block_type_id;
                                    /* type ID of block             */
SpvDecoration           decoration; /* decoration type              */
SpvOp                   opcode;     /* instruction's opcode         */
parser_state_type       parser;     /* rewound parser               */
VkDescriptorType        ret;        /* return descriptor type       */

rewind_parser( SECTION_NAME_ANNOTATIONS, context, &parser );

ret = VK_DESCRIPTOR_TYPE_MAX_ENUM;
while( next_instruction( &parser ) )
    {
    /*------------------------------------------------------
    Read the instruction header
    ------------------------------------------------------*/
    if( !read_instruction_header( &parser, &opcode, NULL ) )
        {
        break;
        }

    /*------------------------------------------------------
    Looking for non-members
    ------------------------------------------------------*/
    if( opcode != SpvOpDecorate )
        {
        continue;
        }

    /*------------------------------------------------------
    Of a particular block type
    ------------------------------------------------------*/
    block_type_id = read_word( &parser );
    if( type_id != block_type_id)
        {
        continue;
        }

    /*------------------------------------------------------
    And we want the block type decoration
    ------------------------------------------------------*/
    decoration = (SpvDecoration)read_word( &parser );
    switch( decoration )
        {
        case SpvDecorationBlock:
            ret = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            break;

        case SpvDecorationBufferBlock:
            ret = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            break;

        default:
            continue;
        }

    break;
    }

debug_assert( ret != VK_DESCRIPTOR_TYPE_MAX_ENUM );

return( ret );

}   /* get_block_descriptor_type() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       get_constant
*
*   DESCRIPTION:
*       Get the given constant.
*
*********************************************************************/

static constant_type get_constant
    (
    const u32           const_id,   /* spir-v name of constant type */
    const parser_state_type
                       *context     /* parser state                 */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     const_type_id;
                                    /* type ID of constant          */
bool                    found;      /* did we find it?              */
SpvOp                   opcode;     /* instruction's opcode         */
parser_state_type       parser;     /* rewound parser               */
u32                     result_id;  /* instruction's name id        */
constant_type           ret;        /* return constant              */
SpvOp                   type_opcode;/* the constant's primitive type*/

clr_struct( &ret );
found = FALSE;
rewind_parser( SECTION_NAME_TYPES_VARIABLES_CONSTANTS, context, &parser );
while( next_instruction( &parser ) )
    {
    /*------------------------------------------------------
    Read the instruction header
    ------------------------------------------------------*/
    if( !read_instruction_header( &parser, &opcode, NULL ) )
        {
        break;
        }

    /*------------------------------------------------------
    Looking for constants
    ------------------------------------------------------*/
    if( !is_constant( opcode ) )
        {
        continue;
        }

    /*------------------------------------------------------
    That matches our given id
    ------------------------------------------------------*/
    const_type_id = read_word( &parser );
    result_id     = read_word( &parser );
    if( const_id != result_id )
        {
        continue;
        }

    /*------------------------------------------------------
    Fill out the constant
    ------------------------------------------------------*/
    type_opcode = get_type_opcode( const_type_id, &parser );
    switch( type_opcode )
        {
        case SpvOpTypeInt:
            ret.kind  = PRIMITIVE_UINT32;
            ret.u.u32 = read_word( &parser );
            break;

        case SpvOpTypeFloat:
            ret.kind  = PRIMITIVE_FLOAT32;
            ret.u.f32 = (float)read_word( &parser );
            break;

        default:
            /*----------------------------------------------
            Unsupported
            ----------------------------------------------*/
            debug_assert_always();
            break;
        }

    /*------------------------------------------------------
    Found it
    ------------------------------------------------------*/
    found = TRUE;
    break;
    }

/*----------------------------------------------------------
Return the stride
----------------------------------------------------------*/
(void)found;
debug_assert( found );

return( ret );

}   /* get_constant() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       get_descriptor_binding_descriptor_count
*
*   DESCRIPTION:
*       Discover the given type's descriptor count.
*
*********************************************************************/

static u32 get_descriptor_binding_descriptor_count
    (
    const u32           type_id,    /* spir-v name of the type      */
    const parser_state_type
                       *context     /* parser state                 */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
bool                    found;      /* did we find the type?        */
SpvOp                   opcode;     /* instruction's opcode         */
parser_state_type       parser;     /* rewound parser               */
u32                     ret;        /* return descriptor count      */
u32                     result_id;  /* instructions type id         */
type_type               type_working;
                                    /* working type                 */

rewind_parser( SECTION_NAME_TYPES_VARIABLES_CONSTANTS, context, &parser );

ret = 1;
while( next_instruction( &parser ) )
    {
    /*------------------------------------------------------
    Read the instruction header
    ------------------------------------------------------*/
    if( !read_instruction_header( &parser, &opcode, NULL ) )
        {
        debug_assert_always();
        return( max_uint_value( ret ) );
        }

    /*------------------------------------------------------
    Check if we reached the end of the types without finding
    the request
    ------------------------------------------------------*/
    if( opcode == SpvOpFunction )
        {
        debug_assert_always();
        return( max_uint_value( ret ) );
        }

    /*------------------------------------------------------
    We only care about types
    ------------------------------------------------------*/
    if( !is_type( opcode ) )
        {
        continue;
        }

    /*------------------------------------------------------
    All type start with their type id
    ------------------------------------------------------*/
    result_id = read_word( &parser );
    if( result_id != type_id )
        {
        /*--------------------------------------------------
        Not the type we are looking for
        --------------------------------------------------*/
        continue;
        }

    found = TRUE;

    /*------------------------------------------------------
    Descend on pointer types
    ------------------------------------------------------*/
    if( opcode == SpvOpTypePointer )
        {
        clr_struct( &type_working );
        type_working.as_pointer.storage_class = (SpvStorageClass)read_word( &parser );
        // TODO <MPA> Maybe verify the storage class matches descriptor binding?
        type_working.as_pointer.type_id       = read_word( &parser );

        ret = get_descriptor_binding_descriptor_count( type_working.as_pointer.type_id, &parser );
        break;
        }

    /*------------------------------------------------------
    Process the type
    ------------------------------------------------------*/
    clr_struct( &type_working );
    switch( opcode )
        {
        /*--------------------------------------------------
        Arrays
        --------------------------------------------------*/
        case SpvOpTypeArray:
            type_working.as_array.type_id = read_word( &parser );
            type_working.as_array.length  = read_word( &parser );

            ret = get_array_length( type_working.as_array.length, &parser );
            break;

        default:
            /*----------------------------------------------
            We should be handling all the types
            ----------------------------------------------*/
            debug_assert( opcode == SpvOpTypeVoid
                         || opcode == SpvOpTypeBool
                         || opcode == SpvOpTypeInt
                         || opcode == SpvOpTypeFloat
                         || opcode == SpvOpTypeVector
                         || opcode == SpvOpTypeMatrix // TODO <MPA> - Should we be processing matrices as well?
                         || opcode == SpvOpTypeImage
                         || opcode == SpvOpTypeSampler
                         || opcode == SpvOpTypeSampledImage
                         || opcode == SpvOpTypeRuntimeArray
                         || opcode == SpvOpTypeStruct
                         || opcode == SpvOpTypeOpaque
                         || opcode == SpvOpTypePointer
                         || opcode == SpvOpTypeFunction
                         || opcode == SpvOpTypeEvent
                         || opcode == SpvOpTypeDeviceEvent
                         || opcode == SpvOpTypeReserveId
                         || opcode == SpvOpTypeQueue
                         || opcode == SpvOpTypePipe
                         || opcode == SpvOpTypeForwardPointer );
            break;
        }

    break;
    }

/*----------------------------------------------------------
Return the descriptor count
----------------------------------------------------------*/
(void)found;
debug_assert( found );

return( ret );

}   /* get_descriptor_binding_descriptor_count() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       get_descriptor_binding_descriptor_type
*
*   DESCRIPTION:
*       Discover the given type's descriptor type.
*
*********************************************************************/

static VkDescriptorType get_descriptor_binding_descriptor_type
    (
    const u32           type_id,    /* spir-v name of the type      */
    const parser_state_type
                       *context     /* parser state                 */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
bool                    found;      /* did we find it?              */
SpvOp                   opcode;     /* instruction's opcode         */
parser_state_type       parser;     /* rewound parser               */
VkDescriptorType        ret;        /* return descriptor type       */
u32                     result_id;  /* instructions type id         */
type_type               type_working;
                                    /* working type                 */
u16                     word_cnt;   /* instruction word count       */

found = FALSE;
rewind_parser( SECTION_NAME_TYPES_VARIABLES_CONSTANTS, context, &parser );

ret = VK_DESCRIPTOR_TYPE_MAX_ENUM;
while( next_instruction( &parser ) )
    {
    /*------------------------------------------------------
    Read the instruction header
    ------------------------------------------------------*/
    if( !read_instruction_header( &parser, &opcode, &word_cnt ) )
        {
        debug_assert_always();
        return( VK_DESCRIPTOR_TYPE_MAX_ENUM );
        }

    /*------------------------------------------------------
    Check if we reached the end of the types without finding
    the request
    ------------------------------------------------------*/
    if( opcode == SpvOpFunction )
        {
        debug_assert_always();
        return( VK_DESCRIPTOR_TYPE_MAX_ENUM );
        }

    /*------------------------------------------------------
    We only care about types
    ------------------------------------------------------*/
    if( !is_type( opcode ) )
        {
        continue;
        }

    /*------------------------------------------------------
    All type start with their type id
    ------------------------------------------------------*/
    result_id = read_word( &parser );
    if( result_id != type_id )
        {
        /*--------------------------------------------------
        Not the type we are looking for
        --------------------------------------------------*/
        continue;
        }

    found = TRUE;
    clr_struct( &type_working );

    /*------------------------------------------------------
    Descend on pointer and array types
    ------------------------------------------------------*/
    if( opcode == SpvOpTypePointer )
        {
        type_working.as_pointer.storage_class = (SpvStorageClass)read_word( &parser );
        // TODO <MPA> Maybe verify the storage class matches descriptor binding?
        type_working.as_pointer.type_id       = read_word( &parser );

        if( type_working.as_pointer.storage_class == SpvStorageClassStorageBuffer )
            {
            ret = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            }
        else
            {
            ret = get_descriptor_binding_descriptor_type( type_working.as_pointer.type_id, &parser );
            }
        break;
        }
    if( opcode == SpvOpTypeArray )
        {
        type_working.as_array.type_id = read_word( &parser );
        ret = get_descriptor_binding_descriptor_type( type_working.as_array.type_id, &parser );
        break;
        }

    /*------------------------------------------------------
    Process the type
    ------------------------------------------------------*/
    switch( opcode )
        {
        /*--------------------------------------------------
        Images
        --------------------------------------------------*/
        case SpvOpTypeImage:
        case SpvOpTypeSampledImage:
            type_working.as_image.sample_type_id = read_word( &parser );
            type_working.as_image.dimensions     = (SpvDim)read_word( &parser );
            type_working.as_image.depth          = (image_depth_flag_type)read_word( &parser );
            type_working.as_image.arrayed        = (image_arrayed_flag_type)read_word( &parser );
            type_working.as_image.multisample    = (image_multisample_flag_type)read_word( &parser );
            type_working.as_image.sampled        = (image_sampled_flag_type)read_word( &parser );

            if( type_working.as_image.dimensions == SpvDimBuffer )
                {
                /*------------------------------------------
                Buffer type image
                ------------------------------------------*/
                if( type_working.as_image.sampled == IMAGE_SAMPLED_FLAG_SAMPLER )
                    {
                    ret = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
                    break;
                    }
                else if( type_working.as_image.sampled == IMAGE_SAMPLED_FLAG_READ_WRITE )
                    {
                    ret = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
                    break;
                    }
                else
                    {
                    debug_assert_always();
                    break;
                    }
                }
            else if( opcode == SpvOpTypeSampledImage )
                {
                /*------------------------------------------
                Both image resource and used with sampler
                ------------------------------------------*/
                ret = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                }
            else if( type_working.as_image.dimensions == SpvDimSubpassData )
                {
                /*------------------------------------------
                Framebuffer attachment
                ------------------------------------------*/
                ret = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
                break;
                }
            else
                {
                /*------------------------------------------
                Non-buffer image
                ------------------------------------------*/
                if( type_working.as_image.sampled == IMAGE_SAMPLED_FLAG_SAMPLER )
                    {
                    ret = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                    break;
                    }
                else if( type_working.as_image.sampled == IMAGE_SAMPLED_FLAG_READ_WRITE )
                    {
                    ret = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                    break;
                    }
                else
                    {
                    debug_assert_always();
                    break;
                    }

                }

            break;

        /*--------------------------------------------------
        Raytracing accelerator
        --------------------------------------------------*/
        case _lcl_SpvOpTypeAccelerationStructureKHR:
            ret = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
            break;

        /*--------------------------------------------------
        Structures
        --------------------------------------------------*/
        case SpvOpTypeStruct:
            ret = get_block_descriptor_type( type_id, &parser );
            break;

        /*--------------------------------------------------
        Samplers
        --------------------------------------------------*/
        case SpvOpTypeSampler:
            ret = VK_DESCRIPTOR_TYPE_SAMPLER;
            break;

        default:
            /*----------------------------------------------
            We should be handling all the types
            ----------------------------------------------*/
            debug_assert_always();
            continue;
        }

    break;
    }

/*----------------------------------------------------------
Return the descriptor type
----------------------------------------------------------*/
(void)found;
debug_assert( found );

return( ret );

}   /* get_descriptor_binding_descriptor_type() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       get_descriptor_binding_index
*
*   DESCRIPTION:
*       Discover the given variable's binding index.
*
*********************************************************************/

static u32 get_descriptor_binding_index
    (
    const u32           variable_id,/* spir-v name of the variable  */
    const parser_state_type
                       *context     /* parser state                 */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
SpvDecoration           decoration; /* decoration type              */
bool                    found;      /* did we find it?              */
SpvOp                   opcode;     /* instruction's opcode         */
parser_state_type       parser;     /* rewound parser               */
u32                     result_id;  /* instructions variable id     */
u32                     ret;        /* return binding index         */

found = FALSE;
rewind_parser( SECTION_NAME_ANNOTATIONS, context, &parser );

ret = max_uint_value( ret );
while( next_instruction( &parser ) )
    {
    /*------------------------------------------------------
    Read the instruction header
    ------------------------------------------------------*/
    if( !read_instruction_header( &parser, &opcode, NULL ) )
        {
        debug_assert_always();
        return( max_uint_value( ret ) );
        }

    /*------------------------------------------------------
    Looking for non-members
    ------------------------------------------------------*/
    if( opcode != SpvOpDecorate )
        {
        continue;
        }

    /*------------------------------------------------------
    Of a particular variable
    ------------------------------------------------------*/
    result_id = read_word( &parser );
    if( variable_id != result_id )
        {
        continue;
        }

    /*------------------------------------------------------
    And we want the binding index decoration
    ------------------------------------------------------*/
    decoration = (SpvDecoration)read_word( &parser );
    if( decoration != SpvDecorationBinding )
        {
        continue;
        }

    /*------------------------------------------------------
    Found it
    ------------------------------------------------------*/
    found = TRUE;
    ret = read_word( &parser );
    break;
    }

/*----------------------------------------------------------
Return the descriptor binding index
----------------------------------------------------------*/
(void)found;
debug_assert( found );

return( ret );

}   /* get_descriptor_binding_index() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       get_descriptor_type_offset
*
*   DESCRIPTION:
*       Discover the offset of a descriptor's type.
*
*********************************************************************/

static u32 get_descriptor_type_offset
    (
    const u32           type_id,    /* spir-v name of the type      */
    const parser_state_type
                       *context     /* parser state                 */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
bool                    found;      /* did we find it?              */
u32                     i;          /* loop counter                 */
u32                     member_cnt; /* number of type members       */
SpvOp                   opcode;     /* instruction's opcode         */
parser_state_type       parser;     /* rewound parser               */
u32                     result_id;  /* instructions type id         */
u32                     ret;        /* return byte offset           */
type_type               type_working;
                                    /* working type                 */
u16                     word_cnt;   /* instruction word count       */

ret = 0;
found = FALSE;
rewind_parser( SECTION_NAME_TYPES_VARIABLES_CONSTANTS, context, &parser );
while( next_instruction( &parser ) )
    {
    /*------------------------------------------------------
    Read the instruction header
    ------------------------------------------------------*/
    if( !read_instruction_header( &parser, &opcode, &word_cnt ) )
        {
        return( 0 );
        }

    /*------------------------------------------------------
    Check if we reached the end of the types without finding
    the request
    ------------------------------------------------------*/
    if( opcode == SpvOpFunction )
        {
        debug_assert_always();
        return( 0 );
        }

    /*------------------------------------------------------
    We only care about types
    ------------------------------------------------------*/
    if( !is_type( opcode ) )
        {
        continue;
        }

    /*------------------------------------------------------
    All type start with their type id
    ------------------------------------------------------*/
    result_id = read_word( &parser );
    if( result_id != type_id )
        {
        /*--------------------------------------------------
        Not the type we are looking for
        --------------------------------------------------*/
        continue;
        }

    found = TRUE;

    /*------------------------------------------------------
    Descend on pointer types
    ------------------------------------------------------*/
    if( opcode == SpvOpTypePointer )
        {
        clr_struct( &type_working );
        type_working.as_pointer.storage_class = (SpvStorageClass)read_word( &parser );
        // TODO <MPA> Maybe verify the storage class matches push constant or descriptor binding?
        type_working.as_pointer.type_id       = read_word( &parser );

        ret = get_descriptor_type_offset( type_working.as_pointer.type_id, &parser );
        break;
        }

    /*------------------------------------------------------
    We must now be looking at a structure type
    ------------------------------------------------------*/
    if( opcode != SpvOpTypeStruct )
        {
        debug_assert_always();
        return( 0 );
        }

    /*------------------------------------------------------
    Get the offset of the structure's first member
    ------------------------------------------------------*/
    member_cnt = word_cnt - 2; /* instruction header and result id */
    for( i = 0; i < member_cnt; i++ )
        {
        if( !i )
            {
            ret = max_uint_value( ret );
            }

        ret = clamp_max_u32( get_member_offset( result_id, i, &parser ), ret );
        }

    break;
    }

/*----------------------------------------------------------
Return the offset
----------------------------------------------------------*/
(void)found;
debug_assert( found );

return( ret );

}   /* get_descriptor_type_offset() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       get_member_offset
*
*   DESCRIPTION:
*       Discover a structure member's offset.
*
*********************************************************************/

static u32 get_member_offset
    (
    const u32           type_id,    /* spir-v name of structure type*/
    const u32           index,      /* member index                 */
    const parser_state_type
                       *context     /* parser state                 */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
SpvDecoration           decoration; /* decoration type              */
bool                    found;      /* did we find it?              */
u32                     member_index;
                                    /* structure member index       */
SpvOp                   opcode;     /* instruction's opcode         */
parser_state_type       parser;     /* rewound parser               */
u32                     ret;        /* return byte offset           */
u32                     struct_type_id;
                                    /* type ID of structure         */

ret = 0;
found = FALSE;
rewind_parser( SECTION_NAME_ANNOTATIONS, context, &parser );
while( next_instruction( &parser ) )
    {
    /*------------------------------------------------------
    Read the instruction header
    ------------------------------------------------------*/
    if( !read_instruction_header( &parser, &opcode, NULL ) )
        {
        return( 0 );
        }

    /*------------------------------------------------------
    Looking for members
    ------------------------------------------------------*/
    if( opcode != SpvOpMemberDecorate )
        {
        continue;
        }

    /*------------------------------------------------------
    Of a particular structure type
    ------------------------------------------------------*/
    struct_type_id = read_word( &parser );
    if( type_id != struct_type_id )
        {
        continue;
        }

    /*------------------------------------------------------
    With a particular index
    ------------------------------------------------------*/
    member_index = read_word( &parser );
    if( index != member_index )
        {
        continue;
        }

    /*------------------------------------------------------
    And we want the offset decoration
    ------------------------------------------------------*/
    decoration = (SpvDecoration)read_word( &parser );
    if( decoration != SpvDecorationOffset )
        {
        continue;
        }

    /*------------------------------------------------------
    Found it
    ------------------------------------------------------*/
    found = TRUE;
    ret = read_word( &parser );
    break;
    }

/*----------------------------------------------------------
Return the offset
----------------------------------------------------------*/
(void)found;
debug_assert( found );

return( ret );

}   /* get_member_offset() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       get_type_opcode
*
*   DESCRIPTION:
*       Get the type opcode of the given type id.
*
*********************************************************************/

static SpvOp get_type_opcode
    (
    const u32           type_id,    /* spir-v name of type          */
    const parser_state_type
                       *context     /* parser state                 */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
bool                    found;      /* did we find it?              */
SpvOp                   opcode;     /* instruction's opcode         */
parser_state_type       parser;     /* rewound parser               */
SpvOp                   ret;        /* return type opcode           */
u32                     result_id;  /* instruction's type id        */

ret = SpvOpMax;
found = FALSE;
rewind_parser( SECTION_NAME_TYPES_VARIABLES_CONSTANTS, context, &parser );
while( next_instruction( &parser ) )
    {
    /*------------------------------------------------------
    Read the instruction header
    ------------------------------------------------------*/
    if( !read_instruction_header( &parser, &opcode, NULL ) )
        {
        return( SpvOpNop );
        }

    /*------------------------------------------------------
    Check if we reached the end of the types without finding
    the request
    ------------------------------------------------------*/
    if( opcode == SpvOpFunction )
        {
        debug_assert_always();
        return( SpvOpNop );
        }

    /*------------------------------------------------------
    We only care about types
    ------------------------------------------------------*/
    if( !is_type( opcode ) )
        {
        continue;
        }

    /*------------------------------------------------------
    Looking for a particular type
    ------------------------------------------------------*/
    result_id = read_word( &parser );
    if( type_id != result_id )
        {
        continue;
        }

    /*------------------------------------------------------
    Found it
    ------------------------------------------------------*/
    found = TRUE;
    ret = opcode;
    break;
    }

/*----------------------------------------------------------
Return the offset
----------------------------------------------------------*/
(void)found;
debug_assert( found );

return( ret );

}   /* get_type_opcode() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       get_type_size
*
*   DESCRIPTION:
*       Discover a type's size.
*
*********************************************************************/

static u32 get_type_size
    (
    const u32           type_id,    /* spir-v name of the type      */
    const parse_flags_type
                        flags,      /* parsing flags                */
    const parser_state_type
                       *context     /* parser state                 */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     i;          /* loop counter                 */
u32                     member_cnt; /* number of type members       */
u32                     member_largest_offset;
                                    /* largest memer offset seen    */
u32                     member_last;/* last struct member in memory */
u32                     member_offset;
                                    /* byte offset to member start  */
u32                     member_size;/* byte size of member          */
u32                     member_type_id;
                                    /* member's type id             */
SpvOp                   opcode;     /* instruction's opcode         */
parser_state_type       parser;     /* rewound parser               */
u32                     result_id;  /* instruction's type id        */
u32                     ret;        /* return type size             */
parser_state_type       saved;      /* saved position of parser     */
type_type               type_working;
                                    /* working type                 */
u16                     word_cnt;   /* instruction word count       */

rewind_parser( SECTION_NAME_TYPES_VARIABLES_CONSTANTS, context, &parser );

ret = 0;
while( next_instruction( &parser ) )
    {
    /*------------------------------------------------------
    Read the instruction header
    ------------------------------------------------------*/
    if( !read_instruction_header( &parser, &opcode, &word_cnt ) )
        {
        return( 0 );
        }

    /*------------------------------------------------------
    Check if we reached the end of the types without finding
    the request
    ------------------------------------------------------*/
    if( opcode == SpvOpFunction )
        {
        debug_assert_always();
        return( 0 );
        }

    /*------------------------------------------------------
    We only care about types
    ------------------------------------------------------*/
    if( !is_type( opcode ) )
        {
        continue;
        }

    /*------------------------------------------------------
    All type start with their type id
    ------------------------------------------------------*/
    result_id = read_word( &parser );
    if( result_id != type_id )
        {
        /*--------------------------------------------------
        Not the type we are looking for
        --------------------------------------------------*/
        continue;
        }

    /*------------------------------------------------------
    Process the type
    ------------------------------------------------------*/
    clr_struct( &type_working );
    switch( opcode )
        {
        /*--------------------------------------------------
        Structure
        --------------------------------------------------*/
        case SpvOpTypeStruct:
            member_cnt = word_cnt - 2; /* instruction header and result id */

            /*----------------------------------------------
            We need to find the member with the largest
            offset (the last member in member in memory)
            ----------------------------------------------*/
            member_last = 0;
            member_largest_offset = 0;
            saved = parser;
            for( i = 0; i < member_cnt; i++ )
                {
                member_offset = get_member_offset( type_id, i, &parser );
                if( member_offset > member_largest_offset )
                    {
                    member_largest_offset = member_offset;
                    member_last = i;
                    }
                }

            /*----------------------------------------------
            Now calculate the size of the structure by the
            last member in memory
            ----------------------------------------------*/
            parser = saved;
            for( i = 0; i < member_last; i++ )
                {
                /*------------------------------------------
                Fast-forward
                ------------------------------------------*/
                (void)read_word( &parser );
                }

            member_type_id = read_word( &parser );
            member_size    = get_type_size( member_type_id, PARSE_FLAGS_NONE, &parser );
            member_offset  = member_largest_offset;

            /* size must include alignment padding */
            ret = member_offset + member_size;
            if( !test_bits( flags, PARSE_FLAGS_PARENT_RUNTIME_ARRAY ) )
                {
                ret = ceiling( ret, SPIRV_ALIGNMENT );
                }
            break;

        /*--------------------------------------------------
        Void/Sampler
        --------------------------------------------------*/
        case SpvOpTypeVoid:
        case SpvOpTypeSampler:
            ret = 0;
            break;

        /*--------------------------------------------------
        Booleans
        --------------------------------------------------*/
        case SpvOpTypeBool:
            /* bools are stored as 4 bytes */
            ret = sizeof( u32 );
            break;

        /*--------------------------------------------------
        Integers/Floats
        --------------------------------------------------*/
        case SpvOpTypeInt:
        case SpvOpTypeFloat:
            type_working.as_int.bit_width  = read_word( &parser );

            /* convert from bits to bytes */
            debug_assert( type_working.as_int.bit_width % CHAR_BIT == 0 );
            ret = type_working.as_int.bit_width / CHAR_BIT;
            break;

        /*--------------------------------------------------
        Vectors/Matrix
        --------------------------------------------------*/
        case SpvOpTypeVector: // TODO <MPA> REVISIT VECTOR for offset
        case SpvOpTypeMatrix: // TODO <MPA> REVISIT MATRIX for stride
            /* size is the component size by the number of components */
            type_working.as_vector.type_id    = read_word( &parser );
            type_working.as_vector.components = read_word( &parser );
            ret = type_working.as_vector.components * get_type_size( type_working.as_vector.type_id, PARSE_FLAGS_NONE, &parser );
            break;

        /*--------------------------------------------------
        Arrays
        --------------------------------------------------*/
        case SpvOpTypeArray:
            type_working.as_array.type_id = read_word( &parser );
            type_working.as_array.length  = read_word( &parser );

            ret = get_array_length( type_working.as_array.length, &parser ) * get_array_stride( type_id, &parser );
            break;

        /*--------------------------------------------------
        Runtime arrays
        --------------------------------------------------*/
        case SpvOpTypeRuntimeArray:
            type_working.as_runtime_array.type_id = read_word( &parser );
            ret = 0;
            if( SpvOpTypeStruct == get_type_opcode( type_working.as_runtime_array.type_id, &parser ) )
                {
                ret = get_type_size( type_working.as_runtime_array.type_id, PARSE_FLAGS_PARENT_RUNTIME_ARRAY, &parser );
                }
            break;

        /*--------------------------------------------------
        Pointers
        --------------------------------------------------*/
        case SpvOpTypePointer:
            /* get the size of the type pointed to */
            type_working.as_pointer.storage_class = (SpvStorageClass)read_word( &parser );
            type_working.as_pointer.type_id       = read_word( &parser );
            ret = get_type_size( type_working.as_pointer.type_id, PARSE_FLAGS_NONE, &parser );
            break;

        /*--------------------------------------------------
        We should be handling all the types
        --------------------------------------------------*/
        default:
            debug_assert( opcode == SpvOpTypeOpaque
                         || opcode == SpvOpTypeFunction
                         || opcode == SpvOpTypeEvent
                         || opcode == SpvOpTypeDeviceEvent
                         || opcode == SpvOpTypeReserveId
                         || opcode == SpvOpTypeQueue
                         || opcode == SpvOpTypePipe
                         || opcode == SpvOpTypeImage
                         || opcode == SpvOpTypeSampledImage
                         || opcode == SpvOpTypeForwardPointer );
            continue;
        }

    break;
    }

return( ret );

}   /* get_type_size() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       get_variable_name_string
*
*   DESCRIPTION:
*       Get the name string for the requested varible, given its
*       spir-v name.
*
*********************************************************************/

static const char * get_variable_name_string
    (
    const u32           variable_id,/* variable name                */
    const u32           variable_type_id,
                                    /* type id of variable          */
    const parser_state_type
                       *context,    /* parser state                 */
    u32                *out_length  /* output str length (inc null) */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     name;       /* hash name                    */
SpvOp                   opcode;     /* instruction's opcode         */
parser_state_type       parser;     /* rewound parser               */
u32                     result_id;  /* instruction's name id        */
const char             *ret;        /* return string pointer        */
u32                     search_id;  /* result id to search for      */

ret = NULL;
search_id = variable_id;

/*----------------------------------------------------------
Find the name of the variable
----------------------------------------------------------*/
name = 0;
rewind_parser( SECTION_NAME_DEBUG_INFORMATION, context, &parser );

while( next_instruction( &parser ) )
    {
    /*------------------------------------------------------
    Read the instruction header
    ------------------------------------------------------*/
    if( !read_instruction_header( &parser, &opcode, NULL ) )
        {
        break;
        }

    /*------------------------------------------------------
    Looking for non-members
    ------------------------------------------------------*/
    if( opcode != SpvOpName )
        {
        continue;
        }

    /*------------------------------------------------------
    That matches our given id
    ------------------------------------------------------*/
    result_id = read_word( &parser );
    if( search_id != result_id )
        {
        continue;
        }

    /*------------------------------------------------------
    Found it
    ------------------------------------------------------*/
    ret = read_string( &parser, out_length );
    if( !ret
     || *out_length < 1 )
        {
        /*--------------------------------------------------
        Unexpected string read error
        --------------------------------------------------*/
        debug_assert_always();
        return( NULL );
        }

    break;
    }

if( *out_length > 1 )
    {
    return( ret );
    }

/*----------------------------------------------------------
Unwind pointers
----------------------------------------------------------*/
if( get_type_opcode( variable_type_id, context ) == SpvOpTypePointer )
    {
    rewind_parser( SECTION_NAME_TYPES_VARIABLES_CONSTANTS, context, &parser );

    search_id = variable_type_id;
    while( next_instruction( &parser ) )
        {
        /*--------------------------------------------------
        Read the instruction header
        --------------------------------------------------*/
        if( !read_instruction_header( &parser, &opcode, NULL ) )
            {
            return( 0 );
            }

        /*--------------------------------------------------
        Check if we reached the end of the types without
        finding the request
        --------------------------------------------------*/
        if( opcode == SpvOpFunction )
            {
            debug_assert_always();
            return( 0 );
            }

        /*--------------------------------------------------
        We only care about types
        --------------------------------------------------*/
        if( !is_type( opcode ) )
            {
            continue;
            }

        /*--------------------------------------------------
        Looking for a particular type
        --------------------------------------------------*/
        result_id = read_word( &parser );
        if( search_id != result_id )
            {
            continue;
            }

        if( opcode != SpvOpTypePointer )
            {
            debug_assert_always();
            return( NULL );
            }

        read_word( &parser );/* storage class */
        search_id = read_word( &parser );
        break;
        }
    }

/*----------------------------------------------------------
Find the name of the variable
----------------------------------------------------------*/
name = 0;
rewind_parser( SECTION_NAME_DEBUG_INFORMATION, context, &parser );

while( next_instruction( &parser ) )
    {
    /*------------------------------------------------------
    Read the instruction header
    ------------------------------------------------------*/
    if( !read_instruction_header( &parser, &opcode, NULL ) )
        {
        break;
        }

    /*------------------------------------------------------
    Looking for non-members
    ------------------------------------------------------*/
    if( opcode != SpvOpName )
        {
        continue;
        }

    /*------------------------------------------------------
    That matches our given id
    ------------------------------------------------------*/
    result_id = read_word( &parser );
    if( search_id != result_id )
        {
        continue;
        }

    /*------------------------------------------------------
    Found it
    ------------------------------------------------------*/
    ret = read_string( &parser, out_length );
    if( !ret
     || *out_length < 1 )
        {
        /*--------------------------------------------------
        Unexpected string read error
        --------------------------------------------------*/
        debug_assert_always();
        return( NULL );
        }

    break;
    }

return( ret );

}   /* get_variable_name_string() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       initialize_parser
*
*   DESCRIPTION:
*       Parse the given descriptor binding.
*
*********************************************************************/

static void initialize_parser
    (
    const u32          *binary,     /* spir-v bytecode              */
    const u32           binary_sz,  /* spir-v bytecode byte size    */
    VKN_shader_reflect_descriptor_binding_type
                       *descriptor_bindings,
                                    /* descriptor set bindings      */
    u32                *descriptor_binding_cnt,
                                    /* descriptor set binding count */
    VKN_shader_reflect_push_constant_type
                       *push_constants,
                                    /* push constants               */
    u32                *push_constant_cnt,
                                    /* push constant count          */
    parser_common_type *common,     /* common parser state          */
    parser_state_type  *parser      /* parser state to initialize   */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
int                     i;          /* loop counter                 */

/*----------------------------------------------------------
Instance state
----------------------------------------------------------*/
clr_struct( parser );
parser->common = common;

/*----------------------------------------------------------
Common state
----------------------------------------------------------*/
clr_struct( common );

parser->common->should_endian = INVALID_ENDIAN;
parser->common->binary        = binary;
parser->common->word_cnt      = binary_sz / sizeof( *binary );
debug_assert( binary_sz % sizeof( *binary ) == 0 );

parser->common->out_bindings  = descriptor_bindings;
parser->common->out_constants = push_constants;

for( i = 0; i < cnt_of_array( parser->common->section_start ); i++ )
    {
    /* section not found */
    parser->common->section_start[ i ] = parser->common->word_cnt;
    }

if( descriptor_binding_cnt )
    {
    parser->common->binding_capacity = *descriptor_binding_cnt;
    }

if( push_constant_cnt )
    {
    parser->common->constant_capacity = *push_constant_cnt;
    }

}   /* initialize_parser() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       is_section_types_variable_constants
*
*   DESCRIPTION:
*       Given an opcode, is the instruction found in the types/
*       variables/constants section?
*
*********************************************************************/

static bool is_section_types_variable_constants
    (
    const SpvOp         opcode,     /* instruction opcode           */
    const SpvStorageClass           /* instruction storage class    */
                        storage_class
    )
{
return( is_type( opcode )
     || is_constant( opcode )
     || ( opcode == SpvOpVariable
       && storage_class != SpvStorageClassFunction ) );

}   /* is_section_types_variable_constants() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       parse_descriptor_binding
*
*   DESCRIPTION:
*       Parse the given descriptor binding.
*
*********************************************************************/

static bool parse_descriptor_binding
    (
    const variable_type
                       *variable,   /* variable properties          */
    const VKN_shader_reflect_spirv_override_type
                       *overrides,  /* override descriptions        */
    const u32           override_cnt,
                                    /* number of overrides          */
    parser_state_type  *parser      /* parser state                 */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     length;     /* string length (inc NULL)     */
VKN_shader_reflect_descriptor_binding_type
                       *out_binding;/* output descriptor binding    */
const char             *str;        /* string                       */

/*----------------------------------------------------------
Tally
----------------------------------------------------------*/
parser->common->binding_cnt++;
if( !parser->common->out_bindings )
    {
    /*------------------------------------------------------
    Only counting
    ------------------------------------------------------*/
    return( TRUE );
    }

/*----------------------------------------------------------
Parse
----------------------------------------------------------*/
if( parser->common->binding_cnt > parser->common->binding_capacity )
    {
    debug_assert_always();
    return( FALSE );
    }

str           = get_variable_name_string( variable->result_id, variable->type_id, parser, &length );
out_binding   = &parser->common->out_bindings[ parser->common->binding_cnt - 1 ];

clr_struct( out_binding );
if( str
 && length > 1 )
    {
    out_binding->name = VKN_hash_blob( VKN_HASH_SEED, str, length );
    }

out_binding->set     = get_binding_set_index( variable->result_id, parser );
out_binding->binding = get_descriptor_binding_index( variable->result_id, parser );
out_binding->count   = get_descriptor_binding_descriptor_count( variable->type_id, parser );

if( !get_binding_override_descriptor_type( out_binding->name, overrides, override_cnt, parser, &out_binding->kind ) )
    {
    out_binding->kind = get_descriptor_binding_descriptor_type( variable->type_id, parser );
    }

return( TRUE );

}   /* parse_descriptor_binding() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       parse_header
*
*   DESCRIPTION:
*       Parse the binary header block.
*
*********************************************************************/

static bool parse_header
    (
    parser_state_type  *parser,     /* parser state                 */
    header_type        *header      /* output parsed header         */
    )
{
/*----------------------------------------------------------
Local literals
----------------------------------------------------------*/
#define HEADER_BLOCK_WORD_CNT       ( 5 )

/*----------------------------------------------------------
Validate
----------------------------------------------------------*/
debug_assert( parser->caret == 0 );
parser->caret = 0;
if( parser->common->word_cnt < HEADER_BLOCK_WORD_CNT )
    {
    debug_assert_always();
    return( FALSE );
    }

/*----------------------------------------------------------
Read
----------------------------------------------------------*/
/* magic word */
header->magic = read_word( parser );
parser->common->should_endian = needs_endian( header->magic );
header->magic = correct_endian( parser, header->magic );

/* spir-v version */
header->version = read_word( parser );
debug_assert( header->version <= SPV_VERSION );

/* generator ID */
header->generator = read_word( parser );

/* maximum opcode ID */
header->bound = read_word( parser );

/* reserved word */
header->reserved = read_word( parser );

return( TRUE );

#undef HEADER_BLOCK_WORD_CNT
}   /* parse_header() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       parse_layout
*
*   DESCRIPTION:
*       Parse the program's layout variables.
*
*********************************************************************/

static bool parse_layout
    (
    const parser_state_type
                       *context,    /* parser state                 */
    const VKN_shader_reflect_spirv_override_type
                       *overrides,  /* override descriptions        */
    const u32           override_cnt/* number of overrides          */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
SpvOp                   opcode;     /* instruction's opcode         */
parser_state_type       parser;     /* parser state                 */
variable_type           variable;   /* variable properties          */

rewind_parser( SECTION_NAME_TYPES_VARIABLES_CONSTANTS, context, &parser );
while( next_instruction( &parser ) )
    {
    /*------------------------------------------------------
    Read the instruction header
    ------------------------------------------------------*/
    if( !read_instruction_header( &parser, &opcode, NULL ) )
        {
        return( FALSE );
        }

    /*------------------------------------------------------
    Read the layout
    ------------------------------------------------------*/
    switch( opcode )
        {
        /*--------------------------------------------------
        Variables
        --------------------------------------------------*/
        case SpvOpVariable:
            clr_struct( &variable );
            variable.type_id       = read_word( &parser );
            variable.result_id     = read_word( &parser );
            variable.storage_class = (SpvStorageClass)read_word( &parser );

            if( is_push_constant( opcode, variable.storage_class ) )
                {
                parse_push_constant( &variable, &parser );
                }
            else if( is_descriptor_binding( opcode, variable.storage_class ) )
                {
                parse_descriptor_binding( &variable, overrides, override_cnt, &parser );
                }

            break;

        /*--------------------------------------------------
        Unprocessed
        --------------------------------------------------*/
        default:
            break;
        }
    }

return( TRUE );

}   /* parse_layout() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       parse_push_constant
*
*   DESCRIPTION:
*       Parse the given push constant.
*
*********************************************************************/

static bool parse_push_constant
    (
    const variable_type
                       *variable,   /* variable properties          */
    parser_state_type  *parser      /* parser state                 */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     length;     /* string length (inc NULL)     */
VKN_shader_reflect_push_constant_type
                       *out;        /* output push constant         */
const char             *str;        /* string                       */

/*----------------------------------------------------------
Tally
----------------------------------------------------------*/
parser->common->constant_cnt++;
if( !parser->common->out_constants )
    {
    /*------------------------------------------------------
    Only counting
    ------------------------------------------------------*/
    return( TRUE );
    }

/*----------------------------------------------------------
Parse
----------------------------------------------------------*/
if( parser->common->constant_cnt > parser->common->constant_capacity )
    {
    debug_assert_always();
    return( FALSE );
    }

str = get_variable_name_string( variable->result_id, variable->type_id, parser, &length );
out = &parser->common->out_constants[ parser->common->constant_cnt - 1 ];

clr_struct( out );
if( str
 && length > 1 )
    {
    out->name = VKN_hash_blob( VKN_HASH_SEED, str, length );
    }

out->constant.offset = get_descriptor_type_offset( variable->type_id, parser );
out->constant.size   = get_type_size( variable->type_id, PARSE_FLAGS_NONE, parser );

return( TRUE );

}   /* parse_push_constant() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       parse_sections
*
*   DESCRIPTION:
*       Discover the section boundaries.
*
*********************************************************************/

static void parse_sections
    (
    const parser_state_type
                       *context     /* parser state                 */
    )
{
/*----------------------------------------------------------
Local literals
----------------------------------------------------------*/
#define STORAGE_CLASS_NA            ( SpvStorageClassMax )

/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
SpvOp                  opcode;      /* instruction's opcode         */
parser_state_type      parser;      /* constant value               */
section_name_type      section;     /* section name                 */
u32                    start;       /* instruction start            */
SpvStorageClass        storage_class;
                                    /* instruction storage class    */

rewind_parser( SECTION_NAME_INSTRUCTIONS, context, &parser );
while( next_instruction( &parser ) )
    {
    start = parser.caret;
    if( !read_instruction_header( &parser, &opcode, NULL ) )
        {
        return;
        }

    /*------------------------------------------------------
    If this is a variable, we need its storage class
    ------------------------------------------------------*/
    storage_class = STORAGE_CLASS_NA;
    if( opcode == SpvOpVariable )
        {
        (void)read_word( &parser );/* result type */
        (void)read_word( &parser );/* result id */
        storage_class = (SpvStorageClass)read_word( &parser );
        }

    /*------------------------------------------------------
    Determine the section
    ------------------------------------------------------*/
    if( is_section_debug_info( opcode ) )
        {
        section = SECTION_NAME_DEBUG_INFORMATION;
        }
    else if( is_section_annotations( opcode ) )
        {
        section = SECTION_NAME_ANNOTATIONS;
        }
    else if( is_section_types_variable_constants( opcode, storage_class ) )
        {
        section = SECTION_NAME_TYPES_VARIABLES_CONSTANTS;
        }
    else if( is_section_functions( opcode ) )
        {
        section = SECTION_NAME_FUNCTIONS;
        }
    else
        {
        continue;
        }

    /*------------------------------------------------------
    Store the first instruction of the section
    ------------------------------------------------------*/
    parser.common->section_start[ section ] = clamp_max_u32( start, parser.common->section_start[ section ] );

    /*------------------------------------------------------
    Early out if we are done
    ------------------------------------------------------*/
    if( parser.common->section_start[ SECTION_NAME_FUNCTIONS ] < parser.common->word_cnt )
        {
        break;
        }
    }

#undef STORAGE_CLASS_NA
}   /* parse_sections() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       read_instruction_header
*
*   DESCRIPTION:
*       Read the opcode and instruction word count.
*
*********************************************************************/

static bool read_instruction_header
    (
    parser_state_type  *parser,     /* parser state                 */
    SpvOp              *out_opcode, /* instruction's opcode         */
    u16                *out_word_cnt/* instructions's word count    */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     header;     /* instruction header           */
SpvOp                   opcode;     /* instruction opcode           */
u16                     word_cnt;   /* instruction word count       */

/*----------------------------------------------------------
Read and decipher the instruction header
----------------------------------------------------------*/
header = read_word( parser );

opcode   = (SpvOp)unpack_word(  header, UNPACK_WORD_MASK_U16, UNPACK_WORD_SHIFT_NIBBLE_LOW );
word_cnt = (u16)unpack_word( header, UNPACK_WORD_MASK_U16, UNPACK_WORD_SHIFT_NIBBLE_HIGH );

*out_opcode = opcode;

if( out_word_cnt )
    {
    *out_word_cnt = word_cnt;
    }

parser->remain_instruction_word = word_cnt - 1;

/*----------------------------------------------------------
Verify the instructions operands are not off the end of the
binary
----------------------------------------------------------*/
if( parser->caret + ( word_cnt - 1 ) > parser->common->word_cnt )
    {
    debug_assert_always();
    return( FALSE );
    }

return( TRUE );

}   /* read_instruction_header() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       read_string
*
*   DESCRIPTION:
*       Read a string from the binary.
*
*********************************************************************/

static const char * read_string
    (
    parser_state_type  *parser,     /* parser state                 */
    u32                *length      /* length of string (inc NULL)  */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
bool                    found;      /* did we find the null term?   */
u32                     i;          /* loop counter                 */
u32                     len;        /* string length so far         */
const char             *ret;        /* return string pointer        */

for( ret = NULL, len = 0, found = FALSE; !found && parser->caret < parser->common->word_cnt && parser->remain_instruction_word; parser->caret++ )
    {
    if( !ret )
        {
        ret = (const char*)&parser->common->binary[ parser->caret ];
        }

    /*------------------------------------------------------
    Scan the next word for null termination
    ------------------------------------------------------*/
    for( i = 0; i < sizeof(u32); i++ )
        {
        len++;
        if( !ret[ len - 1 ] )
            {
            found = TRUE;
            break;
            }
        }

    parser->remain_instruction_word--;
    }

/*----------------------------------------------------------
We should have found the NULL terminator
----------------------------------------------------------*/
if( !found )
    {
    debug_assert_always();
    return( NULL );
    }

/*----------------------------------------------------------
Output
----------------------------------------------------------*/
*length = len;

return( ret );

}   /* read_string() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       read_word
*
*   DESCRIPTION:
*       Read an endian corrected word from the binary and advance the
*       caret.
*
*********************************************************************/

static u32 read_word
    (
    parser_state_type  *parser      /* parser state                 */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
u32                     ret;        /* return word                  */

if( parser->caret >= parser->common->word_cnt )
    {
    debug_assert_always();
    return( 0 );
    }

ret = parser->common->binary[ parser->caret++ ];
if( parser->common->should_endian != INVALID_ENDIAN )
    {
    ret = correct_endian( parser, ret );
    }

parser->remain_instruction_word--;

return( ret );

}   /* read_word() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       remove_duplicate_descriptor_bindings
*
*   DESCRIPTION:
*       Remove bindings with matching set and binding indices.
*
*       Note that duplicates will cause the output binding count to
*       change between queries (parameter out_bindings=NULL) and
*       binding fetches.
*
*********************************************************************/

static void remove_duplicate_descriptor_bindings
    (
    parser_state_type  *parser      /* parser state                 */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_shader_reflect_descriptor_binding_type
                       *a;          /* first swap item              */
VKN_shader_reflect_descriptor_binding_type
                       *b;          /* second swap item             */
u32                     i;          /* loop counter                 */

/*----------------------------------------------------------
Check we have work to do
----------------------------------------------------------*/
if( !parser->common->out_bindings
 || parser->common->binding_cnt < 2 )
    {
    return;
    }

/*----------------------------------------------------------
Bindings are sorted set index (major) and binding index
(minor).  We can detect and remove duplicates in a single
pass.
----------------------------------------------------------*/
for( i = 1; i < parser->common->binding_cnt; i++ )
    {
    a = &parser->common->out_bindings[ i - 1 ];
    b = &parser->common->out_bindings[ i - 0 ];

    /*------------------------------------------------------
    Detect
    ------------------------------------------------------*/
    if( a->set < b->set
     || a->binding < b->binding )
        {
        continue;
        }

    debug_assert( a->set == b->set );
    debug_assert( a->binding == b->binding );
    debug_assert( a->kind == b->kind );
    debug_assert( a->count == b->count );

    /*------------------------------------------------------
    Shift-pack duplicate
    ------------------------------------------------------*/
    *b = *a;
    parser->common->binding_cnt--;
    i--;
    }

}   /* remove_duplicate_descriptor_bindings() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       rewind_parser
*
*   DESCRIPTION:
*       Begin parsing from the start of the instructions.
*
*********************************************************************/

static void rewind_parser
    (
    const section_name_type
                        section,    /* desired parser section       */
    const parser_state_type
                       *context,    /* parser state                 */
    parser_state_type  *parser      /* rewound parser               */
    )
{
/*----------------------------------------------------------
Initialize the new context
----------------------------------------------------------*/
*parser = *context;

parser->section                 = section;
parser->caret                   = context->common->section_start[ section ];
parser->remain_instruction_word = 0;

parser->section_end = context->common->word_cnt;
if( parser->section != SECTION_NAME_INSTRUCTIONS
 && parser->section != SECTION_NAME_FUNCTIONS )
    {
    parser->section_end = context->common->section_start[ section + 1 ];
    }

}   /* rewind_parser() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       sort_descriptor_bindings
*
*   DESCRIPTION:
*       Sort the descriptor bindings by their set index (major) and
*       binding index (minor).
*
*********************************************************************/

static void sort_descriptor_bindings
    (
    parser_state_type  *parser      /* parser state                 */
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
VKN_shader_reflect_descriptor_binding_type
                       *a;          /* test variable                */
VKN_shader_reflect_descriptor_binding_type
                       *b;          /* test variable                */
u32                     i;          /* loop counter                 */
u32                     j;          /* loop counter                 */
VKN_shader_reflect_descriptor_binding_type
                        temp;       /* swap variable                */

/*----------------------------------------------------------
Check we have work to do
----------------------------------------------------------*/
if( !parser->common->out_bindings
 || parser->common->binding_cnt < 2 )
    {
    return;
    }

/*----------------------------------------------------------
Bubble sort
----------------------------------------------------------*/
for( i = 1; i < parser->common->binding_cnt; i++ )
    {
    for( j = 0; j < parser->common->binding_cnt - i; j++ )
        {
        a = &parser->common->out_bindings[ j + 0 ];
        b = &parser->common->out_bindings[ j + 1 ];

        if( a->set < b->set
         || ( a->set == b->set
           && a->binding < b->binding ) )
            {
            continue;
            }

        /*--------------------------------------------------
        Swap
        --------------------------------------------------*/
        temp = *a;
        *a = *b;
        *b = temp;
        }
    }

}   /* sort_descriptor_bindings() */
