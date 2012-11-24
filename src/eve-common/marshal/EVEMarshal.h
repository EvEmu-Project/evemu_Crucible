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
    Author:     Zhur
*/

#ifndef EVE_MARSHAL_H
#define EVE_MARSHAL_H

#include "marshal/EVEMarshalOpcodes.h"
#include "python/PyVisitor.h"

/*
 * @brief Marshal Stream builder.
 *
 * @param[in]  rep  Python object to marshal.
 * @param[out] into Buffer which receives marshaled stream.
 *
 * @retval true  Marshaling ran successfully.
 * @retval false Error occured during marshaling.
 */
extern bool Marshal( const PyRep* rep, Buffer& into );
/*
 * @brief Deflated Marshal Stream builder.
 *
 * @param[in]  rep            Python object to marshal.
 * @param[out] into           Buffer which receives deflated marshaled stream.
 * @param[in]  deflationLimit The least size of buffer which gets deflated.
 *
 * @retval true  Marshaling ran successfully.
 * @retval false Error occured during marshaling.
 */
extern bool MarshalDeflate( const PyRep* rep, Buffer& into, const uint32 deflationLimit = 0x2000 );

/**
 * @brief Turns Python objects into marshal bytecode.
 *
 * @author Captnoord, Bloody.Rabbit
 */
class MarshalStream
: protected PyVisitor
{
public:
    /** initializes object */
    MarshalStream();

    /** saves given rep to given buffer */
    bool Save( const PyRep* rep, Buffer& into );

protected:
    /** saves new stream with given rep. */
    bool SaveStream( const PyRep* rep );

    /** adds given value to the data stream */
    template<typename T>
    void Put( const T& value ) { mBuffer->Append<T>( value ); }
    /** adds given bytes to the data stream */
    template<typename Iter>
    void Put( Iter first, Iter last ) { mBuffer->AppendSeq<Iter>( first, last ); }

    /** utility for extended size. */
    void PutSizeEx( uint32 size )
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

    /**
     * adds a integer to the data stream
     *
     * @note assuming the value is unsigned
     *       research shows that Op_PyByte can be negative
     */
    bool VisitInteger( const PyInt* rep );
    //! Adds a long to the stream
    bool VisitLong( const PyLong* rep );
    //! Adds a boolean to the stream
    bool VisitBoolean( const PyBool* rep );
    //! Adds a double to the stream
    bool VisitReal( const PyFloat* rep );
    //! Adds a None object to the stream
    bool VisitNone( const PyNone* rep );
    //! Adds a buffer to the stream
    bool VisitBuffer( const PyBuffer* rep );
    //! add a string object to the data stream
    bool VisitString( const PyString* rep );
    //! add a wide string object to the data stream
    bool VisitWString( const PyWString* rep );
    //! add a token object to the data stream
    bool VisitToken( const PyToken* rep );

    /** Add a tuple object to the stream */
    bool VisitTuple( const PyTuple* rep );
    /** Add a list object to the stream */
    bool VisitList( const PyList* rep );
    /** Add a dict object to the stream */
    bool VisitDict( const PyDict* rep );

    //! Adds an object to the stream
    bool VisitObject( const PyObject* rep );
    //! Adds a New object to the stream
    bool VisitObjectEx( const PyObjectEx* rep );

    //! Adds a packed row to the stream
    bool VisitPackedRow( const PyPackedRow* rep );

    //! Adds a sub structure to the stream
    bool VisitSubStruct( const PySubStruct* rep );
    //! Adds a sub stream to the stream
    bool VisitSubStream( const PySubStream* rep );
    //! Adds a checksumed stream to the stream
    bool VisitChecksumedStream( const PyChecksumedStream* rep );

private:
    // utility to handle Op_PyVarInteger (a bit hacky......)
    void SaveVarInteger( const PyLong* v );
    // zero-compresses given buffer and adds it to the stream
    bool SaveZeroCompressed( const Buffer& data );

    Buffer* mBuffer;
};

#endif

