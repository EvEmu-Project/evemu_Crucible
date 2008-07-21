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


#ifndef MYMUTEX_H
#define MYMUTEX_H
#ifdef WIN32
	#include <winsock.h>
	#include <windows.h>
#else
	#include <pthread.h>
	#include "../common/unix.h"
#endif
#include "../common/types.h"

class Mutex {
public:
	Mutex();
	~Mutex();

	void lock();
	void unlock();
	bool trylock();
protected:
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
// Multi-read, single write mutex -Quagmire
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

	sint32	ReadLockCount();
	sint32	WriteLockCount();
private:
	sint32	rl;	// read locks in effect
	sint32	wr;	// write lock requests pending
	sint32	wl;	// write locks in effect (should never be more than 1)
	Mutex	MCounters;
};

#endif

