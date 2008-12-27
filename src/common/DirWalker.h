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
	Author:		Zhur
*/

/* Capt: Even that I don't always like boost, it has a very useful class called FileSystem which does this stuff very crossplatform */

#ifndef __DIRWALKER_H_INCL__
#define __DIRWALKER_H_INCL__

#include "common.h"
#ifndef WIN32
#include <sys/types.h>
#include <dirent.h>
#endif

#include <vector>
#include <string>

class DirWalker {
public:
	static bool ListDirectory(const char *dir, std::vector<std::string> &into);
	
	DirWalker(const char *dir);
	~DirWalker();

	bool GetFile();
	
	//access to the current file.
	const char *CurrentFileName();
	
protected:
	bool m_valid;
#ifdef WIN32
	HANDLE m_find;
	WIN32_FIND_DATA m_data;
	bool m_first;
#else
	DIR *m_dir;
	struct dirent *m_data;
#endif
};

#endif // __DIRWALKER_H_INCL__
