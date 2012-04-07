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
    Author:     Bloody.Rabbit
*/

#ifndef __UTILS__REF_PTR_H__INCL__
#define __UTILS__REF_PTR_H__INCL__

/**
 * ENABLE_REF_TRACE
 *
 * A way to trace deleted objects still beeing handled in the python layer.
 *
 * As I remember Linux doesn't clear the deleted memory so this would work.
 * Same goes for windows which does clear the memory but set it to 0xDEADBEEF or something.
 * That way the assert will trigger a crash when the object is still being handled but in
 * reality its already deleted.
 */
//#define ENABLE_REF_TRACE
#ifdef ENABLE_REF_TRACE
#  define REF_TRACE_MACRO() do { assert( mDeleted == false ); } while (0)
#else
#  define REF_TRACE_MACRO() do {} while (0)
#endif

/**
 * @brief A reference-counted object.
 *
 * This class has all stuff needed to cooperate with
 * RefPtr. If you want some of your classes to be
 * reference-counted, derive them from this class.
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
    RefObject( size_t initRefCount )
    : mRefCount( initRefCount )
    {
#ifdef ENABLE_REF_TRACE
         mDeleted = false;
#endif
    }

    /**
     * @brief Destructor; must be virtual.
     *
     * Must be virtual if proper destructor should be
     * invoked upon destruction.
     */
    virtual ~RefObject()
    {
#ifdef ENABLE_REF_TRACE
        mDeleted = true;
#endif
        assert( mRefCount == 0);
    }

protected:
    /**
     * @brief Increments reference count of object by one.
     */
    void IncRef() const
    {
        REF_TRACE_MACRO();
        ++mRefCount;
    }
    /**
     * @brief Decrements reference count of object by one.
     *
     * If reference count of object reaches zero, object
     * is deleted.
     */
    void DecRef() const
    {
        REF_TRACE_MACRO();
        assert( mRefCount > 0 );
        --mRefCount;

        if( mRefCount <= 0 )
            delete this;
    }

    /// Reference count of instance.
    mutable size_t mRefCount;
#ifdef ENABLE_REF_TRACE
    mutable bool mDeleted;
#endif
};

/**
 * @brief Reference-counting-based smart pointer.
 *
 * This smart pointer cares about acquiring/releasing reference
 * of the stored object.
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
    explicit RefPtr( X* p = NULL )
    : mPtr( p )
    {
        if( *this )
            (*this)->IncRef();
    }
    /**
     * @brief Copy constructor.
     *
     * @param[in] oth Object to copy the reference from.
     */
    RefPtr( const RefPtr& oth )
    : mPtr( oth.get() )
    {
        if( *this )
            (*this)->IncRef();
    }
    /**
     * @brief Casting copy constructor.
     *
     * @param[in] oth Object to copy the reference from.
     */
    template<typename Y>
    RefPtr( const RefPtr<Y>& oth )
    : mPtr( oth.get() )
    {
        if( *this )
            (*this)->IncRef();
    }

    /**
     * @brief Destructor, releases reference.
     */
    ~RefPtr()
    {
        if( *this )
            (*this)->DecRef();
    }

    /**
     * @brief Copy operator.
     *
     * @param[in] oth Object to copy the reference from.
     */
    RefPtr& operator=( const RefPtr& oth )
    {
        if( *this )
            (*this)->DecRef();

        mPtr = oth.get();

        if( *this )
            (*this)->IncRef();

        return *this;
    }
    /**
     * @brief Casting copy operator.
     *
     * @param[in] oth Object to copy the reference from.
     */
    template<typename Y>
    RefPtr& operator=( const RefPtr<Y>& oth )
    {
        if( *this )
            (*this)->DecRef();

        mPtr = oth.get();

        if( *this )
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
    operator bool() const { return !( get() == NULL ); }

    X& operator*() const { assert( *this ); return *get(); }
    X* operator->() const { assert( *this ); return get(); }

    /**
     * @brief Compares two references.
     *
     * @return True if both references are of same object, false if not.
     */
    template<typename Y>
    bool operator==( const RefPtr<Y>& oth ) const
    {
        return ( get() == oth.get() );
    }

    /**
     * @brief Acts as static_cast.
     */
    template<typename Y>
    static RefPtr StaticCast( const RefPtr<Y>& oth )
    {
        return RefPtr( static_cast<X*>( oth.get() ) );
    }

protected:
    /// The pointer to the reference-counted object.
    X* mPtr;
};

#endif /* !__UTILS__REF_PTR_H__INCL__ */
