
import json
import os
import subprocess


FILE_CONTENT_TEMPLATE = """
/*********************************************************************
*
*   This file is auto-generated.  DO NOT MODIFY IT DIRECTLY.
*   See 'vkn_compile_shaders.py' for more details.
*
*********************************************************************/

#pragma once

#include "Global.hpp"

#include "VknShaderParamTypes.hpp"

{6}

{7}{8}{9}

{0}{1}{2}{3}{4}{5}{10}

"""

ENUM_TEMPLATE = """
typedef enum
    {{
{0}
    }} {1}_name_type;
"""

UNIFORM_PARAMETER_TEMPLATE = """
typedef struct
    {{
    VKN_shader_param_vector_name_type
                        name;       /* shader vector param name     */
    u8                  num_floats; /* floats read of parameter     */    
    }} {0};
"""

UNIFORM_TEMPLATE = """
typedef struct
    {{
    const char         *str_name;   /* uniform string name          */
    const {1}
                       *vectors;    /* shader vector parameters     */
    u32                 vector_cnt; /* number of vector parameters  */
    const VKN_shader_param_image_name_type
                       *image;      /* shader image parameter       */
    }} {0};
"""

UNIFORM_SHADER_TEMPLATE = """
static const {0} {1}[] =
    {{
{2}
    }};
"""

IMAGES_SHADER_TEMPLATE = """
static const VKN_shader_param_image_name_type {0}[] =
    {{
{1}
    }};
"""

TABLE_ELEMENT_TEMPLATE = """
typedef struct
    {{
    const u32          *bytecode;   /* shader bytecode              */
    u32                 size;       /* bytecode size                */    
    const {0}
                       *uniforms;   /* shader uniforms              */
    u32                 uniform_cnt;/* number of shader uniforms    */
    }} {1};
"""

TABLE_TEMPLATE = """
/*--------------------------------------------------------
    Code and uniforms
--------------------------------------------------------*/
static const {0} {1}[] =
    {{
{2}
    }};
"""

VERTEX_SHADER_TEMPLATE = """
/*--------------------------------------------------------
    Vertex Shaders
--------------------------------------------------------*/
{0}
"""

TESS_CONTROL_TEMPLATE = """
/*--------------------------------------------------------
    Tessellation Control Shaders
--------------------------------------------------------*/
{0}
"""

TESS_EVAL_TEMPLATE = """
/*--------------------------------------------------------
    Tessellation Evaluation Shaders
--------------------------------------------------------*/
{0}
"""

GEOMETRY_TEMPLATE = """
/*--------------------------------------------------------
    Geometry Shaders
--------------------------------------------------------*/
{0}
"""

FRAGMENT_TEMPLATE = """
/*--------------------------------------------------------
    Fragment Shaders
--------------------------------------------------------*/
{0}
"""

COMPUTE_TEMPLATE = """
/*--------------------------------------------------------
    Compute Shaders
--------------------------------------------------------*/
{0}
"""

PER_SHADER_TEMPLATE = """
/*********************************************************
*
*   SHADER NAME:
*       {0}
*
*********************************************************/

static const u32 {1}[] =
{2};
"""

GLSLC_FILENAME = "glslc.exe"
MAX_LINE_LENGTH = 170

class JsonMalformedEntryException(Exception):
    filename = None
    hint = "Unexpected stage name.  Found {0}.  Expected 'vertex', 'tessellation_control', 'tessellation_evaluation', 'geometry', 'fragment', or 'compute'."

class JsonNotFoundException(Exception):
    filename_and_path = None
		
class ShaderFailedCompileException(Exception):
    filename_and_path = None
    error_str = None

class GlslcNotFoundException(Exception):
    filename_and_path = None
    		
class ShaderFileNotFoundException(Exception):
    filenames = list()

def make_shader_tag( prefix, name, split_filename ):
    return "{0}_{1}_{2}_{3}".format( prefix, name, split_filename[ 0 ].upper(), split_filename[ 1 ].replace( '.', '' ).upper() )

class ShaderCompiler:
    def __init__( self, glslc_path, shaders_root, bytecode_prefix ):
        self.glslc_path = glslc_path
        self.shaders_root = shaders_root
        self.bytecode_prefix = bytecode_prefix

    def compile_shader( self, filename ):
        cmd = " ".join( [ self.glslc_path, filename, " -o -" ] )
        p = subprocess.Popen( cmd, stdout=subprocess.PIPE )
        output = p.communicate()[ 0 ]

        ret = p.returncode
        if ret != 0:
            ex = ShaderFailedCompileException()
            ex.filename = filename
            ex.error_str = output.decode()
            raise ex
        
        bytecode = []
        for a, b, c, d in ( output[ i:i+4 ] for i in range( 0, 4 * int( len( output )/4 ), 4 ) ):
            bytecode.append( int( ( a << 0 ) + ( b << 8 ) + ( c << 16 ) + ( d << 24 ) ) ) # little-endian

        str_bytecode = list( ", ".join( "0x{:08x}".format( i ) for i in bytecode ) )
        char_cnt = 0
        for i in range( 0, len( str_bytecode ) ):
            if str_bytecode[ i ] == " " and char_cnt > MAX_LINE_LENGTH:
                str_bytecode[ i ] = "\n\t"
                char_cnt = 0
            else:
                char_cnt = char_cnt + 1

        str_bytecode = "\t{{\n\t{0}\n\t}}".format( "".join( str_bytecode ) )

        shader_filename = os.path.basename( filename ).lower()
        split_filename = os.path.splitext( shader_filename )
        split_filename[ 1 ].replace( '.', '' ).upper() + '_' + split_filename[ 0 ].upper()

        bytecode_tag = make_shader_tag( self.bytecode_prefix, "BYTECODE", split_filename )
        template_str = PER_SHADER_TEMPLATE.format( shader_filename, bytecode_tag, str_bytecode ).lstrip().rstrip()

        return template_str

    def make_uniform_arrays( self, entry, uniform_param_type_name, uniform_type_name ):
        if not "uniforms" in entry:
            return "", "", "", ( "NULL", 0 )

        # gather the uniforms
        image_list = []
        param_list = []
        uniform_list = []
        for uniform in entry[ "uniforms" ]:
            if not "uniform_name" in uniform:
                continue
            elif "shader_params" in uniform:
                this_params = []
                for param in uniform[ "shader_params" ]:
                    if not "name" in param or not "floats" in param:
                        print( "JSON error in shader_params" )
                        continue
                    this_params.append( ( param[ "name" ], int( param[ "floats" ] ) ) )

                this_tuple = ( uniform[ "uniform_name" ], len( param_list ), len( this_params ), -1 )
                uniform_list.append( this_tuple )
                param_list.extend( this_params )
            elif "image" in uniform:
                uniform_list.append( ( uniform[ "uniform_name" ], -1, 0, len( image_list ) ) )
                image_list.append( "\t" + uniform[ "image" ] )
            else:
                continue

        # create the split filename to use for tags
        filename = os.path.basename( entry[ "filename" ] )
        shader_filename = os.path.basename( filename ).lower()
        split_filename = os.path.splitext( shader_filename )
        split_filename[ 1 ].replace( '.', '' ).upper() + '_' + split_filename[ 0 ].upper()

        params_str = ""
        params_variable_name = make_shader_tag( self.bytecode_prefix, "SHADER_PARAMS", split_filename )

        # uniform params
        if len( param_list ):
            name_list, num_float_list = zip( *param_list )
            max_name_len = 0
            new_name_list = []
            for i in range( 0, len( name_list ) ):
                max_name_len = max( len( name_list[ i ] ) + 2, max_name_len )
                new_name_list.append( "{0},".format( name_list[ i ] ) )

            for i in range( 0, len( new_name_list ) ):
                new_name_list[ i ] = str( "{:<" + str( max_name_len ) + "}" ).format( new_name_list[ i ] )

            param_list = zip( new_name_list, num_float_list )
            param_strings = []
            for param in param_list:
                param_strings.append( "\t{{ {0}{1} }}".format( param[ 0 ], param[ 1 ] ) )

            params_str = "\n" + UNIFORM_SHADER_TEMPLATE.format( uniform_param_type_name, params_variable_name, ",\n".join( param_strings ) )

        # image params
        images_variable_name = make_shader_tag( self.bytecode_prefix, "IMAGE_PARAMS", split_filename )

        images_str = ""
        if len( image_list ):
            images_str = "\n" + IMAGES_SHADER_TEMPLATE.format( images_variable_name, ",".join( image_list ) )

        uniforms_str = ""
        uniforms_variable_name = make_shader_tag( self.bytecode_prefix, "UNIFORMS", split_filename )

        if len( images_str ) or len( params_str ):
            uniform_name_str_list = []
            param_ptr_str_list = []
            param_count_str_list = []
            image_ptr_str_list = []

            for uniform in uniform_list:
                param_ptr_str = "NULL"
                if uniform[ 1 ] != -1:
                    param_ptr_str = "&{0}[ {1} ]".format( params_variable_name, uniform[ 1 ] )

                image_ptr_str = "NULL"
                if uniform[ 3 ] != -1:
                    image_ptr_str = "&{0}[ {1} ]".format( images_variable_name, uniform[ 3 ] )

                uniform_name_str_list.append( '\"{0}\"'.format( uniform[ 0 ] ) )
                param_ptr_str_list.append( param_ptr_str )
                param_count_str_list.append( str( uniform[ 2 ] ) )
                image_ptr_str_list.append( image_ptr_str )

            max_lengths = ( 0, 0, 0, 0 )
            for item in zip( uniform_name_str_list, param_ptr_str_list, param_count_str_list, image_ptr_str_list ):
                max_lengths = ( max( max_lengths[ 0 ], len( item[ 0 ] ) + 2 ),
                                max( max_lengths[ 1 ], len( item[ 1 ] ) + 2 ),
                                max( max_lengths[ 2 ], len( item[ 2 ] ) + 2 ),
                                max( max_lengths[ 3 ], len( item[ 3 ] ) ) )
             
            rows = []
            i = 0
            for item in zip( uniform_name_str_list, param_ptr_str_list, param_count_str_list, image_ptr_str_list ):
                uniform_name_str_list[ i ] = uniform_name_str_list[ i ] + ","
                while len( uniform_name_str_list[ i ] ) < max_lengths[ 0 ]:
                    uniform_name_str_list[ i ] = uniform_name_str_list[ i ] + " "
                param_ptr_str    = str( "{:<" + str( max_lengths[ 1 ] ) + "}" ).format( item[ 1 ] + "," )
                param_count_str  = str( "{:<" + str( max_lengths[ 2 ] ) + "}" ).format( item[ 2 ] + "," )
                image_ptr_str    = str( "{:<" + str( max_lengths[ 3 ] ) + "}" ).format( item[ 3 ] )

                rows.append( '\t{{ {0}{1}{2}{3} }}'.format( uniform_name_str_list[ i ], param_ptr_str, param_count_str, image_ptr_str ) )
                i += 1
                
            uniforms_str = UNIFORM_SHADER_TEMPLATE.format( uniform_type_name, uniforms_variable_name, ",\n".join( rows ) )
            
        return params_str, images_str, uniforms_str, ( uniforms_variable_name, len( uniform_list ) )

    
    def compile_stage_entries( self, entries, uniform_param_type_name, uniform_type_name, header_template ):
        if not len( entries ):
            return "", ""

        # gather all the resolved file paths
        ex = ShaderFileNotFoundException()

        files = []
        for entry in entries:
            length = len( files )
            for root, dirnames, filenames in os.walk( self.shaders_root ):
                for filename in filenames:
                    if filename == entry[ "filename" ]:
                        files.append( ( os.path.join( root, filename ), entry ) )
            if len( files ) == length:
                ex.filenames.append( entry[ "filename" ] )

        if len( ex.filenames ):
            raise ex

        # compile each shader
        return_str_list = [];
        uniform_data_list = []
        for file, entry in files:
            shader_str = self.compile_shader( file )
            params_str, images_str, uniforms_str, uniforms_data = self.make_uniform_arrays( entry, uniform_param_type_name, uniform_type_name )
            return_str = shader_str + params_str + images_str + uniforms_str
            return_str_list.append( return_str )
            if len( uniforms_data ):
                uniform_data_list.append( uniforms_data )
            else:
                uniform_data_list.append( "NULL" )
                

            if file != files[ len( files ) - 1 ]:
                return_str_list.append( "\n\n" )

        return header_template.format( "".join( return_str_list ) ), uniform_data_list
        

def make_enum_and_tables( entry_sets, uniforms, element_type_name, uniform_type_name, bytecode_prefix ):
    if( len( entry_sets ) == 0 ):
        return "", "", ""
        
    enums = []
    split_filenames = []
    for the_set in entry_sets:
        for entry in the_set:
            filename = os.path.basename( entry[ "filename" ] )
            split_filename = os.path.splitext( filename )
            split_filenames.append( split_filename )
            enums.append( "{0}_NAME_{1}_{2}".format( bytecode_prefix, split_filename[ 1 ].replace( '.', '' ).upper(), split_filename[ 0 ].upper() ) )

    unpacked_uniforms = []
    unpacked_uniform_cnts = []
    for stage in uniforms:
        for shader in stage:
            unpacked_uniforms.append( shader[ 0 ] )
            unpacked_uniform_cnts.append( shader[ 1 ] )

    # enum
    unsorted_enums = enums
    for i in range( 0, len( enums ) ):
        enums[ i ] = "\t{0}".format( enums[ i ] )

    enums = sorted( enums )

    enums_w_count = enums
    enums_w_count.append( "\t/* count */\n\t{0}_NAME_CNT".format( bytecode_prefix ) )
    enum_string = ENUM_TEMPLATE.format( ",\n".join( enums ), bytecode_prefix.lower() )

    # tables ( we'll take advantage of the fact we are preceding these by the enum name in comment, they will be sorted in the same order as the enum string above )
    table_enums = []
    table_codes = []
    table_code_sizes = []
    table_uniforms = []
    table_uniform_cnts = []
    max_chars_enums = 0
    max_chars_codes = 0
    max_chars_sizes = 0
    max_chars_uniforms = 0

    for i in range( 0, len( unsorted_enums ) ):
        table_enum = "{0}".format( unsorted_enums[ i ] ).lstrip()
        table_code = make_shader_tag( bytecode_prefix, "BYTECODE", split_filenames[ i ] ).rstrip()
        table_size = " sizeof( {0} )".format( table_code )
        table_enums.append( table_enum )
        table_codes.append( "{0},".format( table_code ) )
        table_code_sizes.append( "{0},".format( table_size ) )
        table_uniforms.append( "{0},".format( unpacked_uniforms[ i ] ) )
        table_uniform_cnts.append( unpacked_uniform_cnts[ i ] )
        max_chars_enums    = max( max_chars_enums,    len( table_enums[ i ]      ) + 1 )
        max_chars_codes    = max( max_chars_codes,    len( table_codes[ i ]      ) + 1 )
        max_chars_sizes    = max( max_chars_sizes,    len( table_code_sizes[ i ] ) + 1 )
        max_chars_uniforms = max( max_chars_uniforms, len( table_uniforms[ i ]   ) + 1 )
        
    table_entry_list = []
    for i in range( 0, len( unsorted_enums ) ):
        table_enums[ i ]      = str( "{:<" + str( max_chars_enums )    + "}" ).format( table_enums[ i ] )
        table_codes[ i ]      = str( "{:<" + str( max_chars_codes )    + "}" ).format( table_codes[ i ] )
        table_code_sizes[ i ] = str( "{:<" + str( max_chars_sizes )    + "}" ).format( table_code_sizes[ i ] )
        table_uniforms[ i ]   = str( "{:<" + str( max_chars_uniforms ) + "}" ).format( table_uniforms[ i ] )
        table_entry_list.append( "\t{{ /* {0} */ {1}{2}{3}{4} }}".format( table_enums[ i ], table_codes[ i ], table_code_sizes[ i ], table_uniforms[ i ], table_uniform_cnts[ i ] ) )

    table_entry_list = sorted( table_entry_list )
    joined_entries = ",\n".join( table_entry_list )
    table_string = TABLE_TEMPLATE.format( element_type_name, "{0}_TABLE".format( bytecode_prefix ), joined_entries )

    return enum_string, table_string


def compile_from_json(
    output_folder="",
    output_filename="",
    bytecode_prefix="",
    json_path_w_filename="",
    glslc_folder="",
    shaders_root_folder=""
):
    # Verify JSON file exists
    if not os.path.exists( json_path_w_filename ):
        ex = JsonNotFoundException()
        ex.filename_and_path = json_path_w_filename
        raise ex

    # Verify we can find glslc 
    glslc_path = os.path.join( glslc_folder, GLSLC_FILENAME )
    if not os.path.exists( glslc_path ):
        ex = GlslcNotFoundException()
        ex.filename_and_path = glslc_path
        raise ex

    output_path = os.path.join( output_folder, output_filename ) + ".hpp"

    # Parse the JSON
    with open( json_path_w_filename, 'r' ) as f:
        loaded_json = json.load( f )
        
    # Divide all the JSON entries into their pipeline stages
    vertex_entries = []
    tess_control_entries = []
    tess_eval_entries = []
    geometry_entries = []
    fragment_entries = []
    compute_entries = []
    ex = JsonMalformedEntryException()
    for entry in loaded_json[ "shaders" ]:
        stage = entry[ "stage" ]
        if stage == "vertex":
            vertex_entries.append( entry )
        elif stage == "tessellation_control":
            tess_control_entries.append( entry )
        elif stage == "tessellation_evaluation":
            tess_eval_entries.append( entry )
        elif stage == "geometry":
            geometry_entries.append( entry )
        elif stage == "fragment":
            fragment_entries.append( entry )
        elif stage == "compute":
            compute_entries.append( entry )
        else:
            ex.filename = stage[ "filename" ]
            ex.hint.format( stage )
    
    if not ex.filename is None:
        raise ex

    # types
    uniform_param_type_name = "{0}_uniform_vector_type".format( bytecode_prefix.lower() )
    string_uniform_param    = UNIFORM_PARAMETER_TEMPLATE.format( uniform_param_type_name )

    uniform_type_name = "{0}_uniform_type".format( bytecode_prefix.lower() )
    string_uniform    = UNIFORM_TEMPLATE.format( uniform_type_name, uniform_param_type_name, )

    code_table_element_type_name = "{0}_element_type".format( bytecode_prefix.lower() )
    string_code_table_element    = TABLE_ELEMENT_TEMPLATE.format( uniform_type_name, code_table_element_type_name )

    # Compile all shaders for each stage
    compiler = ShaderCompiler( glslc_path, shaders_root_folder, bytecode_prefix )

    string_vertex, uniforms_vertex             = compiler.compile_stage_entries( vertex_entries, uniform_param_type_name, uniform_type_name, VERTEX_SHADER_TEMPLATE )
    string_tess_control, uniforms_tess_control = compiler.compile_stage_entries( tess_control_entries, uniform_param_type_name, uniform_type_name, TESS_CONTROL_TEMPLATE )
    string_tess_eval, uniforms_tess_eval       = compiler.compile_stage_entries( tess_eval_entries, uniform_param_type_name, uniform_type_name, TESS_EVAL_TEMPLATE )
    string_geometry, uniforms_geometry         = compiler.compile_stage_entries( geometry_entries, uniform_param_type_name, uniform_type_name, GEOMETRY_TEMPLATE )
    string_fragment, uniforms_fragment         = compiler.compile_stage_entries( fragment_entries, uniform_param_type_name, uniform_type_name, FRAGMENT_TEMPLATE )
    string_compute, uniforms_compute           = compiler.compile_stage_entries( compute_entries, uniform_param_type_name, uniform_type_name, COMPUTE_TEMPLATE )
    
    #tables
    entry_list = [ vertex_entries, tess_control_entries, tess_eval_entries, geometry_entries, fragment_entries, compute_entries ]
    uniform_list = [ uniforms_vertex, uniforms_tess_control, uniforms_tess_eval, uniforms_geometry, uniforms_fragment, uniforms_compute ]
    string_enum, string_table = make_enum_and_tables( entry_list, uniform_list, code_table_element_type_name, uniform_type_name, bytecode_prefix )
    
    # Create the output folder
    if not os.path.exists( output_folder ):
        os.mkdir( output_folder )

    # write the master string for the file
    filename_no_ext =  os.path.splitext( output_filename )[ 0 ]
    final_str = FILE_CONTENT_TEMPLATE.format( string_vertex,
                                              string_tess_control,
                                              string_tess_eval,
                                              string_geometry,
                                              string_fragment,
                                              string_compute,
                                              string_enum,
                                              string_uniform_param,
                                              string_uniform,
                                              string_code_table_element,
                                              string_table )

    with open( output_path, "w" ) as f:
        f.write( final_str )
        
    return len( vertex_entries ) + len( tess_control_entries ) + len( tess_eval_entries ) + len( geometry_entries ) + len( fragment_entries ) + len( compute_entries )
         