/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2022 The EVEmu Team
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
    Author:        Pursche
*/

#ifndef __ALLOCATORS_H__INCL__
#define __ALLOCATORS_H__INCL__

class Allocators
    : public Singleton<Allocators>
{
public:
    Allocators() {};
    ~Allocators() {};

    static const size_t TICK_ALLOCATOR_SIZE = 32 * 1024 * 1024; // 32 MB should be plenty
    Memory::StackAllocator tickAllocator; // This allocator will be reset every at the start of every tick so only store temporary things in it
};

//Singleton
#define sAllocators \
    ( Allocators::get() )

#endif // __ALLOCATORS_H__INCL__