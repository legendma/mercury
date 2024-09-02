#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "Command.hpp"
#include "Event.hpp"
#include "HashMap.hpp"
#include "NonOwningGroup.hpp"
#include "Universe.hpp"
#include "Utilities.hpp"

using namespace ECS;

#define IN_DEBUGGER_PATH            "../src/game/"
#define IN_DEPLOY_PATH              "./"
#define HOTVAR_FILENAME             "HotVars.txt"
#define HOTVAR_TEMP_FILENAME        "_HotVars.temp"
#define LINE_BUFFER_LENGTH          ( 200 )
#define DEFINITIONS_MAX_CNT         ( 150 )
#define BINDINGS_MAX_CNT            ( 150 )

const char *ROOT_DIRECTORY_NAME = ":/";

HASH_MAP_IMPLEMENT( NameHashMap, DEFINITIONS_MAX_CNT, EntityId );

namespace Game
{

typedef struct _HotVarsState
    {
    char               *line_buffer;
    char                file_path[ MAX_FILEPATH_LENGTH ];
    char                temp_file_path[ MAX_FILEPATH_LENGTH ];
    Universe           *universe;
    NonOwningGroupIterator
                        group;
    NameHashMap         names;
    EntityId            current_parse_directory;
    } HotVarsState;


/*******************************************************************
*
*   AsHotVarsState()
*
*   DESCRIPTION:
*       Get the private system state.
*
*******************************************************************/

static inline HotVarsState * AsHotVarsState( Universe *universe )
{
SingletonGameModeComponent *component = (SingletonGameModeComponent*)Universe_GetSingletonComponent( COMPONENT_SINGLETON_HOT_VARS, universe );
return( (HotVarsState *)component->ptr );

} /* AsHotVarsState() */


static void                          Bind( const HotVarBinding *request, HotVarsState *system );
static void                          DropDefinitions( HotVarsState *system );
static EntityId *                    FindDirectoryFromName( const char *name, HotVarsState *system );
static UniverseComponentOnRemoveProc OnRemoveComponent;
static void                          ParseDirectory( HotVarsState *system );
static void                          ParseKeyValuePair( HotVarsState *system );
static CommandProcedure              ProcessCommand;
static void                          ReloadAndUpdateBindings( HotVarsState *system );
static void                          SaveBoundValuesToFile( HotVarsState *system );
static void                          Unbind( const HotVarBinding *request, HotVarsState *system );
static void                          UpdateBindingFromDefinition( const HotVarDefinitionComponent *definition, HotVarBindingComponent *binding );
static void                          UpdateDefinitionFromBinding( const HotVarBindingComponent *binding, HotVarDefinitionComponent *definition );


/*******************************************************************
*
*   HotVars_Init()
*
*   DESCRIPTION:
*       Initialize the hot loaded variables system.
*       Returns TRUE if the system successfully initialized.
*
*******************************************************************/

bool HotVars_Init( Universe *universe )
{
SingletonHotVarsComponent *component = (SingletonHotVarsComponent*)Universe_GetSingletonComponent( COMPONENT_SINGLETON_HOT_VARS, universe );
component->ptr = (HotVarsState*)malloc( sizeof(HotVarsState) );

HotVarsState *system = AsHotVarsState( universe );
clr_struct( system );
system->universe = universe;

Command_RegisterCommandProcessor( COMMAND_PROCESSOR_HOT_VARS, ProcessCommand, universe );
Command_AddCommandClass( COMMAND_PROCESSOR_HOT_VARS, PENDING_COMMAND_BIND_HOT_VAR, COMMAND_PROCESSOR_ACTION_ADD, universe );
Command_AddCommandClass( COMMAND_PROCESSOR_HOT_VARS, PENDING_COMMAND_RELOAD_HOT_VARS, COMMAND_PROCESSOR_ACTION_ADD, universe );
Command_AddCommandClass( COMMAND_PROCESSOR_HOT_VARS, PENDING_COMMAND_SAVE_HOT_VARS, COMMAND_PROCESSOR_ACTION_ADD, universe );

Universe_RegisterComponentLifetime( COMPONENT_HOT_VAR_DEFINITION, nullptr, OnRemoveComponent, universe );
Universe_RegisterComponentLifetime( COMPONENT_HOT_VAR_BINDING,    nullptr, OnRemoveComponent, universe );

HashMap_InitImplementation( &system->names );

/* find the file location */
strcpy( system->file_path, IN_DEPLOY_PATH HOTVAR_FILENAME );
strcpy( system->temp_file_path, IN_DEPLOY_PATH HOTVAR_TEMP_FILENAME );
FILE *fhnd = fopen( system->file_path, "r" );
if( !fhnd )
    {
    strcpy( system->file_path, IN_DEBUGGER_PATH HOTVAR_FILENAME );
    strcpy( system->temp_file_path, IN_DEBUGGER_PATH HOTVAR_TEMP_FILENAME );
    fhnd = fopen( system->file_path, "r" );
    }

if( !fhnd )
    {
    debug_assert_always();
    return( false );
    }

fclose( fhnd );

/* create the line buffer */
hard_assert( !system->line_buffer );
system->line_buffer = (char*)malloc( LINE_BUFFER_LENGTH );
if( !system->line_buffer )
    {
    debug_assert_always();
    return( false );
    }

/* load it */
ReloadAndUpdateBindings( system );

return( true );

} /* HotVars_Init() */


/*******************************************************************
*
*   Bind()
*
*   DESCRIPTION:
*       Process a bind request.
*
*******************************************************************/

static void Bind( const HotVarBinding *request, HotVarsState *system )
{
EntityId *pdirectory = FindDirectoryFromName( request->name, system );
if( !pdirectory )
    {
    /* binding request's directory didn't exist in definitions or wasn't given */
    debug_assert_always();
    return;
    }

u32 keycode = Utilities_HashString( request->name, strlen( request->name ) );
EntityId *pentity = (EntityId*)HashMap_At( keycode, &system->names.map );
if( !pentity )
    {
    /* no definition exists for this binding request */
    debug_assert_always();
    return;
    }

HotVarDefinitionComponent *definition = (HotVarDefinitionComponent*)Universe_TryGetComponent( *pentity, COMPONENT_HOT_VAR_DEFINITION, system->universe );
if( !definition )
    {
    /* no definition exists for this binding request */
    debug_assert_always();
    return;
    }

HotVarBindingComponent *sanity = (HotVarBindingComponent*)Universe_TryGetComponent( *pentity, COMPONENT_HOT_VAR_BINDING, system->universe );
if( sanity )
    {
    /* Detected a possible binding from multiple locations.  Let's check that the binding point matches before ignoring it.
        Currently we only support a 1:1 definition/binding pair.  It would be possible to support 1:many (by storing bindings on their own entities), but today we don't.
        Addendum: on second thought we should keep 1:1 because if multiple bindings, when one changes it's ambiguous which holds the correct value (affects saves) */
    debug_assert( request->type == sanity->binding.type );
    if( request->type == sanity->binding.type )
        {
        switch( sanity->binding.type )
            {
            case HOT_VARS_BOOL:
                debug_assert( request->bind.b == sanity->binding.bind.b );
                break;

            case HOT_VARS_FLOAT:
                debug_assert( request->bind.f == sanity->binding.bind.f );
                break;

            case HOT_VARS_SINT:
                debug_assert( request->bind.s == sanity->binding.bind.s );
                break;

            default:
                debug_assert_always();
                break;
            }
        }
        
    return;
    }

HotVarBindingComponent *binding = (HotVarBindingComponent*)Universe_AttachComponentToEntity( *pentity, COMPONENT_HOT_VAR_BINDING, system->universe );
char *name = (char*)malloc( strlen( request->name ) + 1 );
if( !name )
    {
    hard_assert_always();
    return;
    }

strcpy( name, request->name );

binding->binding.name = name;
binding->binding.type = request->type;
binding->binding.bind = request->bind;

UpdateBindingFromDefinition( definition, binding );

} /* Bind() */


/*******************************************************************
*
*   DropDefinitions()
*
*   DESCRIPTION:
*       Remove all the definitions.
*
*******************************************************************/

static void DropDefinitions( HotVarsState *system )
{
NonOwningGroup_CreateIterator( system->universe, &system->group, NonOwningGroup_GroupIds( COMPONENT_HOT_VAR_DEFINITION ) );

EntityId entity;
HotVarDefinitionComponent *definition;
while( NonOwningGroup_GetNext( &system->group, &entity, (void**)&definition))
    {
    u32 keycode = Utilities_HashString( definition->name, strlen( definition->name ) );
    Universe_RemoveComponentFromEntity( entity, COMPONENT_HOT_VAR_DEFINITION, system->universe );
    HotVarBindingComponent *binding = (HotVarBindingComponent*)Universe_TryGetComponent( entity, COMPONENT_HOT_VAR_BINDING, system->universe );
    if( !binding )
        {
        Universe_DestroyEntity( entity, system->universe );
        HashMap_Delete( keycode, &system->names.map );
        }    
    }

}   /* DropDefinitions() */


/*******************************************************************
*
*   FindDirectoryFromName()
*
*   DESCRIPTION:
*       Give a binding name, find it's entity in our name map.
*
*******************************************************************/

static EntityId * FindDirectoryFromName( const char *name, HotVarsState *system )
{
int length = (int)strlen( name );
while( length )
    {
    if( name[ length - 1 ] == '/' )
        {
        break;
        }

    length--;
    }

if( length != 1 )
    {
    length--;
    }

if( !length )
    {
    return( nullptr );
    }

u32 keycode = Utilities_HashString( name, length );
return( (EntityId*)HashMap_At( keycode, &system->names.map ) );

}    /* FindDirectoryFromName() */


/*******************************************************************
*
*   OnRemoveComponent()
*
*   DESCRIPTION:
*       Clean up memory on component.
*
*******************************************************************/

static void OnRemoveComponent( const EntityId entity, const ComponentClass cls, void *component, _Universe *universe )
{
switch( cls )
    {
    case COMPONENT_HOT_VAR_BINDING:
        {
        HotVarBindingComponent *bind = (HotVarBindingComponent*)component;
        free( (void*)bind->binding.name );
        break;
        }

    case COMPONENT_HOT_VAR_DEFINITION:
        {
        HotVarDefinitionComponent *def = (HotVarDefinitionComponent*)component;
        free( (void*)def->name );
        break;
        }

    default:
        debug_assert_always();
        break;
    }

}   /* OnRemoveComponent() */


/*******************************************************************
*
*   ParseDirectory()
*
*   DESCRIPTION:
*       Parse the directory in the system's line buffer and create a
*       definition from it.
*
*******************************************************************/

static void ParseDirectory( HotVarsState *system )
{
debug_assert( system->line_buffer[ 0 ] == ':' );
int caret = 1;

/* remove any comments to isolate the directory name */
bool found_space = false;
int i = caret;
while( true )
    {
    if( system->line_buffer[ i ] == 0 )
        {
        break;
        }
     
     if( system->line_buffer[ i ] == ' ' )
        {
        found_space = true;
        system->line_buffer[ i ] = 0;
        }

    i++;
    if( found_space
     && system->line_buffer[ i ] != 0
     && system->line_buffer[ i ] != ' ' )
        {
        debug_assert( system->line_buffer[ i ] == '#' );/* found trailing characters that were not a comment */
        break;
        }
    }

size_t name_length = strlen( &system->line_buffer[ caret ] );
if( name_length < 1 )
    {
    debug_assert_always();
    return;
    }
else if( system->line_buffer[ caret ] != '/' )
    {
    /* directories must start with a forward slash */
    debug_assert_always();
    return;
    }

/* trim any trailing slashes */
while( name_length > 1
    && system->line_buffer[ caret + ( name_length - 1 ) ] == '/' )
    {
    name_length--;
    }

u32 keycode = Utilities_HashString( &system->line_buffer[ caret ], name_length );
EntityId *pentity = (EntityId*)HashMap_At( keycode, &system->names.map );
if( pentity )
    {
    system->current_parse_directory = *pentity;
    HotVarDefinitionComponent *def = (HotVarDefinitionComponent*)Universe_TryGetComponent( *pentity, COMPONENT_HOT_VAR_DEFINITION, system->universe );
    if( def )
        {
        /* already have this directory in our definitions, so just set it current */
        return;
        }
    }

/* add the definition */
char *name = (char*)malloc( name_length + 1 );
if( !name )
    {
    hard_assert_always();
    return;
    }

if( !pentity )
    {
    system->current_parse_directory = Universe_CreateNewEntity( system->universe );
    HashMap_Insert( keycode, &system->current_parse_directory, &system->names.map );
    }

strcpy( name, &system->line_buffer[ caret ] );

HotVarDefinitionComponent *def = (HotVarDefinitionComponent*)Universe_AttachComponentToEntity( system->current_parse_directory, COMPONENT_HOT_VAR_DEFINITION, system->universe );
def->type = HOT_VARS_DIRECTORY;
def->name = name;

}   /* ParseDirectory() */


/*******************************************************************
*
*   ParseKeyValuePair()
*
*   DESCRIPTION:
*       Parse the key value pair in the system's line buffer and
*       create a definition from it.
*
*******************************************************************/

static void ParseKeyValuePair( HotVarsState *system )
{
char read_char;
int caret = 0;

/* parse the key name */
char *key_str = system->line_buffer;

while( Utilities_ReadCharFromBuffer( &caret, system->line_buffer, LINE_BUFFER_LENGTH, &read_char ) )
    {
    if( read_char == ' ' )
        {
        break;
        }
    }

system->line_buffer[ --caret ] = 0;
int key_str_length = caret;
caret++;

while( Utilities_ReadCharFromBuffer( &caret, system->line_buffer, LINE_BUFFER_LENGTH, &read_char ) )
    {
    /* skip any extra preceding spaces or errant = sign */
    if( read_char != ' '
     && read_char != '=' )
        {
        caret--;
        break;
        }    
    }

char *value_str = &system->line_buffer[ caret ];

/* remove any comments to isolate the value */
bool reach_end_of_value = false;
int i = caret;
while( true )
    {
    if( system->line_buffer[ i ] == 0 )
        {
        break;
        }
     
     if( system->line_buffer[ i ] == '#' )
        {
        system->line_buffer[ i ] = 0;
        break;
        }

     if( system->line_buffer[ i ] == ' ' )
        {
        reach_end_of_value = true;
        system->line_buffer[ i ] = 0;
        }

    i++;
    if( reach_end_of_value
     && system->line_buffer[ i ] != 0
     && system->line_buffer[ i ] != ' ' )
        {
        debug_assert( system->line_buffer[ i ] == '#' );/* found trailing characters that were not a comment */
        break;
        }
    }

/* parse the value */
int value_str_length = (int)strlen( value_str );
HotVarDefinitionComponent entry = {};

if( Utilities_StrContainsStr( value_str, true, "true"  )
 || Utilities_StrContainsStr( value_str, true, "false" ) )
    {
    /* boolean pair */
    entry.type    = HOT_VARS_BOOL;
    entry.value.b = Utilities_StrContainsStr( value_str, true, "true" );
    }
else if( Utilities_StrContainsStr( value_str, false, "." )
      || Utilities_StrContainsStr( value_str, false, "f" ) )
    {
    /* float pair */
    while( !Utilities_CharIsNumber( value_str[ value_str_length - 1 ] ) )
        {
        /* erase any '.' or other non-numbers dangling on the right */
        value_str_length--;
        value_str[ value_str_length ] = 0;
        }

    while( Utilities_ReadCharFromBuffer( &caret, system->line_buffer, LINE_BUFFER_LENGTH, &read_char ) )
        {
        if( read_char != '.'
        &&  ( read_char != '-' && value_str != &system->line_buffer[ caret - 1 ] )
        &&  ( read_char != '+' && value_str != &system->line_buffer[ caret - 1 ] )
        && !Utilities_CharIsNumber( read_char ) )
            {
            /* found invalid character in expected float */
            debug_assert_always();
            return;
            }
        }

    entry.type    = HOT_VARS_FLOAT;
    entry.value.f = (float)atof( value_str );
    }
else
    {
    /* integer pair */
    while( Utilities_ReadCharFromBuffer( &caret, system->line_buffer, LINE_BUFFER_LENGTH, &read_char ) )
        {
        if( ( read_char != '-' && value_str != &system->line_buffer[ caret - 1 ] )
        &&  ( read_char != '+' && value_str != &system->line_buffer[ caret - 1 ] )
        && !Utilities_CharIsNumber( read_char ) )
            {
            /* found invalid character in expected signed integer */
            debug_assert_always();
            return;
            }
        }

    entry.type    = HOT_VARS_SINT;
    entry.value.s = atoi( value_str );
    value_str = nullptr;
    }

/* get the directory */
HotVarDefinitionComponent *dir = (HotVarDefinitionComponent*)Universe_TryGetComponent( system->current_parse_directory, COMPONENT_HOT_VAR_DEFINITION, system->universe );
if( !dir )
    {
    debug_assert_always();
    return;
    }

/* add the definition */
int name_length = (int)strlen( dir->name ) + 1/* slash */ + (int)strlen( key_str );
int buffer_length = name_length + 1;
char *name = (char*)malloc( buffer_length );
if( !name )
    {
    hard_assert_always();
    return;
    }

if( strlen( dir->name ) > 1 )
    {
    sprintf_s( name, buffer_length, "%s/%s", dir->name, key_str );
    }
else
    {
    sprintf_s( name, buffer_length, "%s%s", dir->name, key_str );
    name_length--;
    }
    
u32 keycode       = Utilities_HashString( name, name_length );
EntityId *pentity = (EntityId*)HashMap_At( keycode, &system->names.map );
EntityId entity;
if( pentity )
    {
    entity = *pentity;
    HotVarDefinitionComponent *sanity = (HotVarDefinitionComponent*)Universe_TryGetComponent( entity, COMPONENT_HOT_VAR_DEFINITION, system->universe );
    if( sanity )
        {
        /* this definition appears to be defined twice in the file.  Let's verify the value matches before ignoring it. */
        debug_assert( 0 == strcmp( sanity->name, name ) );
        debug_assert( sanity->directory.id_and_version == system->current_parse_directory.id_and_version );
        debug_assert( sanity->type == entry.type );
        if( sanity->type == entry.type )
            {
            switch( sanity->type )
                {
                case HOT_VARS_BOOL:
                    debug_assert( sanity->value.b == entry.value.b );
                    break;

                case HOT_VARS_FLOAT:
                    debug_assert( sanity->value.f == entry.value.f );
                    break;

                case HOT_VARS_SINT:
                    debug_assert( sanity->value.s == entry.value.s );
                    break;

                default:
                    debug_assert_always();
                }
            }

        free( name );
        return;
        }
    }
else
    {
    entity = Universe_CreateNewEntity( system->universe );
    HashMap_Insert( keycode, &entity, &system->names.map );
    }

HotVarDefinitionComponent *def = (HotVarDefinitionComponent*)Universe_AttachComponentToEntity( entity, COMPONENT_HOT_VAR_DEFINITION, system->universe );
*def = entry;

def->directory = system->current_parse_directory;
def->name      = name;

}   /* ParseKeyValuePair() */


/*******************************************************************
*
*   ProcessCommand()
*
*   DESCRIPTION:
*       Process a command.
*
*******************************************************************/

static void ProcessCommand( const PendingCommandComponent *command, Universe *universe )
{
HotVarsState *system = AsHotVarsState( universe );
switch( command->cls )
    {
    case PENDING_COMMAND_BIND_HOT_VAR:
        if( command->u.hot_vars_bind.is_bind )
            {
            Bind( &command->u.hot_vars_bind.request, system );
            }
        else
            {
            Unbind( &command->u.hot_vars_bind.request, system );
            }
        break;

    case PENDING_COMMAND_RELOAD_HOT_VARS:
        ReloadAndUpdateBindings( system );
        break;

    case PENDING_COMMAND_SAVE_HOT_VARS:
        SaveBoundValuesToFile( system );
        break;

    default:
        break;
    }

} /* ProcessCommand() */


/*******************************************************************
*
*   ReloadAndUpdateBindings()
*
*   DESCRIPTION:
*       Reload the hot variables file and update all bindings.
*
*******************************************************************/

static void ReloadAndUpdateBindings( HotVarsState *system )
{
DropDefinitions( system );

/* add the root directory */
strcpy( system->line_buffer, ROOT_DIRECTORY_NAME );
ParseDirectory( system );

/* parse the file */
int file_sz = 0;
char *file_buffer = Utilities_ReadWholeTextFile( system->file_path, &file_sz );

int caret = 0;
while( Utilities_ReadLineFromBuffer( &caret, file_buffer, file_sz, system->line_buffer, LINE_BUFFER_LENGTH ) )
    {
    if( !strlen( system->line_buffer )
     || system->line_buffer[ 0 ] == '#' )
        {
        /* blank line - ignore */
        continue;
        }

    if( system->line_buffer[ 0 ] == ':' )
        {
        ParseDirectory( system );
        }
    else
        {
        ParseKeyValuePair( system );
        }
    }

free( file_buffer );
file_buffer = nullptr;

/* update the bindings */
NonOwningGroup_CreateIterator( system->universe, &system->group, NonOwningGroup_GroupIds( COMPONENT_HOT_VAR_BINDING, COMPONENT_HOT_VAR_DEFINITION ) );
EntityId entity;
while( NonOwningGroup_GetNext( &system->group, &entity, nullptr ) )
    {
    HotVarBindingComponent *binding       = (HotVarBindingComponent*)Universe_TryGetComponent( entity, COMPONENT_HOT_VAR_BINDING, system->universe );
    HotVarDefinitionComponent *definition = (HotVarDefinitionComponent*)Universe_TryGetComponent( entity, COMPONENT_HOT_VAR_DEFINITION, system->universe );
    UpdateBindingFromDefinition( definition, binding );
    }

/* notify listeners */
Event_Enqueue( EVENT_NOTIFICATION_HOT_VARS_RELOADED, nullptr, system->universe );

}   /* ReloadAndUpdateBindings() */


/*******************************************************************
*
*   SaveBoundValuesToFile()
*
*   DESCRIPTION:
*       Reload the hot variables file and update all bindings.
*
*******************************************************************/

static void SaveBoundValuesToFile( HotVarsState *system )
{
#define write_crlf( _fhnd ) \
    fwrite( "\n", 1, 1, _fhnd )

/* set the root directory as current */
u32 keycode = Utilities_HashString( &ROOT_DIRECTORY_NAME[ 1 ], strlen( &ROOT_DIRECTORY_NAME[1] ) );
EntityId *pdir = (EntityId*)HashMap_At( keycode, &system->names.map );
if( !pdir )
    {
    /* we've not loaded for some reason - critical error */
    debug_assert_always();
    return;
    }

system->current_parse_directory = *pdir;
HotVarDefinitionComponent *current_parse_directory = (HotVarDefinitionComponent*)Universe_TryGetComponent( system->current_parse_directory, COMPONENT_HOT_VAR_DEFINITION, system->universe );
int current_parse_directory_len = (int)strlen( current_parse_directory->name );

/* read the file to buffer */
int file_sz = 0;
char *file_buffer = Utilities_ReadWholeTextFile( system->file_path, &file_sz );
if( !file_buffer )
    {
    debug_assert_always();
    return;
    }

/* update the definitions */
NonOwningGroup_CreateIterator( system->universe, &system->group, NonOwningGroup_GroupIds( COMPONENT_HOT_VAR_BINDING, COMPONENT_HOT_VAR_DEFINITION ) );
EntityId entity;
while( NonOwningGroup_GetNext( &system->group, &entity, nullptr ) )
    {
    HotVarBindingComponent *binding       = (HotVarBindingComponent*)Universe_TryGetComponent( entity, COMPONENT_HOT_VAR_BINDING, system->universe );
    HotVarDefinitionComponent *definition = (HotVarDefinitionComponent*)Universe_TryGetComponent( entity, COMPONENT_HOT_VAR_DEFINITION, system->universe );
    UpdateDefinitionFromBinding( binding, definition );
    }

/* create the temp file */
FILE *fhnd = fopen( system->temp_file_path, "w" );
if( !fhnd )
    {
    debug_assert_always();
    free( file_buffer );
    return;
    }

/* parse the hot var file and write it to temp w/ new values */
int file_caret = 0;
while( Utilities_ReadLineFromBuffer( &file_caret, file_buffer, file_sz, system->line_buffer, LINE_BUFFER_LENGTH ) )
    {
    if( !strlen( system->line_buffer ) )
        {
        /* blank line */
        write_crlf( fhnd );
        continue;
        }
     else if( system->line_buffer[ 0 ] == '#' )
        {
        /* comment line - just copying it */
        fwrite( system->line_buffer, strlen( system->line_buffer ), 1, fhnd );
        write_crlf( fhnd );
        continue;
        }

    if( system->line_buffer[ 0 ] == ':' )
        {
        /* directory - write the whole line verbatim */
        fwrite( ":", 1, 1, fhnd );
        int line_caret = 1;
        char read_char = 0;
        int name_length = 0;
        bool name_parsed = false;
        while( Utilities_ReadCharFromBuffer( &line_caret, system->line_buffer, LINE_BUFFER_LENGTH, &read_char ) )
            {
            fwrite( &read_char, 1, 1, fhnd );
            /* we also need to set the directory to current */
            name_length++;
            if( system->line_buffer[ line_caret ] == ' '
             || system->line_buffer[ line_caret ] == '#'
             || system->line_buffer[ line_caret ] == 0 )
                {
                if( !name_parsed )
                    {
                    /* find the directory and set it current */
                    name_parsed = true;
                    u32 dir_keycode = Utilities_HashString( &system->line_buffer[ 1 ], name_length );
                    EntityId *pentity = (EntityId*)HashMap_At( dir_keycode, &system->names.map );
                    if( !pentity )
                        {
                        /* crtical error! */
                        debug_assert_always();
                        free( file_buffer );
                        fclose( fhnd );
                        return;
                        }

                    system->current_parse_directory = *pentity;
                    current_parse_directory = (HotVarDefinitionComponent*)Universe_TryGetComponent( system->current_parse_directory, COMPONENT_HOT_VAR_DEFINITION, system->universe );
                    if( !current_parse_directory )
                        {
                        /* crtical error! */
                        debug_assert_always();
                        free( file_buffer );
                        fclose( fhnd );
                        return;
                        }

                    current_parse_directory_len = (int)strlen( current_parse_directory->name );
                    }
                }
            }

        write_crlf( fhnd );
        }
    else
        {
        /* key/value pair - write the name and new value */
        if( !Utilities_StrContainsStr( system->line_buffer, false, " " ) )
            {
            debug_assert_always();
            free( file_buffer );
            fclose( fhnd );
            return;
            }

        int line_caret = 0;
        char read_char = 0;
        int name_length = 0;
        bool name_parsed = false;
        bool value_parsed = false;
        while( Utilities_ReadCharFromBuffer( &line_caret, system->line_buffer, LINE_BUFFER_LENGTH, &read_char ) )
            {
            if( !name_parsed )
                {
                /* name */
                fwrite( &read_char, 1, 1, fhnd );
                name_length++;
                if( system->line_buffer[ line_caret ] == ' ' )
                    {
                    system->line_buffer[ line_caret++ ] = 0;
                    fwrite( " ", 1, 1, fhnd );

                    /* find the definition by its name and write the current value to file */
                    name_parsed = true;
                    u32 malloc_len = current_parse_directory_len + 1/* slash */ + name_length + 1/* null */;
                    char *full_name = (char*)malloc( malloc_len );
                    if( current_parse_directory_len > 1 )
                        {
                        sprintf( full_name, "%s/%s", current_parse_directory->name, system->line_buffer );
                        }
                    else
                        {
                        sprintf( full_name, "%s%s", current_parse_directory->name, system->line_buffer );
                        }

                    u32 name_keycode = Utilities_HashString( full_name, strlen( full_name ) );
                    free( full_name );
                    EntityId *pentity = (EntityId*)HashMap_At( name_keycode, &system->names.map );
                    if( !pentity )
                        {
                        /* crtical error! */
                        debug_assert_always();
                        free( file_buffer );
                        fclose( fhnd );
                        return;
                        }

                    HotVarDefinitionComponent *value_def = (HotVarDefinitionComponent*)Universe_TryGetComponent( *pentity, COMPONENT_HOT_VAR_DEFINITION, system->universe );
                    if( !value_def )
                        {
                        /* crtical error! */
                        debug_assert_always();
                        free( file_buffer );
                        fclose( fhnd );
                        return;
                        }

                    char value_str[ 100 ];
                    if( value_def->type == HOT_VARS_BOOL
                     && value_def->value.b == true )
                        {
                        strcpy( value_str, "True" );
                        }
                    else if( value_def->type == HOT_VARS_BOOL )
                        {
                        strcpy( value_str, "False" );
                        }
                    else if( value_def->type == HOT_VARS_FLOAT )
                        {
                        sprintf( value_str, "%f", value_def->value.f );
                        }
                    else if( value_def->type == HOT_VARS_SINT )
                        {
                        sprintf( value_str, "%d", value_def->value.s );
                        }
                    else
                        {
                        /* crtical error! */
                        debug_assert_always();
                        free( file_buffer );
                        fclose( fhnd );
                        return;
                        }

                    fwrite( value_str, strlen( value_str ), 1, fhnd );
                    }
                }
            else if( !value_parsed )
                {
                if( read_char == ' '
                 || read_char == '#' )
                    {
                    value_parsed = true;
                    fwrite( &read_char, 1, 1, fhnd );
                    }
                }
            else
                {
                fwrite( &read_char, 1, 1, fhnd );
                }
            }

        write_crlf( fhnd );
        }
    }

/* close the temp file, and rename it to overwrite the existing file */
fclose( fhnd );
fhnd = nullptr;
do_debug_assert( !remove( system->file_path ) );
do_debug_assert( !rename( system->temp_file_path, system->file_path ) );

/* notify listeners */
Event_Enqueue( EVENT_NOTIFICATION_HOT_VARS_SAVED, nullptr, system->universe );

}   /* SaveBoundValuesToFile() */


/*******************************************************************
*
*   Unbind()
*
*   DESCRIPTION:
*       Process an unbind request.
*
*******************************************************************/

static void Unbind( const HotVarBinding *request, HotVarsState *system )
{
EntityId *pdirectory = FindDirectoryFromName( request->name, system );
if( !pdirectory )
    {
    /* binding request's directory didn't exist in definitions or wasn't given */
    debug_assert_always();
    free( (void *)request->name );
    return;
    }

u32 keycode = Utilities_HashString( request->name, strlen( request->name ) );
EntityId *pentity = (EntityId*)HashMap_At( keycode, &system->names.map );
if( !pentity )
    {
    /* no binding existed */
    free( (void *)request->name );
    return;
    }

HotVarBindingComponent *binding = (HotVarBindingComponent*)Universe_TryGetComponent( *pentity, COMPONENT_HOT_VAR_BINDING, system->universe );
if( !binding )
    {
    /* no binding existed */
    free( (void *)request->name );
    return;
    }

debug_assert( request->type == binding->binding.type );
if( request->type == binding->binding.type )
    {
    switch( binding->binding.type )
        {
        case HOT_VARS_BOOL:
            debug_assert( request->bind.b == binding->binding.bind.b );
            break;

        case HOT_VARS_FLOAT:
            debug_assert( request->bind.f == binding->binding.bind.f );
            break;

        case HOT_VARS_SINT:
            debug_assert( request->bind.s == binding->binding.bind.s );
            break;

        default:
            debug_assert_always();
            free( (void *)request->name );
            return;
        }
    }

Universe_RemoveComponentFromEntity( *pentity, COMPONENT_HOT_VAR_BINDING, system->universe );        
free( (void*)request->name );

} /* Unbind() */


/*******************************************************************
*
*   UpdateBindingFromDefinition()
*
*   DESCRIPTION:
*       Copy the definition's value to the given binding.
*
*******************************************************************/

static void UpdateBindingFromDefinition( const HotVarDefinitionComponent *definition, HotVarBindingComponent *binding )
{
debug_assert( binding->binding.type == definition->type );
if( binding->binding.type == definition->type )
    {
    switch( binding->binding.type )
        {
        case HOT_VARS_BOOL:
            *binding->binding.bind.b = definition->value.b;
            break;

        case HOT_VARS_FLOAT:
            *binding->binding.bind.f = definition->value.f;
            break;

        case HOT_VARS_SINT:
            *binding->binding.bind.s = definition->value.s;
            break;

        default:
            debug_assert_always();
            break;
        }
    }

} /* UpdateBindingFromDefinition() */


/*******************************************************************
*
*   UpdateDefinitionFromBinding()
*
*   DESCRIPTION:
*       Copy the binding's value to the given definition.
*
*******************************************************************/

static void UpdateDefinitionFromBinding( const HotVarBindingComponent *binding, HotVarDefinitionComponent *definition )
{
debug_assert( definition->type == binding->binding.type );
if( definition->type == binding->binding.type)
    {
    switch( definition->type )
        {
        case HOT_VARS_BOOL:
            definition->value.b = *binding->binding.bind.b;
            break;

        case HOT_VARS_FLOAT:
            definition->value.f = *binding->binding.bind.f;
            break;

        case HOT_VARS_SINT:
            definition->value.s = *binding->binding.bind.s;
            break;

        default:
            debug_assert_always();
            break;
        }
    }

} /* UpdateDefinitionFromBinding() */

} /* namespace Game */
