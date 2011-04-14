/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2011 The EVEmu Team
    For the latest information visit http://evemu.org
    ------------------------------------------------------------------------------------
    This program is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by the Free Software
    Foundation; either version 2 of the License, or (at your option) any later
    version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License along with
    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
    http://www.gnu.org/copyleft/lesser.txt.
    ------------------------------------------------------------------------------------
    Author:     Zhur, Bloody.Rabbit
*/

#include "EVEToolPCH.h"

#include "main.h"
#include "Commands.h"

/************************************************************************/
/* Commands declaration                                                 */
/************************************************************************/
void DestinyDumpLogText( const Seperator& cmd );
void CRC32Text( const Seperator& cmd );
void ExitProgram( const Seperator& cmd );
void PrintHelp( const Seperator& cmd );
void ObjectToSQL( const Seperator& cmd );
void TestMarshal( const Seperator& cmd );
void PrintTimeNow( const Seperator& cmd );
void LoadScript( const Seperator& cmd );
void TimeToString( const Seperator& cmd );
void TriToOBJ( const Seperator& cmd );
void UnmarshalLogText( const Seperator& cmd );
void StuffExtract( const Seperator& cmd );

/************************************************************************/
/* Command array stuff                                                  */
/************************************************************************/
const EVEToolCommand EVETOOL_COMMANDS[] =
{
    { "destiny",   &DestinyDumpLogText, "Converts given string to binary and dumps it as destiny binary." },
    { "crc32",     &CRC32Text,          "Computes CRC-32 checksum of given arguments."                    },
    { "exit",      &ExitProgram,        "Quits current session."                                          },
    { "help",      &PrintHelp,          "Lists available commands or prints help about specified one."    },
    { "mtest",     &TestMarshal,        "Performs marshal test."                                          },
    { "now",       &PrintTimeNow,       "Prints current time in Win32 time format."                       },
    { "obj2sql",   &ObjectToSQL,        "Converts specified cache object into an SQL update."             },
    { "script",    &LoadScript,         "Loads input from specified file(s)."                             },
    { "time",      &TimeToString,       "Interprets given integer as Win32 time."                         },
    { "tri2obj",   &TriToOBJ,           "Dumps specified TRI file."                                       },
    { "unmarshal", &UnmarshalLogText,   "Converts given string to binary and unmarshals it."              },
    { "xstuff",    &StuffExtract,       "Dumps specified STUFF file."                                     }
};
const size_t EVETOOL_COMMAND_COUNT = ( sizeof( EVETOOL_COMMANDS ) / sizeof( EVEToolCommand ) );

const EVEToolCommand* FindCommand( const char* name )
{
    return FindCommand( std::string( name ) );
}

const EVEToolCommand* FindCommand( const std::string& name )
{
    for( size_t i = 0; i < EVETOOL_COMMAND_COUNT; ++i )
    {
        const EVEToolCommand* c = &EVETOOL_COMMANDS[i];

        if( c->name == name )
            return c;
    }

    return NULL;
}

void ProcessCommand( const Seperator& cmd )
{
    const char* cmdName = cmd.arg( 0 ).c_str();
    const EVEToolCommand* c = FindCommand( cmdName );

    if( NULL == c )
        sLog.Error( "input", "Unknown command '%s'.", cmdName );
    else
        ( *c->callback )( cmd );
}

/************************************************************************/
/* Commands implementation                                              */
/************************************************************************/
void DestinyDumpLogText( const Seperator& cmd )
{
    const char* cmdName = cmd.arg( 0 ).c_str();

    if( 1 == cmd.argCount() )
    {
        sLog.Error( cmdName, "Usage: %s destiny-binary [destiny-binary] ...", cmdName );
        return;
    }

    for( size_t i = 1; i < cmd.argCount(); ++i )
    {
        const std::string& destinyBinaryStr = cmd.arg( i );

        Buffer destinyBinary;
        if( !PyDecodeEscape( destinyBinaryStr.c_str(), destinyBinary ) )
        {
            sLog.Error( cmdName, "Failed to decode destiny binary." );
            continue;
        }

        Destiny::DumpUpdate( DESTINY__MESSAGE, &destinyBinary[0], destinyBinary.size() );
    }
}

void CRC32Text( const Seperator& cmd )
{
    const char* cmdName = cmd.arg( 0 ).c_str();

    if( 1 == cmd.argCount() )
    {
        sLog.Error( cmdName, "Usage: %s text-to-checksum [text-to-checksum] ...", cmdName );
        return;
    }

    for( size_t i = 1; i < cmd.argCount(); ++i )
    {
        const std::string& s = cmd.arg( i );

        sLog.Log( cmdName, "%X", CRC32::Generate( (const uint8*)s.c_str(), s.size() ) );
    }
}

void ExitProgram( const Seperator& cmd )
{
    // just close standart input
    fclose( stdin );
}

void PrintHelp( const Seperator& cmd )
{
    const char* cmdName = cmd.arg( 0 ).c_str();

    if( 1 == cmd.argCount() )
    {
        sLog.Log( cmdName, "Available commands:" );

        for( size_t i = 0; i < EVETOOL_COMMAND_COUNT; ++i )
        {
            const EVEToolCommand* c = &EVETOOL_COMMANDS[i];

            sLog.Log( cmdName, "%s", c->name );
        }

        sLog.Log( cmdName, "You can get detailed help by typing '%s <command> [<command>] ...'.", cmdName );
    }
    else
    {
        for( size_t i = 1; i < cmd.argCount(); ++i )
        {
            const std::string& cmdStr = cmd.arg( i );
            const EVEToolCommand* c = FindCommand( cmdStr );

            if( NULL == c )
                sLog.Error( cmdName, "Unknown command '%s'.", cmdStr.c_str() );
            else
                sLog.Log( cmdName, "%s: %s", c->name, c->description );
        }
    }
}

void TestMarshal( const Seperator& cmd )
{
    const char* cmdName = cmd.arg( 0 ).c_str();

    DBRowDescriptor *header = new DBRowDescriptor;
    // Fill header:
    header->AddColumn( "historyDate", DBTYPE_FILETIME );
    header->AddColumn( "lowPrice", DBTYPE_CY );
    header->AddColumn( "highPrice", DBTYPE_CY );
    header->AddColumn( "avgPrice", DBTYPE_CY );
    header->AddColumn( "volume", DBTYPE_I8 );
    header->AddColumn( "orders", DBTYPE_I4 );

    CRowSet* rs = new CRowSet( &header );

    PyPackedRow* row = rs->NewRow();
    row->SetField( "historyDate", new PyLong( Win32TimeNow() ) );
    row->SetField( "lowPrice", new PyLong( 18000 ) );
    row->SetField( "highPrice", new PyLong( 19000 ) );
    row->SetField( "avgPrice", new PyLong( 18400 ) );
    row->SetField( "volume", new PyLong( 5463586 ) );
    row->SetField( "orders", new PyInt( 254 ) );

    sLog.Log( cmdName, "Marshaling..." );

    Buffer marshaled;
    bool res = MarshalDeflate( rs, marshaled );
    PyDecRef( rs );

    if( !res )
    {
        sLog.Error( cmdName, "Failed to marshal Python object." );
        return;
    }

    sLog.Log( cmdName, "Unmarshaling..." );

    PyRep* rep = InflateUnmarshal( marshaled );
    if( NULL == rep )
    {
        sLog.Error( cmdName, "Failed to unmarshal Python object." );
        return;
    }

    sLog.Success( cmdName, "Final:" );
    rep->Dump( stdout, "    " );

    PyDecRef( rep );
}

void PrintTimeNow( const Seperator& cmd )
{
    const char* cmdName = cmd.arg( 0 ).c_str();

    sLog.Log( cmdName, "Now in Win32 time: "I64u".", Win32TimeNow() );
}

void ObjectToSQL( const Seperator& cmd )
{
    const char* cmdName = cmd.arg( 0 ).c_str();

    if( 5 != cmd.argCount() )
    {
        sLog.Error( cmdName, "Usage: %s [cache_file] [table_name] [key_field] [file_name]", cmdName );
        return;
    }
    const std::string& cacheFile = cmd.arg( 1 );
    const std::string& tableName = cmd.arg( 2 );
    const std::string& keyField = cmd.arg( 3 );
    const std::string& fileName = cmd.arg( 4 );

    std::string abs_fname( "../data/cache/" );
    abs_fname += cacheFile;
    abs_fname += ".cache";

    sLog.Log( cmdName, "Converting cached object %s:\n", abs_fname.c_str() );

    CachedObjectMgr mgr;
    PyCachedObjectDecoder* obj = mgr.LoadCachedObject( abs_fname.c_str(), cacheFile.c_str() );
    if( obj == NULL )
    {
        sLog.Error( cmdName, "Unable to load or decode '%s'!", abs_fname.c_str() );
        return;
    }

    obj->cache->DecodeData();
    if( obj->cache->decoded() == NULL )
    {
        sLog.Error( cmdName, "Unable to load or decode body of '%s'!", abs_fname.c_str() );

        SafeDelete( obj );
        return;
    }

    FILE* out = fopen( fileName.c_str(), "w" );
    if( out == NULL )
    {
        sLog.Error( cmdName, "Unable to open output file '%s'", fileName.c_str() );

        SafeDelete( obj );
        return;
    }

    SetSQLDumper dumper( tableName.c_str(), keyField.c_str(), out );
    if( obj->cache->decoded()->visit( dumper ) )
        sLog.Success( cmdName, "Dumping of %s succeeded.", tableName.c_str() );
    else
        sLog.Error( cmdName, "Dumping of %s failed.", tableName.c_str() );

    fclose( out );
    SafeDelete( obj );
}

void LoadScript( const Seperator& cmd )
{
    const char* cmdName = cmd.arg( 0 ).c_str();

    if( 1 == cmd.argCount() )
    {
        sLog.Error( cmdName, "Usage: %s file [file] ...", cmdName );
        return;
    }

    for( size_t i = 1; i < cmd.argCount(); ++i )
        ProcessFile( cmd.arg( i ) );
}

void TimeToString( const Seperator& cmd )
{
    const char* cmdName = cmd.arg( 0 ).c_str();

    if( 1 == cmd.argCount() )
    {
        sLog.Error( cmdName, "Usage: %s win32-time [win32-time] ...", cmdName );
        return;
    }

    for( size_t i = 1; i < cmd.argCount(); ++i )
    {
        const std::string& timeStr = cmd.arg( i );

        uint64 t;
        sscanf( timeStr.c_str(), I64u, &t );
        const std::string time = Win32TimeToString( t );

        sLog.Log( cmdName, "%s is %s.", timeStr.c_str(), time.c_str() );
    }
}

void TriToOBJ( const Seperator& cmd )
{
    const char* cmdName = cmd.arg( 0 ).c_str();

    if( 4 != cmd.argCount() )
    {
        sLog.Error( cmdName, "Usage: %s [trifile] [objout] [mtlout]", cmdName );
        return;
    }
    const std::string& trifile = cmd.arg( 1 );
    const std::string& objout = cmd.arg( 2 );
    const std::string& mtlout = cmd.arg( 3 );

    TriExporter::TriFile f;
    if( !f.LoadFile( trifile.c_str() ) )
    {
        sLog.Error( cmdName, "Failed to load trifile '%s'.", trifile.c_str() );
        return;
    }

    f.DumpHeaders();
    f.ExportObj( objout.c_str(), mtlout.c_str() );

    sLog.Success( cmdName, "%s - %s - written.", objout.c_str(), mtlout.c_str() );
}

void UnmarshalLogText( const Seperator& cmd )
{
    const char* cmdName = cmd.arg( 0 ).c_str();

    if( 1 == cmd.argCount() )
    {
        sLog.Error( cmdName, "Usage: %s marshal-binary [marshal-binary] ...", cmdName );
        return;
    }

    for( size_t i = 1; i < cmd.argCount(); ++i )
    {
        const std::string& marshalBinaryStr = cmd.arg( i );

        Buffer marshalBinary;
        if( !PyDecodeEscape( marshalBinaryStr.c_str(), marshalBinary ) )
        {
            sLog.Error( cmdName, "Failed to decode string into binary." );
            continue;
        }

        PyRep* r = InflateUnmarshal( marshalBinary );
        if( NULL == r )
            sLog.Error( cmdName, "Failed to unmarshal binary." );
        else
        {
            sLog.Success( cmdName, "Result:" );
            r->Dump( stdout, "    " );

            PyDecRef( r );
        }
    }
}

#pragma pack( 1 )
struct FileHeader
{
    uint32 file_size;
    uint32 path_len;
};
#pragma pack()

struct FileHeaderObj
{
    uint32 length;
    size_t offset;
    std::string filename;
};

void StuffExtract( const Seperator& cmd )
{
    const char* cmdName = cmd.arg( 0 ).c_str();

    if( 2 != cmd.argCount() )
    {
        sLog.Error( cmdName, "Usage: %s [.stuff file]", cmdName );
        return;
    }
    const std::string& filename = cmd.arg( 1 );

    FILE* in = fopen( filename.c_str(), "rb" );
    if( NULL == in )
    {
        sLog.Error( cmdName, "Unable to open %s.", filename.c_str() );
        return;
    }

    uint32 file_count;
    if( 1 != fread( &file_count, sizeof( file_count ), 1, in ) )
    {
        sLog.Log( cmdName, "Unable to read file count." );
        return;
    }

    sLog.Log( cmdName, "There are %u files in %s.", file_count, filename.c_str() );

    std::vector<FileHeaderObj> headers;
    headers.resize( file_count );

    size_t offset = 0;
    for( uint32 i = 0; i < file_count; i++ )
    {
        FileHeader head;
        if( 1 != fread( &head, sizeof( head ), 1, in ) )
        {
            sLog.Error( cmdName, "Unable to read header of file #%u.", i );
            return;
        }

        FileHeaderObj& obj = headers[ i ];
        obj.length = head.file_size;
        obj.offset = offset;

        ++head.path_len; // NULL term

        //read path name.
        obj.filename.resize( head.path_len );
        if( head.path_len != fread( &obj.filename[0], 1, head.path_len, in ) )
        {
            sLog.Error( cmdName, "Unable to read name of file #%u.", i );
            return;
        }
        //drop read NULL term
        obj.filename.resize( head.path_len - 1 );

        sLog.Log( cmdName, "File #%u has length %u and path %s.", i, obj.length, obj.filename.c_str() );
        offset += head.file_size;
    }

    std::vector<FileHeaderObj>::const_iterator cur, end;
    cur = headers.begin();
    end = headers.end();
    for(; cur != end; cur++)
    {
        //split the path into components, make the intermediate directories
        //if needed, and then finally open the file and write it out.
        std::vector<std::string> components;
        SplitPath( cur->filename, components );

        std::string pathname = ".";

        std::vector<std::string>::const_iterator curc, endc;
        curc = components.begin();
        endc = components.end();
        for(; curc != endc; curc++ )
        {
            pathname += "/";
            pathname += *curc;

            if( ( curc + 1 ) == endc )
            {
                //this is the file component.
                FILE* out = fopen( pathname.c_str(), "wb" );
                if( NULL == out )
                {
                    sLog.Error( cmdName, "Unable to create file %s: %s.", pathname.c_str(), strerror( errno ) );
                    break;
                }

                sLog.Log( cmdName, "Extracting file %s of length %u.", pathname.c_str(), cur->length );

                Buffer buf( cur->length );
                if( cur->length != fread( &buf[0], 1, cur->length, in ) )
                {
                    sLog.Error( cmdName, "Unable to read file %s: %s.", cur->filename.c_str(), strerror( errno ) );
                    break;
                }

                if( cur->length != fwrite( &buf[0], 1, cur->length, out ) )
                {
                    sLog.Error( cmdName, "Unable to write file %s: %s", pathname.c_str(), strerror( errno ) );
                    break;
                }

                fclose( out );
            }
            else
            {
                //this is an intermediate directory.
                struct stat stat_struct;

                if( -1 == stat( pathname.c_str(), &stat_struct ) )
                {
                    if( ENOENT == errno )
                    {
                        if( -1 == mkdir( pathname.c_str(), ( S_IRWXU | S_IRWXG | S_IRWXO ) ) )
                        {
                            sLog.Error( cmdName, "Failed to make intermediate directory %s: %s", pathname.c_str(), strerror( errno ) );
                            break;
                        }
                    }
                    else
                    {
                        sLog.Error( cmdName, "Unable to stat %s: %s", pathname.c_str(), strerror( errno ) );
                        break;
                    }
                }
            }
        }
    }

    fclose( in );

    sLog.Log( cmdName, "Extracting from archive %s finished.", filename.c_str() );
}

