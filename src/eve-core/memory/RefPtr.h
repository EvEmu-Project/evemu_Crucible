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
    Author:     Bloody.Rabbit
    Update:     Allan
*/

#ifndef __UTILS__REF_PTR_H__INCL__
#define __UTILS__REF_PTR_H__INCL__


#include <cassert>
#include <cstdio>
#include <iostream>

#include "utils/misc.h"

/**
 * @brief A reference-counted object.
 *
 * This class has all stuff needed to cooperate with
 * RefPtr. If you want your class to be
 * reference-counted, derive them from this class.
 *
 * NOTE:  this class does NOT increment count on creation (whereas RefPtr does)
 *
 * @author Bloody.Rabbit
 */
class RefObject
{
    template<typename X>
    friend class RefPtr;

public:
    /**
     * @brief Initializes reference count.
     *
     * @param[in] initRefCount Initial reference count.
     */
    RefObject(uint16 initRefCount)
    : mRefCount(initRefCount),
    mDeleted(false)
    {
    }

    /**
     * @brief Destructor; must be virtual.
     *
     * Must be virtual for proper destructor to be
     * invoked upon destruction.
     */
    virtual ~RefObject()
    {
        if (mDeleted) {
            _log(REFPTR__ERROR, "~RefObject() - mDeleted: true");
            EvE::traceStack();
        }

        mDeleted = true;
    }

    uint16 GetCount()           { return mRefCount; }
    bool IsDeleted()            { return mDeleted; }

protected:
    /**
     * @brief Increments reference count of object by one.
     */
    void IncRef() const
    {
        if (mDeleted) {
            _log(REFPTR__ERROR, "IncRef() - mDeleted = true.  Count is %u", mRefCount);
            EvE::traceStack();
            //return;
        }
        assert(mDeleted == false);
        ++mRefCount;
    }
    /**
     * @brief Decrements reference count of object by one.
     *
     * If reference count of object reaches zero, object is deleted.
     */
    void DecRef() const
    {
        if (mDeleted) {
            _log(REFPTR__ERROR, "DecRef() - mDeleted = true.  Count is %u", mRefCount);
            EvE::traceStack();
            return;
        }

        assert(mDeleted == false);
        assert(mRefCount > 0);
        --mRefCount;

        if (mRefCount < 1)
            delete this;
    }

private:
    /// Reference count of instance.
    mutable uint16 mRefCount;
    mutable bool mDeleted;
};

/**
 * @brief Reference-counting-based smart pointer.
 *
 * This smart pointer cares about acquiring/releasing reference of the stored object.
 *
 * NOTE:  this class DOES increment count on creation (whereas RefObject does not)
 *
 * @author Bloody.Rabbit
 */
template<typename X>
class RefPtr
{
public:
    /**
     * @brief Primary constructor.
     *
     * @param[in] p Pointer to object to be referenced.
     */
    explicit RefPtr(X* p = nullptr)
    : mPtr(p)
    {
        if (*this)
            (*this)->IncRef();
    }
    /**
     * @brief Copy constructor.
     *
     * @param[in] oth Object to copy the reference from.
     */
    RefPtr(const RefPtr& oth)
    : mPtr(oth.get())
    {
        if (*this)
            (*this)->IncRef();
    }
    /**
     * @brief Casting copy constructor.
     *
     * @param[in] oth Object to copy the reference from.
     */
    template<typename Y>
    RefPtr(const RefPtr<Y>& oth)
    : mPtr(oth.get())
    {
        if (*this)
            (*this)->IncRef();
    }

    /**
     * @brief Destructor, releases reference.
     */
    virtual ~RefPtr()
    {
        if (*this)
            (*this)->DecRef();
    }

    /**
     * @brief Move operator.
     *
     * @param[in] oth Object to move.
     */
    RefPtr& operator=(const RefPtr&& oth)
    {
        if (*this)
            (*this)->DecRef();
        mPtr = std::move(oth.get());
        if (*this)
            (*this)->IncRef();
        return *this;
    }
    /**
     * @brief Copy operator.
     *
     * @param[in] oth Object to copy the reference from.
     */
    RefPtr& operator=(const RefPtr& oth)
    {
        if (*this)
            (*this)->DecRef();
        mPtr = oth.get();
        if (*this)
            (*this)->IncRef();
        return *this;
    }
    /**
     * @brief Casting copy operator.
     *
     * @param[in] oth Object to copy the reference from.
     */
    template<typename Y>
    RefPtr& operator=(const RefPtr<Y>& oth)
    {
        if (*this)
            (*this)->DecRef();
        mPtr = oth.get();
        if (*this)
            (*this)->IncRef();
        return *this;
    }

    /**
     * @return Stored reference.
     */
    X* get() const { return mPtr; }

    /**
     * @return True if stores a reference, false otherwise.
     */
    operator bool() const { return (mPtr != nullptr); }

    X& operator*() const { assert(*this); return *mPtr; }
    X* operator->() const { assert(*this); return mPtr; }

    /**
     * @brief Compares two references.
     *
     * @return True if both references are of same object, false if not.
     */
    template<typename Y>
    bool operator==(const RefPtr<Y>& oth) const
    {
        return (mPtr == oth.get());
    }

    /**
     * @brief Acts as static_cast from one RefPtr to another
     */
    template<typename Y>
    static RefPtr StaticCast(const RefPtr<Y>& oth)
    {
        return RefPtr(static_cast<X*>(oth.get()));
    }

protected:
    /// The pointer to the reference-counted object.
    X* mPtr;
};

#endif /* !__UTILS__REF_PTR_H__INCL__ */
