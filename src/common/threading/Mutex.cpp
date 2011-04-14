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

#include "CommonPCH.h"

#include "threading/Mutex.h"

/*************************************************************************/
/* Mutex                                                                 */
/*************************************************************************/
Mutex::Mutex()
{
#ifdef WIN32
    InitializeCriticalSection( &mCriticalSection );
#else
    pthread_mutexattr_t attr;
    pthread_mutexattr_init( &attr );

#   if defined( CYGWIN ) || defined( APPLE )
    pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_RECURSIVE );
#   else
    pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_RECURSIVE_NP );
#   endif

    pthread_mutex_init( &mMutex, &attr );
    pthread_mutexattr_destroy( &attr );
#endif
}

Mutex::~Mutex()
{
#ifdef WIN32
    DeleteCriticalSection( &mCriticalSection );
#else
    pthread_mutex_destroy( &mMutex );
#endif
}

void Mutex::Lock()
{
#ifdef WIN32
    EnterCriticalSection( &mCriticalSection );
#else
    pthread_mutex_lock( &mMutex );
#endif
}

bool Mutex::TryLock()
{
#ifdef WIN32
    return TRUE == TryEnterCriticalSection( &mCriticalSection );
#else
    return 0 == pthread_mutex_trylock( &mMutex );
#endif
}

void Mutex::Unlock()
{
#ifdef WIN32
    LeaveCriticalSection( &mCriticalSection );
#else
    pthread_mutex_unlock( &mMutex );
#endif
}

/*************************************************************************/
/* MRMutex                                                               */
/*************************************************************************/
MRMutex::MRMutex() {
    rl = 0;
    wr = 0;
    rl = 0;
}

MRMutex::~MRMutex() {
#ifdef _EQDEBUG
    if (wl || rl) {
        cout << "MRMutex::~MRMutex: poor cleanup detected: rl=" << rl << ", wl=" << wl << endl;
    }
#endif
}

void MRMutex::ReadLock() {
    while (!TryReadLock()) {
        Sleep(1);
    }
}

bool MRMutex::TryReadLock() {
    MCounters.Lock();
    if (!wr && !wl) {
        rl++;
        MCounters.Unlock();
        return true;
    }
    else {
        MCounters.Unlock();
        return false;
    }
}

void MRMutex::UnReadLock() {
    MCounters.Lock();
    rl--;
#ifdef _EQDEBUG
    if (rl < 0) {
        ThrowError("rl < 0 in MRMutex::UnReadLock()");
    }
#endif
    MCounters.Unlock();
}

void MRMutex::WriteLock() {
    MCounters.Lock();
    if (!rl && !wl) {
        wl++;
        MCounters.Unlock();
        return;
    }
    else {
        wr++;
        MCounters.Unlock();
        while (1) {
            Sleep(1);
            MCounters.Lock();
            if (!rl && !wl) {
                wr--;
                MCounters.Unlock();
                return;
            }
            MCounters.Lock();
        }
    }
}

bool MRMutex::TryWriteLock() {
    MCounters.Lock();
    if (!rl && !wl) {
        wl++;
        MCounters.Unlock();
        return true;
    }
    else {
        MCounters.Unlock();
        return false;
    }
}

void MRMutex::UnWriteLock() {
    MCounters.Lock();
    wl--;
#ifdef _EQDEBUG
    if (wl < 0) {
        ThrowError("wl < 0 in MRMutex::UnWriteLock()");
    }
#endif
    MCounters.Unlock();
}

int32 MRMutex::ReadLockCount() {
    MCounters.Lock();
    int32 ret = rl;
    MCounters.Unlock();
    return ret;
}

int32 MRMutex::WriteLockCount() {
    MCounters.Lock();
    int32 ret = wl;
    MCounters.Unlock();
    return ret;
}
