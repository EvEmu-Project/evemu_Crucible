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
#ifdef WIN32
: mStdoutHandle( GetStdHandle( STD_OUTPUT_HANDLE ) ),
  mStderrHandle( GetStdHandle( STD_ERROR_HANDLE ) )
#endif /* WIN32 */
{
    // set initial log system time
    SetTime( time( NULL ) );

    tm t;
    localtime_r( &mTime, &t );

    char log_filename[ MAX_PATH + 1 ];
    snprintf( log_filename, MAX_PATH + 1, "eveserver_%02u-%02u-%04u-%02u-%02u.log", t.tm_mday, t.tm_mon + 1, t.tm_year + 1900, t.tm_hour, t.tm_min );

    mLogfile = fopen( log_filename, "w" );
    // we would crash as soon as anyone tried to print anything anyway
    assert( NULL != mLogfile );

    Debug( "Log", "Log system initiated" );
}

NewLog::~NewLog()
{
    Debug( "Log", "Log system shutting down" );

    if( NULL != mLogfile )
        fclose( mLogfile );
}

void NewLog::Log( const char* source, const char* fmt, ... )
{
    LogTime( stdout ); LogTime( mLogfile );

    SetColor( TNORMAL );
    fputs( "L ", stdout ); fputs( "L ", mLogfile );

    if( source && *source )
    {
        SetColor( TWHITE );
        fputs( source, stdout ); fputs( source, mLogfile );
        fputs( ": ", stdout ); fputs( ": ", mLogfile );
        SetColor( TNORMAL );
    }

    va_list ap, ap2;
    va_start( ap, fmt );
    va_copy( ap2, ap ); /* this is a design flaw ( UNIX related ) */

    vfprintf( stdout, fmt, ap ); vfprintf( mLogfile, fmt, ap2 );
    fputc( '\n', stdout ); fputc( '\n', mLogfile );

    va_end( ap );
    va_end( ap2 );

    SetColor( TNORMAL );
}

void NewLog::Error( const char* source, const char* fmt, ... )
{
    LogTime( stdout ); LogTime( mLogfile );

    SetColor( TRED );
    fputs( "E ", stdout ); fputs( "E ", mLogfile );

    if( source && *source )
    {
        SetColor( TWHITE );
        fputs( source, stdout ); fputs( source, mLogfile );
        fputs( ": ", stdout ); fputs( ": ", mLogfile );
        SetColor( TRED );
    }

    va_list ap, ap2;
    va_start( ap, fmt );
    va_copy( ap2, ap ); /* this is a design flaw ( UNIX related ) */

    vfprintf( stdout, fmt, ap ); vfprintf( mLogfile, fmt, ap2 );
    fputc( '\n', stdout ); fputc( '\n', mLogfile );

    va_end( ap );
    va_end( ap2 );

    SetColor( TNORMAL );
}

void NewLog::Warning( const char* source, const char* fmt, ... )
{
    LogTime( stdout ); LogTime( mLogfile );

    SetColor( TYELLOW );
    fputs( "W ", stdout ); fputs( "W ", mLogfile );

    if( source && *source )
    {
        SetColor( TWHITE );
        fputs( source, stdout ); fputs( source, mLogfile );
        fputs( ": ", stdout ); fputs( ": ", mLogfile );
        SetColor( TYELLOW );
    }

    va_list ap, ap2;
    va_start( ap, fmt );
    va_copy( ap2, ap ); /* this is a design flaw ( UNIX related ) */

    vfprintf( stdout, fmt, ap ); vfprintf( mLogfile, fmt, ap2 );
    fputc( '\n', stdout ); fputc( '\n', mLogfile );

    va_end( ap );
    va_end( ap2 );

    SetColor( TNORMAL );
}

void NewLog::Success( const char* source, const char* fmt, ... )
{
    LogTime( stdout ); LogTime( mLogfile );

    SetColor( TGREEN );
    fputs( "S ", stdout ); fputs( "S ", mLogfile );

    if( source && *source )
    {
        SetColor( TWHITE );
        fputs( source, stdout ); fputs( source, mLogfile );
        fputs( ": ", stdout ); fputs( ": ", mLogfile );
        SetColor( TGREEN );
    }

    va_list ap, ap2;
    va_start( ap, fmt );
    va_copy( ap2, ap ); /* this is a design flaw ( UNIX related ) */

    vfprintf( stdout, fmt, ap ); vfprintf( mLogfile, fmt, ap2 );
    fputc( '\n', stdout ); fputc( '\n', mLogfile );

    va_end( ap );
    va_end( ap2 );

    SetColor( TNORMAL );
}

#if 0
void NewLog::Notice(const char * source, const char * format, ...)
{
#ifdef ENABLE_CONSOLE_LOG
    /* notice is old loglevel 0/string */
    LOCK_LOG;
    va_list ap;
    va_start(ap, format);
    Time();
    fputs("N ", stdout);
    if(*source)
    {
        Color(TWHITE);
        fputs(source, stdout);
        putchar(':');
        putchar(' ');
        Color(TNORMAL);
    }

    vfprintf( mLogfile, format, ap );
    putchar('\n');
    va_end(ap);
    Color(TNORMAL);
    UNLOCK_LOG;
#endif//ENABLE_CONSOLE_LOG
}
#endif

void NewLog::Debug( const char* source, const char* fmt, ... )
{
#ifndef NDEBUG
    LogTime( stdout ); LogTime( mLogfile );

    SetColor( TBLUE );
    fputs( "D ", stdout ); fputs( "D ", mLogfile );

    if( source && *source )
    {
        SetColor( TWHITE );
        fputs( source, stdout ); fputs( source, mLogfile );
        fputs( ": ", stdout ); fputs( ": ", mLogfile );
        SetColor( TBLUE );
    }

    va_list ap, ap2;
    va_start( ap, fmt );
    va_copy( ap2, ap ); /* this is a design flaw ( UNIX related ) */

    vfprintf( stdout, fmt, ap ); vfprintf( mLogfile, fmt, ap2 );
    fputc( '\n', stdout ); fputc( '\n', mLogfile );

    va_end( ap );
    va_end( ap2 );

    SetColor( TNORMAL );
#endif /* !NDEBUG */
}

void NewLog::LogTime( FILE* fp )
{
    // this will be replaced my a timing thread somehow
    SetTime( time(NULL) );

    tm t;
    localtime_r( &mTime, &t );

    fprintf( fp, "%02u:%02u:%02u ", t.tm_hour, t.tm_min, t.tm_sec );
}

void NewLog::SetColor( unsigned int color )
{
#ifdef WIN32
    SetConsoleTextAttribute( mStdoutHandle, (WORD)color );
#else /* !WIN32 */
    fputs( colorstrings[ color ], stdout );
#endif /* !WIN32 */
}

