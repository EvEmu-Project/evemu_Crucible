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
#include "Allocator.h"

namespace Memory
{
    Allocator::Allocator()
    {
        
    }

    Allocator::~Allocator() 
    {
        _totalSize = 0;
    }

    void Allocator::Init(const std::size_t totalSize, std::string name, bool onlyOffsets, bool debug)
    {
        _totalSize = totalSize;
        _used = 0;
        _peak = 0;

        _name = name;
        _debug = debug;
        _initialized = false;
        _onlyOffset = onlyOffsets;
    }
}
