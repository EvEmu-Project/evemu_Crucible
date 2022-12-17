/*
    https://github.com/mtrebi/memory-allocators/
    MIT License

    Copyright (c) 2016 Mariano Trebino

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
    */
#include "StackAllocator.h"

#include <stdlib.h>
#include <algorithm>
#ifdef _DEBUG
#include <iostream>
#endif
#include <cassert>

namespace Memory
{
    StackAllocator::StackAllocator()
        : Allocator() 
    {

    }

    void StackAllocator::Init(const std::size_t totalSize, std::string name, bool onlyOffsets, bool debug)
    {
        assert(!_initialized); // We already initialized this allocator!
        
        Allocator::Init(totalSize, name, onlyOffsets, debug);
        
        if (!_onlyOffset)
        {
            if (_startPtr != nullptr)
            {
                free(_startPtr);
            }
            _startPtr = malloc(_totalSize);
        }
        else
        {
            _startPtr = nullptr;
        }
        
        _offset = 0;
        _initialized = true;
    }

    StackAllocator::~StackAllocator() 
    {
        if (!_onlyOffset)
        {
            free(_startPtr);
        }
        _startPtr = nullptr;
    }

    const std::size_t CalculatePadding(const std::size_t baseAddress, const std::size_t alignment) 
    {
        if (alignment == 0)
            return 0;

        size_t remainder = baseAddress % alignment;
        if (remainder == 0)
            return 0;

        return alignment - remainder;
    }

    void* StackAllocator::Allocate(const std::size_t size, const std::size_t alignment) 
    {
        assert(_initialized); // We need to initialize this allocator!

        return (void*)(AllocateOffset(size, alignment));
    }

    bool StackAllocator::TryAllocate(const std::size_t size, const std::size_t alignment, void*& memory)
    {
        return TryAllocateOffset(size, alignment, (size_t&)(memory));
    }

    size_t StackAllocator::AllocateOffset(const std::size_t size, const std::size_t alignment)
    {
        assert(_initialized); // We need to initialize this allocator!

        std::size_t currentAddress = 0;
        std::size_t padding = 0;
        std::size_t tempOffset = 0;

        {
            std::lock_guard<std::mutex> lock(_lock);

            currentAddress = reinterpret_cast<std::size_t>(_startPtr) + _offset.load();
            padding = CalculatePadding(currentAddress, alignment);

            if (_offset + padding + size > _totalSize)
            {
                printf("We overflowed our allocator");
                assert(false);

                return 0;
            }
            tempOffset = _offset.fetch_add(padding + size);
            _used.store(tempOffset + padding + size);
        }

        const std::size_t nextAddress = currentAddress + padding;

#ifdef _DEBUG
        if (_debug)
        {
            printf("%s\tAllocated \t@C %p \t@R %p \tO %u \tP %u", _name, reinterpret_cast<void*>(currentAddress), reinterpret_cast<void*>(nextAddress), tempOffset, padding);
        }
#endif
        _peak.store(std::max(_peak.load(), _used.load()));

        return nextAddress;
    }

    bool StackAllocator::TryAllocateOffset(const std::size_t size, const std::size_t alignment, size_t& offset)
    {
        assert(_initialized); // We need to initialize this allocator!

        std::size_t currentAddress = 0;
        std::size_t padding = 0;
        std::size_t tempOffset = 0;

        {
            std::lock_guard<std::mutex> lock(_lock);

            currentAddress = reinterpret_cast<std::size_t>(_startPtr) + _offset.load();
            padding = CalculatePadding(currentAddress, alignment);

            if (_offset + padding + size > _totalSize)
            {
                return false;
            }
            tempOffset = _offset.fetch_add(padding + size);
            _used.store(tempOffset + padding + size);
        }

        const std::size_t nextAddress = currentAddress + padding;

#ifdef _DEBUG
        if (_debug)
        {
            std::cout << _name << "\tAllocated " << "\t@C " << reinterpret_cast<void*>(currentAddress) << "\t@R " << reinterpret_cast<void*>(nextAddress) << "\tO " << offset << "\tP " << padding << std::endl;
        }
#endif
        _peak.store(std::max(_peak.load(), _used.load()));
        offset = nextAddress;
        return true;
    }

    void StackAllocator::Free(void* /*ptr*/) 
    {
        // A stack allocator doesn't do individual frees, it resets to 0 but this won't call destructors
    }

    void StackAllocator::Reset() 
    {
        _offset.store(0);
        _used.store(0);
        _peak.store(0);

#ifdef _DEBUG
        if (_debug)
        {
            printf("%s \tReset", _name);
        }
#endif
    }
}