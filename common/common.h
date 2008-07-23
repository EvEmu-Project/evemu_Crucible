/*  EVEmu: EVE Online Server Emulator

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY except by those people which sell it, which
  are required to give you total support for your newly bought product;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#ifndef __COMMON_H
#define __COMMON_H

#ifdef HAVE_CONFIG_H
#	include "config.h"
#endif

#if defined(_DEBUG) && defined(WIN32)
	#ifndef _CRTDBG_MAP_ALLOC
		#include <stdlib.h>
		#include <crtdbg.h>
		#if (_MSC_VER < 1300)
			#include <new>
			#include <memory>
			#define _CRTDBG_MAP_ALLOC
			#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
			#define malloc(s) _malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__)
		#endif
	#endif
#endif

#ifdef WIN32
	// VS6 doesn't like the length of STL generated names: disabling
	#pragma warning(disable:4786)
	#pragma warning(disable:4996)
#endif

#ifndef WIN32
	#define DebugBreak()			if(0) {}
#endif

#define _WINSOCKAPI_	//stupid windows, trying to fix the winsock2 vs. winsock issues
#if defined(WIN32) && ( defined(PACKETCOLLECTOR) || defined(COLLECTOR) )
	// Packet Collector on win32 requires winsock.h due to latest pcap.h
	// winsock.h must come before windows.h
	#include <winsock.h>
#endif

#ifdef WIN32
	#include <windows.h>
	#include <winsock2.h>
	#define snprintf	_snprintf
	#define strncasecmp	_strnicmp
	#define strcasecmp	_stricmp
	#include <process.h>
#else
	#include "unix.h"
	#include <pthread.h>
#endif

#include <stdio.h>
#include <stdarg.h>

#include "./types.h"
#include "./unix.h"
	
#endif
