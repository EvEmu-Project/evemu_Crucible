#include "Common.h"

#include "LogNew.h"
#include <stdarg.h>
#include <time.h>

#ifndef WIN32
const char* colorstrings[TBLUE+1] = {
    "",
    "\033[22;31m",
    "\033[22;32m",
    "\033[01;33m",
    //"\033[22;37m",
    "\033[0m",
    "\033[01;37m",
    "\033[1;34m",
};
#endif//WIN32

#ifdef WIN32
#  define localtime_r localtime_s
#else
//#  define localtime_r localtime_r
#endif//WIN32

NewLog::NewLog()
{
#ifdef WIN32
    stderr_handle = GetStdHandle(STD_ERROR_HANDLE);
    stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
#endif//WIN32
    
    UNIXTIME = time(NULL);// waisting cpu cycles... lolz..

    char * log_filename = (char*)malloc(0x100);
    tm * t = NULL;
    localtime_r(t, &UNIXTIME);
    sprintf(log_filename, "log_%02u;%02u;%04u;%02u;%02u.txt", t->tm_mday, t->tm_mon + 1, t->tm_year + 1900, t->tm_hour, t->tm_min);

    m_logfile = fopen(log_filename, "w");
    if ( m_logfile == NULL )
    {
        /* this is because we haven't done shit so we can't do shit..*/
        SetColor( TRED );
        Time( stdout );
        printf( "E logging: unable to open: %s | creation of logging system is unsuccessful", log_filename );
        free( log_filename );
        return;
    }

    log("Log system initiated");

    free( log_filename );
}

void NewLog::SetColor(unsigned int color)
{
#ifndef WIN32
    fputs(colorstrings[color], stdout);
#else
    SetConsoleTextAttribute(stdout_handle, (WORD)color);
#endif
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

    vfprintf( m_logfile, format, ap );
    putchar('\n');
    va_end(ap);
    Color(TNORMAL);
    UNLOCK_LOG;
#endif//ENABLE_CONSOLE_LOG
}

#endif


void NewLog::Warning( const char * source, const char * str, ... )
{
#ifdef ENABLE_CONSOLE_LOG
    if(log_level < 2)
        return;

    /* warning is old loglevel 2/detail */
    LOCK_LOG;
    va_list ap;
    va_start(ap, format);
    Time( stdout );
    SetColor(TYELLOW);
    fputs("W ", stdout);
    if(*source)
    {
        SetColor(TWHITE);
        fputs(source, stdout);
        putchar(':');
        putchar(' ');
        SetColor(TYELLOW);
    }

    vfprintf( m_logfile, format, ap );
    putchar('\n');
    va_end(ap);
    SetColor(TNORMAL);
    UNLOCK_LOG;
#endif//ENABLE_CONSOLE_LOG
}
void NewLog::Success(const char * source, const char * format, ...)
{
    va_list ap;
    va_start(ap, format);
    Time( stdout ); Time( m_logfile );    
    SetColor( TGREEN );
    fputs("S ", stdout); fputs("S ", m_logfile);
    if(*source)
    {
        SetColor( TWHITE );
        fputs( source, stdout ); fputs( source, m_logfile );
        fputs( ": ", stdout ); fputs( ": ", m_logfile );
        SetColor( TGREEN );
    }

    vfprintf( stdout, format, ap ); vfprintf( m_logfile, format, ap );
    fputc( '\n', stdout ); fputc( '\n', m_logfile );
    va_end(ap);
    SetColor(TNORMAL);
}

void NewLog::Error( const char * source, const char * format, ... )
{
    va_list ap;
    va_start(ap, format);
    Time( stdout ); Time( m_logfile );
    SetColor( TRED );
    fputs("E ", stdout); fputs("E ", m_logfile);
    if( source && *source )
    {
        SetColor( TWHITE );
        fputs( source, stdout ); fputs( source, m_logfile );
        fputs( ": ", stdout ); fputs( ": ", m_logfile );
        SetColor( TRED );
    }

    vfprintf( stdout, format, ap ); vfprintf( m_logfile, format, ap );
    fputc( '\n', stdout ); fputc( '\n', m_logfile );
    va_end(ap);
    SetColor(TNORMAL);
}

void NewLog::SetTime( time_t time )
{
    UNIXTIME = time;
}

NewLog::~NewLog()
{
    log("Log system shutting down");
    if ( m_logfile != NULL)
        fclose( m_logfile );
}
void NewLog::Time( FILE* fp )
{
    tm * t = NULL;
    localtime_r(t, &UNIXTIME);
    fprintf( fp, "%02u:%02u:%02u ", t->tm_hour, t->tm_min, t->tm_sec );
}

void NewLog::log( const char * str, ... )
{
    va_list ap;
    va_start( ap, str );
    /* write time */
    Time( m_logfile );
    
    /* write source, which is internal logger */
    fputs( "I Logger:", m_logfile );
    vfprintf( m_logfile, str, ap );
    fputc( '\n', m_logfile );
    va_end( ap );

    /* this is crap but as these messages are high priority I simply don't care */
    fflush( m_logfile );
}

void NewLog::Log( const char * source, const char * str, ... )
{
    va_list ap;
    va_start(ap, str);
    Time( stdout ); Time( m_logfile );
    SetColor( TNORMAL );
    fputs("L ", stdout); fputs("L ", m_logfile);
    if( source && *source )
    {
        SetColor( TWHITE );
        fputs( source, stdout ); fputs( source, m_logfile );
        fputs( ": ", stdout ); fputs( ": ", m_logfile );
        SetColor( TNORMAL );
    }

    vfprintf( stdout, str, ap ); vfprintf( m_logfile, str, ap );
    fputc( '\n', stdout ); fputc( '\n', m_logfile );
    va_end(ap);
    SetColor(TNORMAL);

    fflush( m_logfile );
}

/*void NewLog::Line()
{
    LOCK_LOG;
    putchar('\n');
    UNLOCK_LOG;
}

void NewLog::Debug(const char * source, const char * format, ...)
{
#ifdef ENABLE_CONSOLE_LOG
    if(log_level < 3)
        return;

    LOCK_LOG;
    va_list ap;
    va_start(ap, format);
    Time();
    SetColor(TBLUE);
    fputs("D ", stdout);
    if(*source)
    {
        SetColor(TWHITE);
        fputs(source, stdout);
        putchar(':');
        putchar(' ');
        SetColor(TBLUE);
    }

    vfprintf( stdout, format, ap );
    putchar('\n');
    va_end(ap);
    SetColor(TNORMAL);
    UNLOCK_LOG;
#endif//ENABLE_CONSOLE_LOG
}*/
