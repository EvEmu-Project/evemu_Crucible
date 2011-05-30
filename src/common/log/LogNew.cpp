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
    Author:     Captnoord
*/

#include "CommonPCH.h"

#include "log/LogNew.h"
#include "log/logtypes.h"
#include "log/logsys.h"

/*************************************************************************/
/* NewLog                                                                */
/*************************************************************************/
#ifdef WIN32
const WORD NewLog::COLOR_TABLE[ COLOR_COUNT ] =
{
    ( FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE                        ), // COLOR_DEFAULT
    ( 0                                                                          ), // COLOR_BLACK
    ( FOREGROUND_RED                                      | FOREGROUND_INTENSITY ), // COLOR_RED
    (                  FOREGROUND_GREEN                   | FOREGROUND_INTENSITY ), // COLOR_GREEN
    ( FOREGROUND_RED | FOREGROUND_GREEN                   | FOREGROUND_INTENSITY ), // COLOR_YELLOW
    (                                     FOREGROUND_BLUE | FOREGROUND_INTENSITY ), // COLOR_BLUE
    ( FOREGROUND_RED                    | FOREGROUND_BLUE | FOREGROUND_INTENSITY ), // COLOR_MAGENTA
    (                  FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY ), // COLOR_CYAN
    ( FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY )  // COLOR_WHITE
};
#else /* !WIN32 */
const char* const NewLog::COLOR_TABLE[ COLOR_COUNT ] =
{
    "\033[" "00"    "m", // COLOR_DEFAULT
    "\033[" "30;22" "m", // COLOR_BLACK
    "\033[" "31;22" "m", // COLOR_RED
    "\033[" "32;22" "m", // COLOR_GREEN
    "\033[" "33;01" "m", // COLOR_YELLOW
    "\033[" "34;01" "m", // COLOR_BLUE
    "\033[" "35;22" "m", // COLOR_MAGENTA
    "\033[" "36;01" "m", // COLOR_CYAN
    "\033[" "37;01" "m"  // COLOR_WHITE
};
#endif /* !WIN32 */

NewLog::NewLog()
: mLogfile( NULL ),
  mTime( 0 )
#ifdef WIN32
  ,mStdOutHandle( GetStdHandle( STD_OUTPUT_HANDLE ) ),
  mStdErrHandle( GetStdHandle( STD_ERROR_HANDLE ) )
#endif /* WIN32 */
{
    // open default logfile
    SetLogfileDefault();

    Debug( "Log", "Log system initiated" );
}

NewLog::~NewLog()
{
    Debug( "Log", "Log system shutting down" );

    // close logfile
    SetLogfile( (FILE*)NULL );
}

void NewLog::Log( const char* source, const char* fmt, ... )
{
    va_list ap;
    va_start( ap, fmt );

    PrintMsg( COLOR_DEFAULT, 'L', source, fmt, ap );

    va_end( ap );
}

void NewLog::Error( const char* source, const char* fmt, ... )
{
    va_list ap;
    va_start( ap, fmt );

    PrintMsg( COLOR_RED, 'E', source, fmt, ap );

    va_end( ap );
}

void NewLog::Warning( const char* source, const char* fmt, ... )
{
    va_list ap;
    va_start( ap, fmt );

    PrintMsg( COLOR_YELLOW, 'W', source, fmt, ap );

    va_end( ap );
}

void NewLog::Success( const char* source, const char* fmt, ... )
{
    va_list ap;
    va_start( ap, fmt );

    PrintMsg( COLOR_GREEN, 'S', source, fmt, ap );

    va_end( ap );
}

void NewLog::Debug( const char* source, const char* fmt, ... )
{
//#ifndef NDEBUG
    if( is_log_enabled( DEBUG__DEBUG ) )
    {
        va_list ap;
        va_start( ap, fmt );

        PrintMsg( COLOR_CYAN, 'D', source, fmt, ap );

        va_end( ap );
    }
//#endif /* !NDEBUG */
}

bool NewLog::SetLogfile( const char* filename )
{
    MutexLock l( mMutex );

    FILE* file = NULL;

    if( NULL != filename )
    {
        file = fopen( filename, "w" );
        if( NULL == file )
            return false;
    }

    return SetLogfile( file );
}

bool NewLog::SetLogfile( FILE* file )
{
    MutexLock l( mMutex );

    if( NULL != mLogfile )
        assert( 0 == fclose( mLogfile ) );

    mLogfile = file;
    return true;
}

void NewLog::PrintMsg( Color color, char pfx, const char* source, const char* fmt, va_list ap )
{
    MutexLock l( mMutex );

    PrintTime();

    SetColor( color );
    Print( " %c ", pfx );

    if( source && *source )
    {
        SetColor( COLOR_WHITE );
        Print( "%s: ", source );

        SetColor( color );
    }

    PrintVa( fmt, ap );
    Print( "\n" );

    SetColor( COLOR_DEFAULT );
}

void NewLog::PrintTime()
{
    MutexLock l( mMutex );

    // this will be replaced my a timing thread somehow
    SetTime( time( NULL ) );

    tm t;
    localtime_r( &mTime, &t );

    Print( "%02u:%02u:%02u", t.tm_hour, t.tm_min, t.tm_sec );
}

void NewLog::Print( const char* fmt, ... )
{
    va_list ap;
    va_start( ap, fmt );

    PrintVa( fmt, ap );

    va_end( ap );
}

void NewLog::PrintVa( const char* fmt, va_list ap )
{
    MutexLock l( mMutex );

    if( NULL != mLogfile )
    {
        // this is a design flaw ( UNIX related )
        va_list ap2;
        va_copy( ap2, ap );

        vfprintf( mLogfile, fmt, ap2 );

#ifndef NDEBUG
        // flush immediately so logfile is accurate if we crash
        fflush( mLogfile );
#endif /* !NDEBUG */

        va_end( ap2 );
    }

    vprintf( fmt, ap );
}

void NewLog::SetColor( Color color )
{
    assert( 0 <= color && color < COLOR_COUNT );

    MutexLock l( mMutex );

#ifdef WIN32
    SetConsoleTextAttribute( mStdOutHandle, COLOR_TABLE[ color ] );
#else /* !WIN32 */
    fputs( COLOR_TABLE[ color ], stdout );
#endif /* !WIN32 */
}

void NewLog::SetLogfileDefault()
{
    MutexLock l( mMutex );

    // set initial log system time
    SetTime( time( NULL ) );

    tm t;
    localtime_r( &mTime, &t );

    // open default logfile
    char filename[ MAX_PATH + 1 ];
    snprintf( filename, MAX_PATH + 1, EVEMU_ROOT_DIR"log/log_%02u-%02u-%04u-%02u-%02u.log",
              t.tm_mday, t.tm_mon + 1, t.tm_year + 1900, t.tm_hour, t.tm_min );

    if( SetLogfile( filename ) )
        Success( "Log", "Opened logfile '%s'.", filename );
    else
        Warning( "Log", "Unable to open logfile '%s': %s", filename, strerror( errno ) );
}
