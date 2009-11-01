/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2008 The EVEmu Team
    For the latest information visit http://evemu.mmoforge.org
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

#include "Commands.h"

/************************************************************************/
/* Command declaration                                                  */
/************************************************************************/
void DestinyDumpLogText( const Seperator& cmd );
void ObjectToSQL( const Seperator& cmd );
void TestMarshal( const Seperator& cmd );
void PrintTimeNow( const Seperator& cmd );
void TimeToString( const Seperator& cmd );
void TriToOBJ( const Seperator& cmd );
void UnmarshalLogText( const Seperator& cmd );
void StuffExtract( const Seperator& cmd );

const EVEToolCommand EVETOOL_COMMANDS[] =
{
    { "destiny",   &DestinyDumpLogText },
    { "obj2sql",   &ObjectToSQL        },
    { "mtest",     &TestMarshal        },
    { "now",       &PrintTimeNow       },
    { "time",      &TimeToString       },
    { "tri2obj",   &TriToOBJ           },
    { "unmarshal", &UnmarshalLogText   },
    { "xstuff",    &StuffExtract       }
};
const size_t EVETOOL_COMMAND_COUNT = ( sizeof( EVETOOL_COMMANDS ) / sizeof( EVEToolCommand ) );

/************************************************************************/
/* Command implementation                                               */
/************************************************************************/
void DestinyDumpLogText( const Seperator& cmd )
{
    Buffer data;
    if( !py_decode_escape( cmd.argplus[1], data ) )
        return;

    Destiny::DumpUpdate( DESTINY__MESSAGE, &data[0], data.size() );
}

void ObjectToSQL( const Seperator& cmd )
{
    if(cmd.argnum != 4) {
        printf("Usage: obj2sql [cache_file] [table_name] [key_field] [file_name]\n");
        return;
    }

    std::string abs_fname( "../data/cache/" );
    abs_fname += cmd.arg[1];
    abs_fname += ".cache";
    printf("Converting cached object %s:\n", abs_fname.c_str());

    CachedObjectMgr mgr;
    PyCachedObjectDecoder *obj = mgr.LoadCachedObject(abs_fname.c_str(), cmd.arg[1]);
    if(obj == NULL) {
        _log(CLIENT__ERROR, "Unable to load or decode '%s'!", abs_fname.c_str());
        return;
    }

    obj->cache->DecodeData();
    if(obj->cache->decoded() == NULL) {
        _log(CLIENT__ERROR, "Unable to load or decode body of '%s'!", abs_fname.c_str());
        delete obj;
        return;
    }

    FILE *out = fopen(cmd.arg[4], "w");
    if(out == NULL) {
        _log(CLIENT__ERROR, "Unable to open output file '%s'", cmd.arg[4]);
        delete obj;
        return;
    }

    bool success = false;
    if(obj->cache->decoded()->IsObject()) {
        util_Rowset rowset;
        if( !rowset.Decode( obj->cache->decoded() ) )
        {
            _log(CLIENT__ERROR, "Unable to load a rowset from the object body!");
            delete obj;
            return;
        }
    
        RowsetReader reader(&rowset);
        success = ReaderToSQL<RowsetReader>(cmd.arg[2], cmd.arg[3], out, reader);
    } else if(obj->cache->decoded()->IsTuple()) {
        util_Tupleset rowset;
        if( !rowset.Decode( obj->cache->decoded() ) )
        {
            _log(CLIENT__ERROR, "Unable to load a tupleset from the object body!");
            delete obj;
            return;
        }

        TuplesetReader reader(&rowset);
        success = ReaderToSQL<TuplesetReader>(cmd.arg[2], cmd.arg[3], out, reader);
    } else {
        _log(CLIENT__ERROR, "Unknown cache body type: %s", obj->cache->decoded()->TypeString());
    }

    fclose(out);
    delete obj;
    if(success)
        printf("Success.\n");
}

void TestMarshal( const Seperator& cmd )
{
    DBRowDescriptor *header = new DBRowDescriptor;
    // Fill header:
    header->AddColumn( "historyDate", DBTYPE_FILETIME );
    header->AddColumn( "lowPrice", DBTYPE_CY );
    header->AddColumn( "highPrice", DBTYPE_CY );
    header->AddColumn( "avgPrice", DBTYPE_CY );
    header->AddColumn( "volume", DBTYPE_I8 );
    header->AddColumn( "orders", DBTYPE_I4 );
    
    CRowSet* rs = new CRowSet( &header );

    PyPackedRow& row = rs->NewRow();
    row.SetField( "historyDate", new PyLong( Win32TimeNow() ) );
    row.SetField( "lowPrice", new PyLong( 18000 ) );
    row.SetField( "highPrice", new PyLong( 19000 ) );
    row.SetField( "avgPrice", new PyLong( 18400 ) );
    row.SetField( "volume", new PyLong( 5463586 ) );
    row.SetField( "orders", new PyInt( 254 ) );

    sLog.Log( "mtest", "Marshaling..." );

    Buffer* marshaled = MarshalDeflate( rs );
    PyDecRef( rs );
    if( marshaled == NULL )
    {
        sLog.Error( "mtest", "Failed to marshal Python object." );
        return;
    }

    sLog.Log( "mtest", "Unmarshaling..." );

    PyRep* rep = InflateUnmarshal( *marshaled );
    SafeDelete( marshaled );
    if( rep == NULL )
    {
        sLog.Error( "mtest", "Failed to unmarshal Python object." );
        return;
    }

    sLog.Success( "mtest", "Final:" );
    rep->Dump( stdout, "    " );

    PyDecRef( rep );
}

void PrintTimeNow( const Seperator& cmd )
{
    sLog.Log( "now", "Now in Win32 time: "I64u".", Win32TimeNow() );
}

void TimeToString( const Seperator& cmd )
{
    uint64 t;
#ifndef WIN32   // Make GCC happy.
    sscanf( cmd.arg[1], I64u, (unsigned long long int*)&t );
#else
    sscanf( cmd.arg[1], I64u, &t );
#endif

    sLog.Log( "time", I64u" is %s.", t, Win32TimeToString( t ).c_str() );
}

void TriToOBJ( const Seperator& cmd )
{
    if(cmd.argnum != 3) {
        printf("Usage: tri2obj [trifile] [objout] [mtlout]\n");
        return;
    }

    TriExporter::TriFile f;
    if(!f.LoadFile(cmd.arg[1])) {
        printf("Failed to load TRI file.\n");
        return;
    }

    f.DumpHeaders();
    f.ExportObj(cmd.arg[2], cmd.arg[3]);
    printf("%s - %s - written.\n", cmd.arg[2], cmd.arg[3]);
}

void UnmarshalLogText( const Seperator& cmd )
{
    Buffer data;
    if( !py_decode_escape( cmd.argplus[1], data ) )
    {
        sLog.Error( "unmarshal", "Failed to decode string into binary." );
        return;
    }

    PyRep* r = InflateUnmarshal( data );
    if( r == NULL )
        sLog.Error( "unmarshal", "Failed to unmarshal binary." );
    else
    {
        sLog.Success( "unmarshal", "Result:" );

        r->Dump( stdout, "    " );
    }
}

void StuffExtract( const Seperator& cmd )
{
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

    if( cmd.argnum != 1 )
    {
        sLog.Error( "xstuff", "Usage: %s [.stuff file]", cmd.arg[0] );
        return;
    }

    FILE* in = fopen( cmd.arg[1], "rb" );
    if( in == NULL )
    {
        sLog.Error( "xstuff", "Unable to open %s.", cmd.arg[1] );
        return;
    }

    uint32 file_count;
    if( fread( &file_count, sizeof( file_count ), 1, in ) != 1 )
    {
        sLog.Log( "xstuff", "Unable to read file count." );
        return;
    }

    sLog.Log( "xstuff", "There are %u files in %s.", file_count, cmd.arg[1] );

    std::vector<FileHeaderObj> headers;
    headers.resize( file_count );

    size_t offset = 0;
    for( uint32 i = 0; i < file_count; i++ )
    {
        FileHeader head;
        if( fread( &head, sizeof( head ), 1, in ) != 1 )
        {
            sLog.Error( "xstuff", "Unable to read header of file #%u.", i );
            return;
        }

        FileHeaderObj& obj = headers[ i ];
        obj.length = head.file_size;
        obj.offset = offset;

        ++head.path_len; // NULL term

        //read path name.
        obj.filename.resize( head.path_len );
        if( fread( &obj.filename[0], 1, head.path_len, in ) != head.path_len )
        {
            sLog.Error( "xstuff", "Unable to read name of file #%u.", i );
            return;
        }
        //drop read NULL term
        obj.filename.resize( head.path_len - 1 );

        sLog.Log( "xstuff", "File #%u has length %u and path %s.", i, obj.length, obj.filename.c_str() );
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
        split_path( cur->filename, components );

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
                if( out == NULL )
                {
                    sLog.Error( "xstuff", "Unable to create file %s: %s.", pathname.c_str(), strerror( errno ) );
                    break;
                }

                sLog.Log( "xstuff", "Extracting file %s of length %u.", pathname.c_str(), cur->length );

                char* buffer = new char[ cur->length ];
                if( fread( buffer, 1, cur->length, in ) != cur->length )
                {
                    sLog.Error( "xstuff", "Unable to read file %s: %s.", cur->filename.c_str(), strerror( errno ) );
                    break;
                }

                if( fwrite( buffer, 1, cur->length, out ) != cur->length )
                {
                    sLog.Error( "xstuff", "Unable to write file %s: %s", pathname.c_str(), strerror( errno ) );
                    break;
                }

                SafeDeleteArray( buffer );
                fclose( out );
            }
            else
            {
                //this is an intermediate directory.
                struct stat stat_struct;

                if( stat( pathname.c_str(), &stat_struct ) == -1 )
                {
                    if( errno == ENOENT )
                    {
                        if( mkdir( pathname.c_str(), ( S_IRWXU | S_IRWXG | S_IRWXO ) ) == -1 )
                        {
                            sLog.Error( "xstuff", "Failed to make intermediate directory %s: %s", pathname.c_str(), strerror( errno ) );
                            break;
                        }
                    }
                    else
                    {
                        sLog.Error( "xstuff", "Unable to stat %s: %s", pathname.c_str(), strerror( errno ) );
                        break;
                    }
                }
            }
        }
    }

    fclose( in );

    sLog.Log( "xstuff", "Extracting from archive %s finished.", cmd.arg[1] );
}

