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
    Author:     Zhur, mmcs
*/

#include "EVECommonPCH.h"

#include "python/PyXMLGenerator.h"
#include "python/PyRep.h"

PyXMLGenerator::PyXMLGenerator( FILE* into, const char* pfx )
: PyPfxVisitor( pfx ),
  mInto( into ),
  mItem( 0 )
{
}

bool PyXMLGenerator::VisitInteger( const PyInt* rep )
{
    fprintf( mInto, "%s<int name=\"integer%u\" />\n", _pfx(), mItem++ );

    return true;
}

bool PyXMLGenerator::VisitLong( const PyLong* rep )
{
    fprintf( mInto, "%s<long name=\"long%u\" />\n", _pfx(), mItem++ );

    return true;
}

bool PyXMLGenerator::VisitReal( const PyFloat* rep )
{
    fprintf( mInto, "%s<real name=\"real%u\" />\n", _pfx(), mItem++ );

    return true;
}

bool PyXMLGenerator::VisitBoolean( const PyBool* rep )
{
    fprintf( mInto, "%s<bool name=\"bool%u\" />\n", _pfx(), mItem++ );

    return true;
}

bool PyXMLGenerator::VisitNone( const PyNone* rep )
{
    fprintf( mInto, "%s<none />\n", _pfx() );

    return true;
}

bool PyXMLGenerator::VisitBuffer( const PyBuffer* rep )
{
    fprintf( mInto, "%s<buffer name=\"buffer%u\" />\n", _pfx(), mItem++ );

    return true;
}

bool PyXMLGenerator::VisitString( const PyString* rep )
{
    fprintf( mInto, "%s<string name=\"string%u\" />\n", _pfx(), mItem++ );

    return true;
}

bool PyXMLGenerator::VisitObject( const PyObject* rep )
{
    fprintf( mInto, "%s<objectInline>\n", _pfx() );

    _pfxExtend( "    " );
    bool res = PyPfxVisitor::VisitObject( rep );
    _pfxWithdraw();

    fprintf( mInto, "%s</objectInline>\n", _pfx() );

    return res;
}

bool PyXMLGenerator::VisitSubStruct( const PySubStruct* rep )
{
    fprintf( mInto, "%s<substructInline>\n", _pfx() );

    _pfxExtend( "    " );
    bool res = PyVisitor::VisitSubStruct( rep );
    _pfxWithdraw();

    fprintf( mInto, "%s</substructInline>\n", _pfx() );

    return res;
}

bool PyXMLGenerator::VisitSubStream( const PySubStream* rep )
{
    fprintf( mInto, "%s<substreamInline>\n", _pfx() );

    _pfxExtend( "    " );
    bool res = PyVisitor::VisitSubStream( rep );
    _pfxWithdraw();

    fprintf( mInto, "%s</substreamInline>\n", _pfx() );

    return res;
}

bool PyXMLGenerator::VisitDict( const PyDict* rep )
{
    enum
    {
        DictInline,
        DictStringKey,
        DictIntKey,
        DictRaw
    } ktype;

    enum
    {
        ValueUnknown,
        ValueString,
        ValueInt,
        ValueReal,
        ValueMixed
    } vtype;

    ktype = DictInline;
    vtype = ValueUnknown;

    //this is kinda a hack, but we want to try and classify the contents of this dict:
    PyDict::const_iterator cur, end;
    cur = rep->begin();
    end = rep->end();
    for(; cur != end; ++cur)
    {
        if( cur->first->IsString() )
        {
            if( ktype == DictIntKey )
            {
                //we have varying key types, raw dict it is.
                ktype = DictRaw;
                break;
            }
            else if( ktype == DictInline )
                ktype = DictStringKey;
        }
        else if( cur->first->IsInt() )
        {
            if( ktype == DictStringKey )
            {
                //we have varying key types, raw dict it is.
                ktype = DictRaw;
                break;
            }
            else if( ktype == DictInline )
                ktype = DictIntKey;
        }
        else
        {
            //anything but those key types is more than we can think about, keep it raw.
            ktype = DictRaw;
            break;
        }

        if( cur->second->IsString() )
        {
            if( vtype == ValueInt || vtype == ValueReal )
                vtype = ValueMixed;
            else if( vtype == ValueUnknown )
                vtype = ValueString;
        }
        else if(cur->second->IsInt())
        {
            if( vtype == ValueString || vtype == ValueReal )
                vtype = ValueMixed;
            else if( vtype == ValueUnknown )
                vtype = ValueInt;
        }
        else if( cur->second->IsFloat() )
        {
            if( vtype == ValueString || vtype == ValueInt )
                vtype = ValueMixed;
            else if(vtype == ValueUnknown)
                vtype = ValueReal;
        }
        else
            vtype = ValueMixed;
    }

    if( ktype == DictRaw )
    {
        fprintf( mInto, "%s<dict name=\"dict%u\" />\n", _pfx(), mItem++ );
        return true;
    }
    else if( ktype == DictIntKey )
    {
        //cant do an inline dict, but can try a vector
        switch( vtype )
        {
        case ValueString:
            fprintf( mInto, "%s<dictInt name=\"dict%u\" type=\"string\" />\n", _pfx(), mItem++ );
            break;
        case ValueInt:
            fprintf( mInto, "%s<dictInt name=\"dict%u\" type=\"int\" />\n", _pfx(), mItem++ );
            break;
        case ValueReal:
            fprintf( mInto, "%s<dictInt name=\"dict%u\" type=\"real\" />\n", _pfx(), mItem++ );
            break;
        case ValueUnknown:
        case ValueMixed:
            fprintf( mInto, "%s<dictRaw name=\"dict%u\" />\n", _pfx(), mItem++ );
            break;
        }

        return true;
    }

    fprintf( mInto, "%s<dictInline>\n", _pfx() );

    _pfxExtend( "    " );

    //! visit dict elements.
    cur = rep->begin();
    end = rep->end();
    for(; cur != end; ++cur)
    {
        if( !cur->first->IsString() )
        {
            fprintf( mInto, "%s<!-- non-string dict key of type %s -->\n", _pfx(), cur->first->TypeString() );
            return false;
        }
        PyString* str = cur->first->AsString();

        fprintf( mInto, "%s<dictInlineEntry key=\"%s\">\n", _pfx(), str->content().c_str() );

        _pfxExtend( "    " );
        cur->second->visit( *this );
        _pfxWithdraw();

        fprintf( mInto, "%s</dictInlineEntry>\n", _pfx() );
    }

    _pfxWithdraw();

    fprintf( mInto, "%s</dictInline>\n", _pfx() );

    return true;
}

bool PyXMLGenerator::VisitList( const PyList* rep )
{
    //for now presume we cant do anything useful with lists that contain
    //more than a few things...
    if( rep->size() < 5 )
    {
        fprintf( mInto, "%s<listInline>\n", _pfx() );

        _pfxExtend( "    " );

        //! visit the list elements.
        PyList::const_iterator cur, end;
        cur = rep->begin();
        end = rep->end();
        for( uint32 i = 0; cur != end; ++cur, ++i )
        {
            fprintf( mInto, "%s<!-- %u -->\n", _pfx(), i );
            (*cur)->visit( *this );
        }

        _pfxWithdraw();

        fprintf( mInto, "%s</listInline>\n", _pfx() );
    }
    else
    {
        enum
        {
            TypeUnknown,
            TypeString,
            TypeInteger,
            TypeReal,
            TypeMixed
        } eletype;
        eletype = TypeUnknown;

        //scan the list to see if we can classify the contents.
        PyList::const_iterator cur, end;
        cur = rep->begin();
        end = rep->end();
        for(; cur != end; cur++)
        {
            if( (*cur)->IsString() )
            {
                if( eletype == TypeInteger || eletype == TypeReal )
                {
                    eletype = TypeMixed;
                    break;
                }
                else if( eletype == TypeUnknown )
                    eletype = TypeString;
            }
            else if( (*cur)->IsInt() )
            {
                if( eletype == TypeString || eletype == TypeReal )
                {
                    eletype = TypeMixed;
                    break;
                }
                else if( eletype == TypeUnknown )
                    eletype = TypeInteger;
            }
            else if( (*cur)->IsFloat() )
            {
                if( eletype == TypeString || eletype == TypeInteger )
                {
                    eletype = TypeMixed;
                    break;
                }
                else if( eletype == TypeUnknown )
                    eletype = TypeReal;
            }
            else
            {
                eletype = TypeMixed;
                break;
            }
        }

        switch( eletype )
        {
        case TypeString:
            fprintf( mInto, "%s<listStr name=\"list%u\" />\n", _pfx(), mItem++ );
            break;
        case TypeInteger:
            fprintf( mInto, "%s<listInt name=\"list%u\" />\n", _pfx(), mItem++ );
            break;
        case TypeReal:
            fprintf( mInto, "%s<listReal name=\"list%u\" />\n", _pfx(), mItem++ );
            break;
        case TypeUnknown:
        case TypeMixed:
            fprintf( mInto, "%s<list name=\"list%u\" />\n", _pfx(), mItem++ );
            break;
        }
    }

    return true;
}

bool PyXMLGenerator::VisitTuple( const PyTuple* rep )
{
    fprintf( mInto, "%s<tupleInline>\n", _pfx() );

    _pfxExtend( "    " );

    //! visits the tuple elements.
    PyTuple::const_iterator cur, end;
    cur = rep->begin();
    end = rep->end();
    for(uint32 i = 0; cur != end; ++cur, ++i)
    {
        fprintf( mInto, "%s<!-- %d -->\n", _pfx(), i );
        (*cur)->visit( *this );
    }

    _pfxWithdraw();

    fprintf( mInto, "%s</tupleInline>\n", _pfx() );

    return true;
}




















