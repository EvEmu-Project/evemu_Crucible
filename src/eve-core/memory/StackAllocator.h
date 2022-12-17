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
#include "Allocator.h"
#include <mutex>

namespace Memory
{
    class StackAllocator : public Allocator
    {
    public:
        StackAllocator();

        virtual ~StackAllocator();

        virtual void* Allocate(const std::size_t size, const std::size_t alignment = 0) override;
        virtual bool TryAllocate(const std::size_t size, const std::size_t alignment, void*& memory) override;
        virtual size_t AllocateOffset(const std::size_t size, const std::size_t alignment = 0) override;
        virtual bool TryAllocateOffset(const std::size_t size, const std::size_t alignment, size_t& offset) override;

        virtual void Free(void* ptr) override;
        virtual void Init(const std::size_t totalSize, std::string name = "", bool onlyOffsets = false, bool debug = false) override;
        virtual void Reset();

    protected:
        void* _startPtr = nullptr;
        std::atomic<std::size_t> _offset;
        std::mutex _lock;

    private:
        StackAllocator(StackAllocator& stackAllocator);
    };
}