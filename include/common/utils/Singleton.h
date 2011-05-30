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
	Author:		Bloody.Rabbit
*/

#ifndef __UTILS__SINGLETON_H__INCL__
#define __UTILS__SINGLETON_H__INCL__

/**
 * @brief Template used for singleton classes.
 *
 * This template shall be used as base for classes
 * which are intended to be singleton (i.e. there
 * should be only 1 instance of this class at all).
 *
 * Uses lazy construction (i.e. object is constructed
 * on first access request).
 *
 * @author Bloody.Rabbit
 */
template<typename X>
class Singleton
{
public:
    /**
     * @brief Primary constructor.
     *
     * Checks if the instance being constructed is first, i.e.
     * mInstance hasn't been filled yet. This only makes sense
     * if the actual class is derived from Singleton.
     */
    Singleton()
    {
        assert( NULL == mInstance.get() );
    }

    /** @return Reference to the singleton instance. */
    static X& get()
    {
        if( NULL == mInstance.get() )
            mInstance.reset( new X );

        return *mInstance;
    }

protected:
    /** Pointer to the singleton instance. */
    static std::auto_ptr<X> mInstance;
};

template<typename X>
std::auto_ptr<X> Singleton<X>::mInstance( NULL );

#endif /* !__UTILS__SINGLETON_H__INCL__ */

