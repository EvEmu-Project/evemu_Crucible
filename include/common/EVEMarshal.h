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

#include "common.h"
#include "EVEMarshalOpcodes.h"
#include "PyVisitor.h"

/*
 * @brief Marshal Stream builder.
 *
 * @param[in]  rep Python object to marshal.
 * @param[out] len Variable which receives length of resulting buffer.
 *
 * @return Ownership of buffer with marshaled data.
 */
extern uint8* Marshal(const PyRep* rep, uint32& len);
/*
 * @brief Deflated Marshal Stream builder.
 *
 * @param[in]  rep            Python object to marshal.
 * @param[out] len            Variable which receives length of resulting buffer.
 * @param[in]  deflationLimit The least size of buffer which gets deflated.
 *
 * @return Ownership of buffer with deflated marshaled data.
 */
extern uint8* MarshalDeflate(const PyRep* rep, uint32& len, const uint32 deflationLimit = 0x2000);

class MarshalVisitor : public PyVisitor
{
public:
    EVEMU_INLINE MarshalVisitor() : mBuffer( NULL ), mWriteIndex( 0 ), mAllocatedMem( 0 )
    {
        Put<uint8>( MarshalHeaderByte );

        /*
         * Mapcount
         * the amount of referenced objects within a marshal stream.
         * Note: Atm not supported.
         */
        Put<uint32>( 0 ); // Mapcount
    }
    EVEMU_INLINE ~MarshalVisitor()
    {
        SafeFree( mBuffer );
    }

    /** @return marshaled data. */
    EVEMU_INLINE const uint8* data() const
    {
        return mBuffer;
    }
    /**
     * @brief Transfers ownership of marshaled data.
     *
     * @param[out] data Receives pointer to marshaled data.
     *
     * @return Length of marshaled data.
     */
    EVEMU_INLINE uint32 data( uint8** data )
    {
        *data = mBuffer;
        uint32 len = mWriteIndex;

        mBuffer = NULL;
        mWriteIndex = mAllocatedMem = 0;

        return len;
    }
    /** @return size of marshaled data. */
    EVEMU_INLINE uint32 size() const
    {
        return mWriteIndex;
    }

    /**
     * adds a integer to the data stream
     *
     * @note assuming the value is unsigned
     *       research shows that Op_PyByte can be negative
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
    EVEMU_INLINE void reserve(uint32 size)
    {
        uint32 neededMem = mWriteIndex + size;
        if( neededMem > mAllocatedMem )
        {
            mBuffer = (uint8*)realloc( mBuffer, neededMem + 0x1000 );
            mAllocatedMem = neededMem + 0x1000;
        }
    }

public:
    /** adds given value to the data stream */
    template<typename X>
    EVEMU_INLINE void Put( const X& value )
    {
        reserve( sizeof( X ) );
        *(X*)&mBuffer[mWriteIndex] = value;
        mWriteIndex += sizeof( X );
    }

    /** adds given bytes to the data stream */
    EVEMU_INLINE void Put(const void* v, uint32 len)
    {
        reserve( len );
        memcpy( &mBuffer[mWriteIndex], v, len );
        mWriteIndex += len;
    }

    /** utility for extended size. */
    EVEMU_INLINE void PutSizeEx(uint32 size)
    {
        if( size < 0xFF )
        {
            Put<uint8>( size );
        }
        else
        {
            Put<uint8>( 0xFF );
            Put<uint32>( size );
        }
    }

protected:
    uint8*              mBuffer;
    uint32              mWriteIndex;
    uint32              mAllocatedMem;
};

#endif

