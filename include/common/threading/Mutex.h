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

#ifndef MYMUTEX_H
#define MYMUTEX_H

class Mutex {
public:
	Mutex();
	~Mutex();

	void lock();
	void unlock();
	bool trylock();
private:
#ifdef WIN32
	CRITICAL_SECTION CSMutex;
#else
	pthread_mutex_t CSMutex;
#endif
};

class LockMutex {
public:
	LockMutex(Mutex* in_mut, bool iLock = true);
	~LockMutex();
	void unlock();
	void lock();
private:
	bool	locked;
	Mutex*	mut;
};

// Somewhat untested...
// Multi-read, single write Mutex
class MRMutex {
public:
	MRMutex();
	~MRMutex();

	void	ReadLock();
	bool	TryReadLock();
	void	UnReadLock();

	void	WriteLock();
	bool	TryWriteLock();
	void	UnWriteLock();

	int32	ReadLockCount();
	int32	WriteLockCount();
private:
	int32	rl;	// read locks in effect
	int32	wr;	// write lock requests pending
	int32	wl;	// write locks in effect (should never be more than 1)
	Mutex	MCounters;
};

#endif
