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
    Author:     Captnoord
*/

#include "CommonPCH.h"

#include "log/LogNew.h"

EXPORT_SINGLETON( NewLog );

// console output colors
#ifdef WIN32
#  define TRED    FOREGROUND_RED | FOREGROUND_INTENSITY
#  define TGREEN  FOREGROUND_GREEN | FOREGROUND_INTENSITY
#  define TYELLOW FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY
#  define TNORMAL FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE
#  define TWHITE  TNORMAL | FOREGROUND_INTENSITY
#  define TBLUE   FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY
#else /* !WIN32 */
#  define TRED 1
#  define TGREEN 2
#  define TYELLOW 3
#  define TNORMAL 4
#  define TWHITE 5
#  define TBLUE 6

const char* colorstrings[ TBLUE + 1 ] =
{
    "",
    "\033[22;31m",
    "\033[22;32m",
    "\033[01;33m",
    //"\033[22;37m",
    "\033[0m",
    "\033[01;37m",
    "\033[1;34m",
};
#endif /* !WIN32 */

NewLog::NewLog()
: mLogfile( NULL ),
  mTime( 0 )
#ifdef WIN32
  ,mStdoutHandle( GetStdHandle( STD_OUTPUT_HANDLE ) ),
  mStderrHandle( GetStdHandle( STD_ERROR_HANDLE ) )
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
    PrintTime();

    SetColor( TNORMAL );
    Print( " L " );

    if( source && *source )
    {
        SetColor( TWHITE );
        Print( "%s: ", source );

        SetColor( TNORMAL );
    }

    va_list ap;
    va_start( ap, fmt );

    PrintVa( fmt, ap );
    Print( "\n" );

    va_end( ap );

    SetColor( TNORMAL );
}

void NewLog::Error( const char* source, const char* fmt, ... )
{
    PrintTime();

    SetColor( TRED );
    Print( " E " );

    if( source && *source )
    {
        SetColor( TWHITE );
        Print( "%s: ", source );

        SetColor( TRED );
    }

    va_list ap;
    va_start( ap, fmt );

    PrintVa( fmt, ap );
    Print( "\n" );

    va_end( ap );

    SetColor( TNORMAL );
}

void NewLog::Warning( const char* source, const char* fmt, ... )
{
    PrintTime();

    SetColor( TYELLOW );
    Print( " W " );

    if( source && *source )
    {
        SetColor( TWHITE );
        Print( "%s: ", source );

        SetColor( TYELLOW );
    }

    va_list ap;
    va_start( ap, fmt );

    PrintVa( fmt, ap );
    Print( "\n" );

    va_end( ap );

    SetColor( TNORMAL );
}

void NewLog::Success( const char* source, const char* fmt, ... )
{
    PrintTime();

    SetColor( TGREEN );
    Print( " S " );

    if( source && *source )
    {
        SetColor( TWHITE );
        Print( "%s: ", source );

        SetColor( TGREEN );
    }

    va_list ap;
    va_start( ap, fmt );

    PrintVa( fmt, ap );
    Print( "\n" );

    va_end( ap );

    SetColor( TNORMAL );
}

void NewLog::Debug( const char* source, const char* fmt, ... )
{
#ifndef NDEBUG
    PrintTime();

    SetColor( TBLUE );
    Print( " D " );

    if( source && *source )
    {
        SetColor( TWHITE );
        Print( "%s: ", source );

        SetColor( TBLUE );
    }

    va_list ap;
    va_start( ap, fmt );

    PrintVa( fmt, ap );
    Print( "\n" );

    va_end( ap );

    SetColor( TNORMAL );
#endif /* !NDEBUG */
}

bool NewLog::SetLogfile( const char* filename )
{
    // if we got NULL, don't try to open it, pass it directly instead
    if( NULL == filename )
        return SetLogfile( (FILE*)NULL );

    // open the new logfile
    FILE* file = fopen( filename, "w" );
    if( NULL == file )
        return false;

    // pass the new logfile
    return SetLogfile( file );
}

bool NewLog::SetLogfile( FILE* file )
{
    // if we already have a logfile, close it first
    if( NULL != mLogfile )
        assert( 0 == fclose( mLogfile ) );

    // set the new logfile
    mLogfile = file;
    return true;
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

void NewLog::PrintTime()
{
    // this will be replaced my a timing thread somehow
    SetTime( time( NULL ) );

    tm t;
    localtime_r( &mTime, &t );

    Print( "%02u:%02u:%02u", t.tm_hour, t.tm_min, t.tm_sec );
}

void NewLog::SetColor( unsigned int color )
{
#ifdef WIN32
    SetConsoleTextAttribute( mStdoutHandle, (WORD)color );
#else /* !WIN32 */
    fputs( colorstrings[ color ], stdout );
#endif /* !WIN32 */
}

void NewLog::SetLogfileDefault()
{
    // set initial log system time
    SetTime( time( NULL ) );

    tm t;
    localtime_r( &mTime, &t );

    // open default logfile
    char filename[ MAX_PATH + 1 ];
    snprintf( filename, MAX_PATH + 1, "../log/log_%02u-%02u-%04u-%02u-%02u.log", t.tm_mday, t.tm_mon + 1, t.tm_year + 1900, t.tm_hour, t.tm_min );

    if( SetLogfile( filename ) )
        sLog.Success( "Log", "Opened logfile '%s'.", filename );
    else
        sLog.Warning( "Log", "Unable to open logfile '%s': %s", filename, strerror( errno ) );
}
