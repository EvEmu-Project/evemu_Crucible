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
    Author:     Zhur
*/

#ifndef __THREADING__MUTEX_H__INCL__
#define __THREADING__MUTEX_H__INCL__

#include "utils/Lock.h"

/**
 * @brief Common wrapper for platform-specific mutexes.
 *
 * @author Zhur, Bloody.Rabbit
 */
class Mutex
: public Lockable
{
public:
    /**
     * @brief Primary contructor.
     */
    Mutex();
    /**
     * @brief Destructor, releases allocated resources.
     */
    ~Mutex();

    /**
     * @brief Locks the mutex.
     */
    void Lock();
    /**
     * @brief Attempts to lock the mutex.
     *
     * @retval true  Mutex successfully locked.
     * @retval false Mutex locked by another thread.
     */
    bool TryLock();

    /**
     * @brief Unlocks the mutex.
     */
    void Unlock();

protected:
#ifdef WIN32
    /// A critical section used for mutex implementation on Windows.
    CRITICAL_SECTION mCriticalSection;
#else
    /// A pthread mutex used for mutex implementation using pthread library.
    pthread_mutex_t mMutex;
#endif
};

/// Convenience typedef for Mutex's lock.
typedef Lock< Mutex > MutexLock;

// Somewhat untested...
// Multi-read, single write Mutex
class MRMutex {
public:
    MRMutex();
    ~MRMutex();

    void    ReadLock();
    bool    TryReadLock();
    void    UnReadLock();

    void    WriteLock();
    bool    TryWriteLock();
    void    UnWriteLock();

    int32    ReadLockCount();
    int32    WriteLockCount();

private:
    int32    rl;    // read locks in effect
    int32    wr;    // write lock requests pending
    int32    wl;    // write locks in effect (should never be more than 1)
    Mutex    MCounters;
};

#endif /* !__THREADING__MUTEX_H__INCL__ */
