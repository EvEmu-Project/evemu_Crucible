#ifndef LOG_NEW_H
#define LOG_NEW_H

/** Small todo regarding the logging system
 * - set log file path and handle creation of "log" directory.
 */

#include "common.h"

// thread safe cross platform localtime stuff.
// needs to be moved to common.
#ifdef WIN32
#  define localtime_r(x,y) localtime_s(y,x)
#else
//#  define localtime_r localtime_r
#endif//WIN32

// console output colors
#ifdef WIN32
#  define TRED FOREGROUND_RED | FOREGROUND_INTENSITY
#  define TGREEN FOREGROUND_GREEN | FOREGROUND_INTENSITY
#  define TYELLOW FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY
#  define TNORMAL FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE
#  define TWHITE TNORMAL | FOREGROUND_INTENSITY
#  define TBLUE FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY
#else
#  define TRED 1
#  define TGREEN 2
#  define TYELLOW 3
#  define TNORMAL 4
#  define TWHITE 5
#  define TBLUE 6
#endif//WIN32

/**
 * \class NewLog
 *
 * @brief a small and simple logging system.
 *
 * This class is designed to be a simple logging system that both logs to file
 * and console regarding the settings.
 *
 * @author Captnoord.
 * @date August 2009
 */
class NewLog
{
public:
    NewLog();
    ~NewLog();
    /**
     * @brief Logs a message to file.
     *
     * @param[in] source is the source from where the message is printed.
     * @param[in] str is the message itself.
     */
    void Log( const char * source, const char * str, ... );

    /**
     * @brief Logs error message to console and file.
     *
     * @param[in] source is the source from where the message is printed.
     * @param[in] str is the error message itself.
     */
    void Error( const char * source, const char * err, ... );

    /**
     * @brief Logs a warning message to file.
     *
     * @param[in] source is the source from where the message is printed.
     * @param[in] str is the message itself.
     */
    void Warning( const char * source, const char * str, ... );

    /**
     * @brief Logs a success message to file.
     *
     * @param[in] source is the source from where the message is printed.
     * @param[in] str is the message itself.
     */
    void Success( const char * source, const char * format, ... );

    /**
     * @brief Logs a debug message to file and console.
     *
     * Logs a debug message to file and console and will be optimized out on a release build.
     *
     * @param[in] source is the source from where the message is printed.
     * @param[in] str is the message itself.
     */
    void Debug(const char * source, const char * format, ...);

    /**
     * @brief Sets the log system time every main loop.
     *
     * @param[in] time is the timestamp.
     */
    void SetTime( time_t time );
private:
#ifdef WIN32
    HANDLE stdout_handle, stderr_handle;
#endif//WIN32
    FILE *  m_logfile;
    time_t  UNIXTIME;// crap there should be 1 generic easy to understand time manager.

    /* internal time logger
     * writes the time to a FILE handle.
     */
    void LogTime( FILE* fp );

    /* internal Console color setter.
     * sets the color of the text that is about to get printed.
     */
    void SetColor( unsigned int color );

    /* internal system logger... */
    void log( const char * str, ... );
};

#endif//LOG_NEW_H

extern NewLog *pLog;
#define sLog (*pLog)
