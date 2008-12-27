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

#include "DirWalker.h"
#include <errno.h>
#include <string.h>

bool DirWalker::ListDirectory(const char *dir, std::vector<std::string> &into) {
	DirWalker d(dir);
	if(!d.GetFile())
		return false;	//empty list
	into.push_back(d.CurrentFileName());
	while(d.GetFile())
		into.push_back(d.CurrentFileName());
	return true;
}

DirWalker::DirWalker(const char *dir)
: m_valid(false)
{
#ifdef WIN32
	m_find = FindFirstFile(dir, &m_data);
	m_first = true;
#else
	m_dir = opendir(dir);
	if(m_dir == NULL) {
		printf("Failed to open dir %s: %s", dir, strerror(errno));
	}
	m_data = NULL;
#endif
}

DirWalker::~DirWalker() {
#ifdef WIN32
	if(m_find != INVALID_HANDLE_VALUE)
		FindClose(m_find);
#else
	if(m_dir != NULL)
		closedir(m_dir);
#endif
}

bool DirWalker::GetFile() {
#ifdef WIN32
	if(m_find == INVALID_HANDLE_VALUE) {
		m_valid = false;
		return false;
	}
	if(m_first) {
		m_first = false;
		return true;
	}
	
	m_valid = (FindNextFile(m_find, &m_data) == TRUE);
	return true;
#else
	if(m_dir == NULL) {
		m_valid = false;
		return false;
	}
	
	m_data = readdir(m_dir);
	m_valid = (m_data != NULL);
	return(m_valid);
#endif
}

//access to the current file.
const char *DirWalker::CurrentFileName() {
	if(!m_valid)
		return NULL;
#ifdef WIN32
	//may not work, it is really a WCHAR
	return((const char *)m_data.cFileName);
#else
	return(m_data->d_name);
#endif
}


















