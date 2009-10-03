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

#ifndef LOG_NEW_H
#define LOG_NEW_H

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
