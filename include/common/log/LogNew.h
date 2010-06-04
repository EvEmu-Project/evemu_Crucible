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

#ifndef __LOG__LOG_NEW_H__INCL__
#define __LOG__LOG_NEW_H__INCL__

#include "threading/Mutex.h"
#include "utils/Singleton.h"

/**
 * @brief a small and simple logging system.
 *
 * This class is designed to be a simple logging system that both logs to file
 * and console regarding the settings.
 *
 * @author Captnoord.
 * @date August 2009
 */
class NewLog
: public Singleton<NewLog>
{
public:
    NewLog();
    ~NewLog();

    /**
     * @brief Logs a message to file.
     *
     * @param[in] source is the source from where the message is printed.
     * @param[in] fmt is the message itself.
     */
    void Log( const char* source, const char* fmt, ... );
    /**
     * @brief Logs error message to console and file.
     *
     * @param[in] source is the source from where the message is printed.
     * @param[in] fmt is the error message itself.
     */
    void Error( const char* source, const char* fmt, ... );
    /**
     * @brief Logs a warning message to file.
     *
     * @param[in] source is the source from where the message is printed.
     * @param[in] fmt is the message itself.
     */
    void Warning( const char* source, const char* fmt, ... );
    /**
     * @brief Logs a success message to file.
     *
     * @param[in] source is the source from where the message is printed.
     * @param[in] fmt is the message itself.
     */
    void Success( const char* source, const char* fmt, ... );
    /**
     * @brief Logs a debug message to file and console.
     *
     * Optimized out on a release build.
     *
     * @param[in] source is the source from where the message is printed.
     * @param[in] fmt is the message itself.
     */
    void Debug( const char* source, const char* fmt, ... );

    /**
     * @brief Sets the logfile to be used.
     *
     * @param[in] filename A name of file.
     *
     * @retval true  The new logfile was successfully opened.
     * @retval false Failed to open the new logfile.
     */
    bool SetLogfile( const char* filename );
    /**
     * @brief Sets the logfile to be used.
     *
     * Passed @a file is closed during destruction.
     *
     * @param[in] file A handle to file.
     *
     * @retval true  The new logfile was successfully opened.
     * @retval false Failed to open the new logfile.
     */
    bool SetLogfile( FILE* file );

    /**
     * @brief Sets the log system time every main loop.
     *
     * @param[in] time is the timestamp.
     */
    void SetTime( time_t time ) { mTime = time; }

private:
    /** The active logfile. */
    FILE* mLogfile;
    /** Current timestamp. */
    time_t mTime; // crap there should be 1 generic easy to understand time manager.
    /** Protection against concurrent log messages. */
    Mutex mMutex;

#ifdef WIN32
    const HANDLE mStdOutHandle;
    const HANDLE mStdErrHandle;
#endif /* WIN32 */

    /**
     * @brief Prints given arguments.
     *
     * This function does not do any processing and
     * is to be used only internally.
     *
     * @param[in] fmt Format string.
     * @param[in] ... Arguments.
     */
    void Print( const char* fmt, ... );
    /**
     * @brief Prints given arguments.
     *
     * This function does not do any processing and
     * is to be used only internally.
     *
     * @param[in] fmt Format string.
     * @param[in] ap  Arguments.
     */
    void PrintVa( const char* fmt, va_list ap );
    /**
     * @brief Prints current time.
     */
    void PrintTime();

    /**
     * @brief Sets the color of the output text.
     */
    void SetColor( unsigned int color );
    /**
     * @brief Sets the default logfile.
     */
    void SetLogfileDefault();
};

#define sLog \
    ( NewLog::get() )

#endif /* !__LOG__LOG_NEW_H__INCL__ */
