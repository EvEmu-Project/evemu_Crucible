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
    Author:     Bloody.Rabbit
*/

#ifndef __UTILS__LOCK_H__INCL__
#define __UTILS__LOCK_H__INCL__

/**
 * @brief Basic interface of a lockable object.
 *
 * @author Bloody.Rabbit
 */
class Lockable
{
public:
    /**
     * @brief Locks the object.
     *
     * This method will block until the lock has been obtained.
     */
    virtual void Lock() = 0;
    /**
     * @brief Attempts to lock the object.
     *
     * If the lock cannot be obtained immediately, the method returns.
     *
     * @retval true  The object has been locked.
     * @retval false Could not obtain the lock immediately.
     */
    virtual bool TryLock() = 0;

    /**
     * @brief Unlocks a locked object.
     */
    virtual void Unlock() = 0;
};

/**
 * @brief A lock for a Lockable object.
 *
 * The object is locked during contruction and unlocked
 * during destruction.
 *
 * The passed typename should be a child of the class Lockable.
 *
 * @author Bloody.Rabbit
 */
template< typename T >
class Lock
{
public:
    /**
     * @brief Primary contructor, locks the object.
     *
     * @param[in] object Object to bound this lock to.
     * @param[in] lock   Lock the object during construction.
     */
    Lock( T& object, bool lock = true )
    : mObject( object ),
      mLocked( false )
    {
        if( lock )
            Relock();
    }
    /**
     * @brief Destructor, unlocks the object.
     */
    ~Lock()
    {
        Unlock();
    }

    /**
     * @brief Obtains the lock state of the object.
     *
     * @retval true  The object is locked.
     * @retval false The object is not locked.
     */
    bool isLocked() const { return mLocked; }

    /**
     * @brief Locks the object.
     */
    void Relock()
    {
        if( !isLocked() )
            mObject.Lock();

        mLocked = true;
    }
    /**
     * @brief Unlocks the object.
     */
    void Unlock()
    {
        if( isLocked() )
            mObject.Unlock();

        mLocked = false;
    }

protected:
    /// The object this lock is bound to.
    T& mObject;
    /// True the @a mObject is locked, false if not.
    bool mLocked;
};

#endif /* !__UTILS__LOCK_H__INCL__ */
