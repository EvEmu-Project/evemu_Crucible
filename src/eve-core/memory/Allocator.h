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
#pragma once
#include <string>
#include <cstdio>
#include <cassert>
#include <atomic>

#ifdef __APPLE__
#include <cstddef> // Ensure std::size_t is defined on Apple platforms
#endif

namespace Memory
{
    class Allocator
    {
    public:
        Allocator();
        virtual ~Allocator();

        virtual void* Allocate(const std::size_t size, const std::size_t alignment = 0) = 0;
        virtual bool TryAllocate(const std::size_t size, const std::size_t alignment, void*& memory) = 0;
        virtual size_t AllocateOffset(const std::size_t size, const std::size_t alignment = 0) = 0;
        virtual bool TryAllocateOffset(const std::size_t size, const std::size_t alignment, size_t& offset) = 0;

        virtual void Free(void* ptr) = 0;
        virtual void Init(const std::size_t totalSize, std::string name = "", bool onlyOffsets = false, bool debug = false);

        template<typename T, typename... Args>
        static T* New(Allocator* allocator, Args&& ... args)
        {
            if (allocator->_onlyOffset)
            {
                printf("Tried to Allocator::New with an allocator that is set to onlyOffset and thus has no backing memory");
                assert(false);
            }

            void* memory = allocator->Allocate(sizeof(T), 8);
            T* object = new(memory) T(std::forward<Args>(args)...);

            return object;
        }

        template<typename T, typename... Args>
        static T* NewArray(Allocator* allocator, size_t count, Args&& ... args)
        {
            if (allocator->_onlyOffset)
            {
                printf("Tried to Allocator::NewArray with an allocator that is set to onlyOffset and thus has no backing memory");
                assert(false);
            }

            void* memory = allocator->Allocate(sizeof(T) * count, 8);
            T* typedArray = static_cast<T*>(memory);
            
            return typedArray;
        }

    protected:
        std::atomic<std::size_t> _totalSize;
        std::atomic<std::size_t> _used;
        std::atomic<std::size_t> _peak;

        bool _debug;
        bool _initialized = false;
        bool _onlyOffset = false;
        std::string _name;
    };
}