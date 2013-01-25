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
    Author:     AknorJaden
*/

#ifndef __LOG__HTML_LOG_H__INCL__
#define __LOG__HTML_LOG_H__INCL__

#include "threading/Mutex.h"

/**
 * @brief An HTML logging system.
 *
 * This class implements an HTML formatted output log file.  It is not derived
 * from Singleton as LogNew is, so this can be instantiated any number of times,
 * and provided with a unique path and file for the log.
 *
 * @author AknorJaden
 * @date January 2013
 */
class HTML_Log
{
public:
    /// Primary constructor, initializes logging.
    HTML_Log();
    HTML_Log(std::string logPath, std::string logFilename);
    /// Destructor, closes the logfile.
    ~HTML_Log();

    /**
     * @brief Initializes and sets the Log file path.
     *
     * @param[in] logPath is the absolute or relative path where log files are to be stored
     */
    void InitializeLogging( std::string logPath );
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

protected:
    /// A convenience color enum.
    enum Color
    {
        COLOR_DEFAULT, ///< A default color.
        COLOR_BLACK,   ///< Black color.
        COLOR_RED,     ///< Red color.
        COLOR_GREEN,   ///< Green color.
        COLOR_YELLOW,  ///< Yellow color.
        COLOR_BLUE,    ///< Blue color.
        COLOR_MAGENTA, ///< Magenta color.
        COLOR_CYAN,    ///< Cyan color.
        COLOR_WHITE,   ///< White color.

        COLOR_COUNT    ///< Number of colors.
    };

    /**
     * @brief Prints a message.
     *
     * This prints a generic message.
     *
     * @param[in] color  Color of the message.
     * @param[in] pfx    Single-character prefix/identificator.
     * @param[in] source Origin of message.
     * @param[in] fmt    The format string.
     * @param[in] ap     The arguments.
     */
    void PrintMsg( Color color, char pfx, const char* source, const char* fmt, va_list ap );
    /**
     * @brief Prints current time.
     */
    void PrintTime();

    /**
     * @brief Prints a raw message.
     *
     * This method only handles printing to all desired
     * destinations (standard output and logfile at the moment).
     *
     * @param[in] fmt The format string.
     * @param[in] ... The arguments.
     */
    void Print( const char* fmt, ... );
    /**
     * @brief Prints a raw message.
     *
     * This method only handles printing to all desired
     * destinations (standard output and logfile at the moment).
     *
     * @param[in] fmt The format string.
     * @param[in] ap  The arguments.
     */
    void PrintVa( const char* fmt, va_list ap );

    /**
     * @brief Sets the color of the output text.
     *
     * @param[in] color The new color of output text.
     */
    void SetColor( Color color );
    /**
     * @brief Sets the default logfile.
     */
    void SetLogfileDefault(std::string logPath);

    /// The active logfile.
	std::string mLogFilename;
    FILE* mLogfile;
    /// Current timestamp.
    time_t mTime; // crap there should be 1 generic easy to understand time manager.
    /// Protection against concurrent log messages
    Mutex mMutex;

    bool m_initialized;

#ifdef HAVE_WINDOWS_H
    /// Color translation table.
    static const WORD COLOR_TABLE[ COLOR_COUNT ];
#else /* !HAVE_WINDOWS_H */
    /// Color translation table.
    static const char* const COLOR_TABLE[ COLOR_COUNT ];
#endif /* !HAVE_WINDOWS_H */
};

#endif /* !__LOG__HTML_LOG_H__INCL__ */
