/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
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
    Author:        Bloody.Rabbit
*/

#include "eve-common.h"

#include "python/classes/PyExceptions.h"
#include "EVEVersion.h"


/* PyException */
PyException::PyException( PyRep* except ) : ssException( except != nullptr ? except : PyStatic.NewNone()) {}
PyException::PyException( const PyException& oth ) : ssException( nullptr ) { *this = oth; }
PyException::~PyException() { PySafeDecRef( ssException ); }

PyException& PyException::operator=( const PyException& oth )
{
    PySafeDecRef( ssException );
    ssException = oth.ssException;
    PySafeIncRef( ssException );

    return *this;
}

GPSTransportClosed::GPSTransportClosed( const char* reason )
: PyObjectEx_Type1( new PyToken( "exceptions.GPSTransportClosed" ), _CreateArgs( reason ), _CreateKeywords( reason ) )
{
}

GPSTransportClosed::GPSTransportClosed( std::string& reason )
: PyObjectEx_Type1( new PyToken( "exceptions.GPSTransportClosed" ), _CreateArgs( reason.c_str() ), _CreateKeywords( reason.c_str() ) )
{

}
void GPSTransportClosed::AddKeyword( const char* name, PyRep* value )
{
    GetReasonArgs()->SetItemString( name, value );
}

PyDict* GPSTransportClosed::GetReasonArgs() const
{
    PyRep* r = FindKeyword( "reasonArgs" );
    assert( r );

    return r->AsDict();
}

PyTuple* GPSTransportClosed::_CreateArgs( const char* reason )
{
    PyTuple* args = new PyTuple( 1 );
    args->SetItem( 0, new PyString( reason ) );

    return args;
}

PyDict* GPSTransportClosed::_CreateKeywords( const char* reason )
{
    PyDict* keywords = new PyDict;
    //keywords->SetItemString( "origin", new PyString( "proxy" ) );
    keywords->SetItemString( "reasonArgs", new PyDict );
    keywords->SetItemString( "clock", new PyLong( GetFileTimeNow() ) );
    //keywords->SetItemString( "loggedOnUserCount", );
    keywords->SetItemString( "region", new PyString( EVEProjectRegion ) );
    keywords->SetItemString( "reason", new PyString( reason ) );
    keywords->SetItemString( "version", new PyFloat( EVEVersionNumber ) );
    keywords->SetItemString( "build", new PyInt( EVEBuildVersion ) );
    //keywords->SetItemString( "reasonCode", );
    keywords->SetItemString( "codename", new PyString( EVEProjectCodename ) );
    keywords->SetItemString( "machoVersion", new PyInt( MachoNetVersion ) );

    return keywords;
}



UserError::UserError (const char* exceptionType)
: PyException (new PyObjectEx_Type1 (new PyToken (EXCEPTION_NAME), _CreateArgs (exceptionType), _CreateKeywords (exceptionType)))
{
}

UserError& UserError::AddFormatValue (const char* name, PyRep* value)
{
    this->AddKeyword (name, value);

    return *this;
}

UserError& UserError::AddParameterKeyword (const char* name, UserError_ParameterIDs type, PyRep* value, PyRep* value2)
{
    PyTuple* param = new PyTuple (value2 == nullptr ? 2 : 3);

    param->SetItem (0, new PyInt (type));
    param->SetItem (1, value);

    if (value2 != nullptr)
        param->SetItem (2, value2);

    return this->AddFormatValue (name, param);
}

UserError& UserError::AddDateTime (const char* name, time_t date)
{
    return this->AddParameterKeyword (name, UserError_Parameter_DateTime, new PyLong (date));
}

UserError& UserError::AddDate (const char* name, time_t date)
{
    return this->AddParameterKeyword (name, UserError_Parameter_Date, new PyLong (date));
}

UserError& UserError::AddTime (const char* name, time_t time)
{
    return this->AddParameterKeyword (name, UserError_Parameter_Time, new PyLong (time));
}

UserError& UserError::AddTimeShort (const char* name, time_t time)
{
    return this->AddParameterKeyword (name, UserError_Parameter_TimeShort, new PyLong (time));
}
UserError& UserError::AddA (const char* name, const char* value)
{
    return this->AddParameterKeyword (name, UserError_Parameter_A, new PyString (value));
}

UserError& UserError::AddThe (const char* name, const char* value)
{
    return this->AddParameterKeyword (name, UserError_Parameter_The, new PyString (value));
}

UserError& UserError::AddUELocalization (const char* name, const char* strKey, PyDict* args)
{
    return this->AddParameterKeyword (name, UserError_Parameter_Localization, new PyString (strKey), args);
}

UserError& UserError::AddList (const char* name, PyList* listEntries, const char* separator)
{
    if (separator == nullptr)
        return this->AddParameterKeyword (name, UserError_Parameter_List, listEntries);
    else
        return this->AddParameterKeyword (name, UserError_Parameter_List, listEntries, new PyString (separator));
}

UserError& UserError::AddOwnerName (const char* name, uint32 ownerID)
{
    return this->AddParameterKeyword (name, UserError_Parameter_OwnerName, new PyLong (ownerID));
}

UserError& UserError::AddOwnerNick (const char* name, uint32 ownerID)
{
    return this->AddParameterKeyword (name, UserError_Parameter_OwnerNick, new PyLong (ownerID));
}

UserError& UserError::AddLocationName (const char* name, uint32 locationID)
{
    return this->AddParameterKeyword (name, UserError_Parameter_LocationName, new PyLong (locationID));
}

UserError& UserError::AddTypeName (const char* name, uint32 typeID)
{
    return this->AddParameterKeyword (name, UserError_Parameter_TypeName, new PyInt (typeID));
}

UserError& UserError::AddTypeDescription (const char* name, uint32 typeID)
{
    return this->AddParameterKeyword (name, UserError_Parameter_TypeDescription, new PyInt (typeID));
}

UserError& UserError::AddTypeList (const char* name, PyList* typeIDs)
{
    return this->AddParameterKeyword (name, UserError_Parameter_TypeIDList, typeIDs);
}

UserError& UserError::AddBlueprintTypeName (const char* name, uint32 bpTypeID)
{
    return this->AddParameterKeyword (name, UserError_Parameter_BluePrintTypeName, new PyInt (bpTypeID));
}

UserError& UserError::AddGroupName (const char* name, uint32 groupID)
{
    return this->AddParameterKeyword (name, UserError_Parameter_GroupName, new PyInt (groupID));
}

UserError& UserError::AddGroupDescription (const char* name, uint32 groupID)
{
    return this->AddParameterKeyword (name, UserError_Parameter_GroupDescription, new PyInt (groupID));
}

UserError& UserError::AddCategoryName (const char* name, uint32 categoryID)
{
    return this->AddParameterKeyword (name, UserError_Parameter_CategoryName, new PyInt (categoryID));
}

UserError& UserError::AddCategoryDescription (const char* name, uint32 categoryID)
{
    return this->AddParameterKeyword (name, UserError_Parameter_CategoryDescription, new PyInt (categoryID));
}

UserError& UserError::AddAmount (const char* name, int quantity)
{
    return this->AddParameterKeyword (name, UserError_Parameter_Amount, new PyInt (quantity));
}

UserError& UserError::AddAmount (const char* name, uint quantity)
{
    return this->AddParameterKeyword (name, UserError_Parameter_Amount, new PyInt (quantity));
}

UserError& UserError::AddAmount (const char* name, double quantity)
{
    return this->AddParameterKeyword (name, UserError_Parameter_Amount, new PyFloat (quantity));
}

UserError& UserError::AddISK (const char* name, double isk)
{
    return this->AddParameterKeyword (name, UserError_Parameter_ISK, new PyFloat (isk));
}

UserError& UserError::AddAUR (const char* name, double aur)
{
    return this->AddParameterKeyword (name, UserError_Parameter_AUR, new PyFloat (aur));
}

UserError& UserError::AddDistance (const char* name, double distance)
{
    return this->AddParameterKeyword (name, UserError_Parameter_Distance, new PyFloat (distance));
}

UserError& UserError::AddTypeIDAndQuantity (const char* name, uint32 typeID, int quantity)
{
    return this->AddParameterKeyword (name, UserError_Parameter_TypeIDAndQuantity, new PyInt (typeID), new PyInt (quantity));
}

void UserError::AddKeyword( const char* name, PyRep* value )
{
    // We need 2 refs ... the first one is given to us,
    // but we must create the second one ...
    PyIncRef( value );

    _GetTupleKeywords()->SetItemString( name, value );
    _GetDictKeywords()->SetItemString( name, value );
}

PyDict* UserError::_GetTupleKeywords() const
{
    return this->m_args->GetItem (1)->AsDict();
}

PyDict* UserError::_GetDictKeywords() const
{
    return this->m_keywords->GetItemString ("dict")->AsDict ();
}

PyTuple* UserError::_CreateArgs( const char* msg )
{
    this->m_args = new PyTuple (2);

    this->m_args->SetItem (0, new PyString (msg));
    this->m_args->SetItem (1, new PyDict);

    return this->m_args;
}

PyDict* UserError::_CreateKeywords( const char* msg )
{
    this->m_keywords = new PyDict;

    this->m_keywords->SetItemString ("msg", new PyString (msg));
    this->m_keywords->SetItemString ("dict", new PyDict);

    return this->m_keywords;
}

CustomError::CustomError(const char *fmt, ...)
: UserError (EXCEPTION_NAME)
{
    va_list va;
    va_start (va, fmt);

    char* str = nullptr;
    vasprintf (&str, fmt, va);
    assert (str);

    va_end (va);

    this->AddFormatValue ("error", new PyString (str));

    SafeFree (str);
}

const char* CustomError::EXCEPTION_NAME = "CustomError";
const char* UserError::EXCEPTION_NAME = "ccp_exceptions.UserError";