/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://evemu.dev
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
    Author:     Aim, Captnoord, Zhur, Bloody.Rabbit
    Updates:	Allan
*/

#ifndef __EVE_CORE_H__INCL__
#define __EVE_CORE_H__INCL__

/** @todo  this file has many specific headers not used by everything in evemu.
 * this should be trimmed down with specific headers in files that need them
 */

/*************************************************************************/
/* Header configuration                                                  */
/*************************************************************************/
#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif /* HAVE_CONFIG_H */

#ifdef HAVE_CRTDBG_H
#   define _CRTDBG_MAP_ALLOC 1
#endif /* HAVE_CRTDBG_H */

#ifdef HAVE_INTTYPES_H
// This requires some ugly tinkering because inttypes.h
// is primarily a C header.
// Pretend to be C99-compliant
#   define __STDC_VERSION__     199901L
// We must "explicitly request" the format strings
#   define __STDC_FORMAT_MACROS 1
#endif /* HAVE_INTTYPES_H */

// Disable auto-linking of any Boost libraries
#define BOOST_ALL_NO_LIB 1

/*************************************************************************/
/* Includes                                                              */
/*************************************************************************/
// Standard library includes
#include <cassert>
#include <cerrno>
#include <cfloat>
#include <climits>
#include <cmath>
#include <csignal>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <thread>

// Standard Template Library includes
#include <algorithm>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <new>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include <functional>
#include <tuple>
#include <unordered_map>
#include <unordered_set>

#ifdef HAVE_CRTDBG_H
#   include <crtdbg.h>
#endif /* HAVE_CRTDBG_H */

#ifdef HAVE_INTTYPES_H
#   include <inttypes.h>
#endif /* HAVE_INTTYPES_H */

#ifdef HAVE_SYS_STAT_H
#   include <sys/stat.h>
#else /* !HAVE_SYS_STAT_H */
#   include <io.h>
#endif /* !HAVE_SYS_STAT_H */

#ifdef HAVE_SYS_TIME_H
#   include <sys/time.h>
#else /* !HAVE_SYS_TIME_H */
#   include <sys/timeb.h>
#endif /* !HAVE_SYS_TIME_H */

#ifdef HAVE_VLD_H
#   include <vld.h>
#endif /* HAVE_VLD_H */

#ifdef _WIN32
typedef unsigned int uint;
typedef unsigned long ulong;
#else /* *nix includes */
#include <dirent.h>
#include <fcntl.h>
#endif

#ifndef HAVE_ASINH
#   include <boost/math/special_functions.hpp>
#endif /* !HAVE_ASINH */

/************************************************************************/
/* Dependencies                                                         */
/************************************************************************/
// Boost.Asio
#include <boost/asio.hpp>

/*************************************************************************/
/* MMGR - Memory Manager                                                 */
/*  Memory managing & tracking software                                  */
/*  This code is designed to track and log memory leaks                  */
/*************************************************************************/
/* note...this code causes free() errors in xmlpacketgenerator.o */
//#include "memory/mmgr.h"

/************************************************************************/
/* Project-specific includes for files using ONLY eve-core.h            */
/************************************************************************/
// compatability defs (evemu unique defs)
#include "eve-compat.h"
// memory
#include "memory/SafeMem.h"
// utils
#include "utils/FastInt.h"
// logs
#include "log/LogNew.h"
#include "log/logsys.h"

#endif /* !__EVE_CORE_H__INCL__ */
