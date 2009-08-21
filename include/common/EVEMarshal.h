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
    Author:     Zhur
*/

#ifndef EVE_MARSHAL_H
#define EVE_MARSHAL_H

#include "types.h"
#include "PyVisitor.h"

//returns ownership!
//default deflate to false as deflation is not implemented yet
extern uint8 *Marshal(const PyRep *rep, uint32 &len, bool inlineSubStream = true);

class MarshalVisitor : public PyVisitor
{
public:
    EVEMU_INLINE MarshalVisitor() : mWriteIndex(0), mAllocatedMem( 0x1000 ), mBuffer(NULL) {
        mBuffer = (uint8*)malloc( mAllocatedMem );
    }
    EVEMU_INLINE ~MarshalVisitor() {
        free( mBuffer );
    }

    /** adds a integer to the data stream
     *  Note:  assuming the value is unsigned
     *         research shows that Op_PyByte can be negative
     */
    void VisitInteger(const PyInt *rep);
    void VisitLong(const PyLong *rep);
    //! Adds a boolean to the stream
    void VisitBoolean(const PyBool *rep);
    //! Adds a double to the stream
    void VisitReal(const PyFloat *rep);
    //! Adds a None object to the stream
    void VisitNone(const PyNone *rep);
    //! Adds a buffer to the stream
    void VisitBuffer(const PyBuffer *rep);
    //! Adds a packed row to the stream
    void VisitPackedRow(const PyPackedRow *rep);
    //! add a string object to the data stream
    void VisitString(const PyString *rep);
    //! Adds a New object to the stream
    void VisitObject(const PyObject *rep);
    void VisitObjectEx(const PyObjectEx *rep);
    void VisitObjectExHeader(const PyObjectEx *rep);
    void VisitObjectExList(const PyObjectEx *rep);
    void VisitObjectExListElement(const PyObjectEx *rep, uint32 index, const PyRep *ele);
    void VisitObjectExDict(const PyObjectEx *rep);
    void VisitObjectExDictElement(const PyObjectEx *rep, uint32 index, const PyRep *key, const PyRep *value);
    //! Adds a sub structure to the stream
    void VisitSubStruct(const PySubStruct *rep);
    //! Adds a sub stream to the stream
    void VisitSubStream(const PySubStream *rep);
    void VisitChecksumedStream(const PyChecksumedStream *rep);

    void VisitDict(const PyDict *rep);
    void VisitDictElement(const PyDict *rep, uint32 index, const PyRep *key, const PyRep *value);

    void VisitList(const PyList *rep);
    void VisitListElement(const PyList *rep, uint32 index, const PyRep *ele);

    void VisitTuple(const PyTuple *rep);
    void VisitTupleElement(const PyTuple *rep, uint32 index, const PyRep *ele);

private:
    // utility to handle Op_PyVarInteger (a bit hacky......)
    void _PyInt_AsByteArray(const PyLong* v);

    // not very efficient but it will do for now
    EVEMU_INLINE void reserve(uint32 size) {
        uint32 neededMem = mWriteIndex + size;
        if (neededMem > mAllocatedMem)
        {
            mBuffer = (uint8*)realloc(mBuffer, neededMem + 0x1000);
            mAllocatedMem = neededMem + 0x1000;
        }
    }

public:

    EVEMU_INLINE void PutByte(uint8 b) {
        reserve(1);
        mBuffer[mWriteIndex] = b;
        mWriteIndex++;
    }

    EVEMU_INLINE void PutSizeEx(uint32 size) {
        if (size < 0xFF)
        {
            reserve(1);
            mBuffer[mWriteIndex] = (uint8)size;
            mWriteIndex++;
        }
        else
        {
            reserve(5);
            mBuffer[mWriteIndex++] = 0xFF;
            (*((uint32*)&mBuffer[mWriteIndex])) = size;
            mWriteIndex+=4;
        }
    }

    EVEMU_INLINE void PutBytes(const void *v, uint32 len) {
        reserve( len );
        memcpy( &mBuffer[mWriteIndex], v, len );
        mWriteIndex += len;
    }

    /** adds a double do the data stream
     */
    EVEMU_INLINE void PutDouble(const double& value) {
        reserve(8);
        (*((double*)&mBuffer[mWriteIndex])) = value;
        mWriteIndex+=8;
    }

    /** adds a uint64 do the data stream
     */
    EVEMU_INLINE void PutUint64(const uint64& value) {
        reserve(8);
        (*((uint64*)&mBuffer[mWriteIndex])) = value;
        mWriteIndex+=8;
    }

    /** adds a uint64 do the data stream
     */
    EVEMU_INLINE void PutInt64(const int64& value) {
        reserve(8);
        (*((int64*)&mBuffer[mWriteIndex])) = value;
        mWriteIndex+=8;
    }

    /** adds a uint32 do the data stream
     */
    EVEMU_INLINE void PutUint32(const uint32 value) {
        reserve(4);
        (*((uint32*)&mBuffer[mWriteIndex])) = value;
        mWriteIndex+=4;
    }

    /** adds a uint32 do the data stream
    */
    EVEMU_INLINE void PutInt32(const int32 value) {
        reserve(4);
        (*((int32*)&mBuffer[mWriteIndex])) = value;
        mWriteIndex+=4;
    }

    /** adds a uint16 do the data stream
      */
    EVEMU_INLINE void PutUint16(const uint16 value) {
        reserve(2);
        (*((uint16*)&mBuffer[mWriteIndex])) = value;
        mWriteIndex+=2;
    }

    EVEMU_INLINE void PutInt16(const int16 value) {
        reserve(2);
        (*((int16*)&mBuffer[mWriteIndex])) = value;
        mWriteIndex+=2;
    }

    /** adds a uint8 do the data stream
      */
    EVEMU_INLINE void PutUint8(uint8 value) {
        reserve(1);
        mBuffer[mWriteIndex] = value; mWriteIndex++;
    }

    /** adds a uint8 do the data stream
    */
    EVEMU_INLINE void PutInt8(int8 value) {
        reserve(1);
        (*((int16*)&mBuffer[mWriteIndex])) = value; mWriteIndex++;
    }

    EVEMU_INLINE uint32 size() {
        return mWriteIndex;
    }

    EVEMU_INLINE uint8* data() {
        return mBuffer;
    }

protected:

    uint32              mWriteIndex;
    uint32              mAllocatedMem;
    uint8               *mBuffer;
};

#endif

