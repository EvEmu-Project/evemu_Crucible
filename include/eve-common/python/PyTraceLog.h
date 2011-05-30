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
	Author:		Zhur, Captnoord
*/

#ifndef PY_TRACE_LOG_H
#define PY_TRACE_LOG_H

#include "common.h"
#include "PyRep.h"

/* ascents cross platform color thingy's */
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

/* ascents UNIX color thingy's */
#ifndef WIN32
static const char* colorstrings[TBLUE+1] = {
	"",
	"\033[22;31m",
	"\033[22;32m",
	"\033[01;33m",
	//"\033[22;37m",
	"\033[0m",
	"\033[01;37m",
	"\033[22;34m",
};
#endif//WIN32

/**
 * \class PyTraceLog
 *
 * @brief a python client stack trace logger for Evemu
 *
 * The PyTraceLog class processes the alert messages into a readable message,
 * because its part of the development system it should be used by non developers.
 *
 * @author Captnoord
 * @date December 2008
 */

class PyTraceLog {
public:
	/**
	 * @brief The constructor of the python stack trace logger.
	 *
	 * The constructor of the python stack trace logger.
	 *
	 * @param[in] path The path to the file including the file extension (basic for now).
	 * @param[in] toConsole When want to have the output written to the console pass true.
	 * @param[in] toFile When want to have the output written to the specified file pass true.
	 */
	PyTraceLog(const char *path, bool toConsole = false, bool toFile = false)
        : mFout(NULL),
          mInitialized(false),
          mLogToConsole(toConsole),
          mLogToFile(toFile)
#ifdef WIN32
          ,mStderrHandle(NULL),
          mStdoutHandle(NULL)
#endif
	{
		if (toFile == true)
		{
			mFout = fopen(path, "w+");
			if (mFout == NULL)
				printf("[error]PyTraceLog: sorry initializing the output file failed\n");
		}

#ifdef WIN32
		// get error handle
		if (mLogToConsole == true)
		{
			mStderrHandle = GetStdHandle(STD_ERROR_HANDLE);
			mStdoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
		}
#endif		
		mInitialized = true;
	}

	/**
	 * @brief the destructor.... nothing special
	 */
	~PyTraceLog()
	{
		if (mLogToFile == true && mFout != NULL)
		{
			fflush(mFout);
			fclose(mFout);
		}
	}

	/**
	 * @brief logTrace is the function what its all about.
	 *
	 * the logTrace function processes the packet object into the readable text and prints it to the screen and or the output file.
	 *
	 * @param[in] tuple tuple contains the packet info of the trace.
	 * @return returns true if a success and false if a error is found (no special error handling for now).
	 */
	bool logTrace(PyTuple &tuple)
	{
		assert(mInitialized && "PyTraceLog isn't initialized");

		// base object should be a tuple
		if(tuple.IsTuple() == false)
		{
			_logInternMessage("trace error because the base object isn't a tuple");
			return false;
		}

		/* the next part is commented out because the payload of the error message contains similar data */

		// the first object should be a tuple
		/*if (tuple[0]->IsTuple() == false)
		{
			_logInternMessage("trace error because the first object within the tuple isn't a tuple");
			return false;
		}

		if (mLogToFile)
			fprintf(mFout, "\nStackTraceAlert:\n");

		// tuple 0 should contain the error message or something binary
		PyRepTuple& tuple0_0 = tuple[0]->AsTuple();
		//tuple0_0[0] some integer... which I don't care about
		if( tuple0_0[1]->IsString() == true )
		{
			_logInternStringMessage(tuple0_0[1]);
		}
		else if (tuple0_0[1]->IsBuffer() == true)
		{
			_logInternBufferMessage(tuple0_0[1]);
		}
		else
		{
			_logInternMessage("trace log error");
			return false;
		}

		if (mLogToFile == true)
		{
			fputc('\n', mFout);
			fflush(mFout);
		}*/

		/* python stack trace payload */
		//if (tuple.GetItem(1)->IsString() == true)
		{
			_logInternBufferPacket(tuple.GetItem(1));
		}

		if (mLogToFile == true)
		{
			fputc('\n', mFout);
			fflush(mFout);
		}

		//tuple[2] should contain a string "Error" but that is a assumption and I am sure it has more stings to give us.
		//tuple[3] gives me for the moment the object None. So its marked as unknown for the moment.

		return true;
	}

protected:

	void _logInternStringMessage(PyRep* packet)
	{
		PyString & msg = *packet->AsString();
		if (mLogToConsole == true)
		{
			fprintf(stdout, "%s\n", msg.content().c_str());
		}

		if (mLogToFile == true)
		{
			fprintf(mFout, "%s\n", msg.content().c_str());
		}
	}

	void _logInternBufferMessage(PyRep* packet)
	{
		PyBuffer & msg = *packet->AsBuffer();
		if (mLogToConsole == true)
		{
			fwrite(&msg.content()[0], msg.size(), 1, stdout);
			fputc('\n', stdout);
		}

		if (mLogToFile == true)
		{
			fwrite(&msg.content()[0], msg.size(), 1, mFout);
			fputc('\n', mFout);
		}
	}

	// its unclear what this consists of so the current code mimics the one of _logInternBufferMessage
	void _logInternBufferPacket(PyRep* packet)
	{
		// just placement code atm...
		/*PyBuffer & msg = *packet->AsBuffer();
		if (mLogToConsole == true)
		{
			fwrite(&msg.content()[0], msg.size(), 1, stdout);
			fputc('\n', stdout);
		}

		if (mLogToFile == true)
		{
			fwrite(&msg.content()[0], msg.size(), 1, mFout);
			fputc('\n', mFout);
		}*/

        if (packet->GetType() != PyRep::PyTypeString)
            return;

        PyString & msg = *packet->AsString();
        if (mLogToConsole == true)
        {
            fwrite(msg.content().c_str(), msg.content().size(), 1, stdout);
            fputc('\n', stdout);
        }

        if (mLogToFile == true)
        {
            fwrite(msg.content().c_str(), msg.content().size(), 1, mFout);
            fputc('\n', mFout);
        }

	}

	/**
	 * @brief intern function for handling logger related messages
	 *
	 * intern function for handling logger related messages
	 *
	 * @param[in] str the format for the output message.
	 */
	void _logInternMessage(const char* str, ...)
	{
		va_list ap;
		va_start(ap, str);

		_setLogColor(TRED);
		// I know its code duplication....on a small scale
		if (mLogToConsole == true)
		{
			fprintf(stdout, "[PyTrace] ");
			vfprintf(stdout, str, ap);
			putc('\n', stdout);
		}

		if (mLogToFile == true)
		{
			fprintf(mFout, "[PyTrace] ");
			vfprintf(mFout, str, ap);
			putc('\n', mFout);
			fflush(mFout);
		}
		_setLogColor(TNORMAL);
		va_end(ap);
	}

	/**
	 * @brief Internal function to set the console output color
	 *
	 * Lazy ass function for a developer "toy". We humans like to see some colors.
	 *
	 * @param[in] color the color flags / value's for Windows or UNIX, see TRED and the rest on top of this file.
	 */
	void _setLogColor(uint32 color)
	{
#ifdef WIN32
		SetConsoleTextAttribute(mStdoutHandle, TNORMAL);
#else
		puts(colorstrings[TNORMAL]);
#endif
	}

private:
	FILE   *mFout;
	bool	mInitialized;
	bool	mLogToConsole;
	bool	mLogToFile;

#ifdef WIN32
	HANDLE mStdoutHandle, mStderrHandle;
#endif
};

#endif//PY_TRACE_LOG_H
