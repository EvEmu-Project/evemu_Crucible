/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://evemu.dev
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
    Author:     Zhur
*/

#include "eve-common.h"

#include "python/classes/PyDatabase.h"
#include "python/PyVisitor.h"
#include "python/PyRep.h"
#include "python/PyDumpVisitor.h"
#include "utils/EVEUtils.h"

/************************************************************************/
/* PyDumpVisitor                                                        */
/************************************************************************/
PyDumpVisitor::PyDumpVisitor( const char* pfx, bool full_nested ) : PyPfxVisitor( pfx ), mFullNested( full_nested ) {}

bool PyDumpVisitor::VisitInteger( const PyInt* rep )
{
    _print( "%s   Integer: %i", _pfx(), rep->value() );
    return true;
}

bool PyDumpVisitor::VisitLong( const PyLong* rep )
{
    _print( "%s      Long: %" PRIi64, _pfx(), rep->value() );
    return true;
}

bool PyDumpVisitor::VisitReal( const PyFloat* rep )
{
    _print( "%s      Real: %f", _pfx(), rep->value() );
    return true;
}

bool PyDumpVisitor::VisitBoolean( const PyBool* rep )
{
    _print( "%s   Boolean: %s", _pfx(), rep->value() ? "true" : "false" );
    return true;
}

bool PyDumpVisitor::VisitNone( const PyNone* rep )
{
    _print( "%s      None", _pfx() );
    return true;
}

bool PyDumpVisitor::VisitBuffer( const PyBuffer* rep )
{
    _print( "%s Buffer (%llu):", _pfx(), rep->content().size() );

    _pfxExtend( "  " );
    _dump( _pfx(), &rep->content()[ 0 ], rep->content().size() );
    _pfxWithdraw();

    return true;
}

bool PyDumpVisitor::VisitString( const PyString *rep )
{
    if (IsPrintable( rep ) )
        _print( "%s    String: '%s'", _pfx(), rep->content().c_str() );
    else
        _print( "%sBinary String: (len=%llu)", _pfx(), rep->content().length() );

    return true;
}

bool PyDumpVisitor::VisitWString( const PyWString* rep )
{
    // how to do it correctly?
    if (IsPrintable( rep ) )
        _print( "%s   WString: '%s'", _pfx(), rep->content().c_str() );
    else
        _print( "%ssBinary WString: (len=%llu)'", _pfx(), rep->content().length() );

    return true;
}

bool PyDumpVisitor::VisitToken( const PyToken* rep )
{
    _print( "%s     Token: '%s'", _pfx(), rep->content().c_str() );

    return true;
}

bool PyDumpVisitor::VisitTuple( const PyTuple* rep )
{
    bool res(true);
    if (rep->empty())
        _print( "%s Tuple: Empty", _pfx() );
    else  {
        _print( "%s Tuple: %llu elements", _pfx(), rep->size() );

        PyTuple::const_iterator cur = rep->begin(), end = rep->end();
        for ( uint8 i(0); cur != end; ++cur, ++i )  {
            if (*cur == nullptr)
                continue;
            if (i > 100 && !fullNested()) {
                _print( "%s  ... truncated ...", _pfx() );
                break;
            }

            _pfxExtend( "  [%2u] ", i );
            res = (*cur)->visit( *this );
            _pfxWithdraw();
        }
    }
    return res;
}

bool PyDumpVisitor::VisitList( const PyList* rep )
{
    bool res(true);
    if (rep->empty())
        _print( "%s  List: Empty", _pfx() );
    else {
        _print( "%s  List: %llu elements", _pfx(), rep->size() );

        PyList::const_iterator cur = rep->begin(), end = rep->end();
        for (uint8 i(0); cur != end; ++cur, ++i )  {
            if (*cur == nullptr)
                continue;
            if (i > 100 && !fullNested()) {
                _print( "%s  ... truncated ...", _pfx() );
                break;
            }

            _pfxExtend( "  [%2u] ", i );
            res = (*cur)->visit( *this );
            _pfxWithdraw();
        }
    }
    return res;
}

bool PyDumpVisitor::VisitDict( const PyDict* rep )
{
    if (rep->empty())
        _print( "%s Dictionary: Empty", _pfx() );
    else {
        _print( "%s Dictionary: %llu entries", _pfx(), rep->size() );

        PyDict::const_iterator cur = rep->begin(), end = rep->end();
        for (uint8 i(0); cur != end; ++cur, ++i )  {
            if (i > 100 && !fullNested() ) {
                _print( "%s  ... truncated ...", _pfx() );
                break;
            }

            _pfxExtend( "  [%2u]   Key: ", i );
            bool res = cur->first->visit( *this );
            _pfxWithdraw();

            if (!res )
                return false;

            _pfxExtend( "  [%2u] Value: ", i );
            res = cur->second->visit( *this );
            _pfxWithdraw();

            if (!res )
                return false;
        }
    }
    return true;
}

bool PyDumpVisitor::VisitObject( const PyObject* rep )
{
    _print( "%sObject:", _pfx() );

    _pfxExtend( "  Type: " );
    bool res = rep->type()->visit( *this );
    _pfxWithdraw();

    if (!res )
        return false;

    _pfxExtend( "  Args: " );
    res = rep->arguments()->visit( *this );
    _pfxWithdraw();

    return res;
}

bool PyDumpVisitor::VisitObjectEx( const PyObjectEx* rep )
{
    _print( "%s ObjectEx%s:", _pfx(), rep->isType2() ? "2" : "1" );

    _print( "%s    Header:", _pfx() );
    if (rep->header() == nullptr ) {
        _print( "%s (None)", _pfx() );
    } else {
        _pfxExtend( "  " );
        bool res = rep->header()->visit( *this );
        _pfxWithdraw();

        if (!res )
            return false;
    }

    _print( "%s  List:", _pfx() );
    if (rep->list().empty()) {
        _print( "%s  Empty", _pfx() );
    } else {
        PyList::const_iterator lItr = rep->list().begin(), lEnd = rep->list().end();
        for(uint8 i(0); lItr != lEnd; ++lItr, ++i ) {
            if (*lItr == nullptr)
                continue;
            if (i > 100 && !fullNested() ) {
                _print( "%s  ... truncated ...", _pfx() );
                break;
            }

            _pfxExtend( "  [%2u] ", i );
            bool res = (*lItr)->visit( *this );
            _pfxWithdraw();

            if (!res )
                return false;
        }
    }

    _print( "%s    Dict:", _pfx() );
    if (rep->dict().empty()) {
        _print( "%s   Empty", _pfx() );
    } else {
        PyDict::const_iterator dItr = rep->dict().begin(), dEnd = rep->dict().end();
        for(uint8 i(0); dItr != dEnd; ++dItr, ++i ) {
            if (i > 100 && !fullNested() ) {
                _print( "%s  ... truncated ...", _pfx() );
                break;
            }

            _pfxExtend( "  [%2u]   Key: ", i );
            bool res = dItr->first->visit( *this );
            _pfxWithdraw();

            if (!res)
                return false;

            _pfxExtend( "  [%2u] Value: ", i );
            res = dItr->second->visit( *this );
            _pfxWithdraw();

            if (!res)
                return false;
        }
    }

    return true;
}

bool PyDumpVisitor::VisitPackedRow( const PyPackedRow* rep )
{
    _print( "%sPacked Row:", _pfx() );
    _print( "%scolumn_count=%u", _pfx(), rep->header()->ColumnCount() );

    PyList::const_iterator itr = rep->begin(), end = rep->end();
    for (uint32 i = 0; itr != end; ++itr, ++i) {
        _pfxExtend( "    [%2u] %s: ", i, rep->header()->GetColumnName( i )->content().c_str() );

        bool res(true);
        if ((*itr) == nullptr )
            _print( "%s  (None)", _pfx() );
        else
            res = (*itr)->visit( *this );

        _pfxWithdraw();

        if (!res)
            return false;
    }

    return true;
}

bool PyDumpVisitor::VisitSubStruct( const PySubStruct* rep )
{
    _print( "%s Substruct:", _pfx() );

    _pfxExtend( "  " );
    bool res = PyVisitor::VisitSubStruct( rep );
    _pfxWithdraw();

    return res;
}

bool PyDumpVisitor::VisitSubStream( const PySubStream* rep )
{
    _print( "%s Substream: %s", _pfx(), ( rep->decoded() == nullptr ) ? "from data" : "from rep" );

    _pfxExtend( "  " );
    bool res = PyVisitor::VisitSubStream( rep );
    _pfxWithdraw();

    return res;
}

bool PyDumpVisitor::VisitChecksumedStream( const PyChecksumedStream* rep )
{
    _print( "%sChecksumStream: 0x%08x", _pfx(), rep->checksum() );

    _pfxExtend( "  " );
    bool res = PyVisitor::VisitChecksumedStream( rep );
    _pfxWithdraw();

    return res;
}

PyLogDumpVisitor::PyLogDumpVisitor( LogType log_type, LogType log_hex_type, const char* pfx, bool full_nested, bool full_hex )
: PyDumpVisitor( pfx, full_nested ),
  mFullHex( full_hex ),
  mLogType( log_type ),
  mLogHexType( log_hex_type )
{
}

void PyLogDumpVisitor::_print( const char* fmt, ... )
{
    if (!is_log_enabled( logType() ) )
        return;

    va_list ap;
    va_start( ap, fmt );

    log_messageVA( logType(), fmt, ap );

    va_end( ap );
}

void PyLogDumpVisitor::_dump( const char* pfx, const uint8* data, size_t len )
{
    if (!is_log_enabled( logHexType() ) )
        return;

    if (fullHex() )
        pfxHexDump( pfx, logHexType(), data, len );
    else
        pfxHexDumpPreview( pfx, logHexType(), data, len );
}

PyFileDumpVisitor::PyFileDumpVisitor( FILE* _file, const char* pfx, bool full_nested/*false*/, bool full_hex/*false*/ )
: PyDumpVisitor( pfx, full_nested ),
  mFullHex( full_hex ),
  mFile( _file )
{
}

void PyFileDumpVisitor::_print( const char* fmt, ... )
{
    va_list ap;
    va_start( ap, fmt );

    vfprintf( file(), fmt, ap );
    fprintf( file(), "\n" );

    va_end( ap );
}

void PyFileDumpVisitor::_dump( const char* pfx, const uint8* data, size_t len )
{
    if (fullHex())
        pfxHexDump( pfx, file(), data, len );
    else
        pfxHexDumpPreview( pfx, file(), data, len );
}
