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
	Author:		Zhur
*/

#include "EVEServerPCH.h"


PyService::PyService(PyServiceMgr *mgr, const char *serviceName)
: m_manager(mgr),
  m_name(serviceName)
{
}

PyService::~PyService()
{
}

//overload this to hack in our special bind routines at the service level
PyResult PyService::Call(const std::string &method, PyCallArgs &args) {
	if(method == "MachoBindObject") {
		_log(SERVICE__CALLS, "Service %s: handling MachoBindObject request directly", GetName());
		return(Handle_MachoBindObject(args));
	} else if(method == "MachoResolveObject"){
		_log(SERVICE__CALLS, "Service %s: handling MachoResolveObject request directly", GetName());
		return(Handle_MachoResolveObject(args));
	} else {
		_log(SERVICE__CALLS, "Service %s: calling %s", GetName(), method.c_str());
		args.Dump(SERVICE__CALL_TRACE);
		return(PyCallable::Call(method, args));
	}
}


/*                                                                              
 * For now, we are going to keep it simple by trying to use only a single
 * node ID, this may have to change some day.
*/

PyResult PyService::Handle_MachoResolveObject(PyCallArgs &call) {
/*	CallMachoResolveObject args;
	if(!args.Decode(!call.packet)) {
		_log(CLIENT__ERROR, "Failed to decode params for MachoResolveObject.");
		return NULL;
	}
*/
	//returns nodeID
	_log(CLIENT__MESSAGE, "%s Service: MachoResolveObject requested, returning %u", GetName(), m_manager->GetNodeID());
	return(new PyInt(m_manager->GetNodeID()));
}


PyResult PyService::Handle_MachoBindObject( PyCallArgs& call )
{
	CallMachoBindObject args;
	if( !args.Decode( &call.tuple ) )
    {
		codelog( SERVICE__ERROR, "%s Service: %s: Failed to decode arguments", GetName(), call.client->GetName() );
		return NULL;
	}

	_log( SERVICE__MESSAGE, "%s Service: %s: Processing MachoBindObject", GetName(), call.client->GetName() );
	
	//first we need to get our implementation to actually create the object
	//which they are trying to bind to.
	PyBoundObject* our_obj = _CreateBoundObject( call.client, args.bindParams );
	if( NULL == our_obj )
    {
		_log( SERVICE__ERROR, "%s Service: %s: Unable to create bound object for:", GetName(), call.client->GetName() );
		args.bindParams->Dump(SERVICE__ERROR, "    ");

		return NULL;
	}

	PyTuple* robjs = new PyTuple( 2 );
	//now we register 
    robjs->SetItem( 0, m_manager->BindObject( call.client, our_obj ) );

	if( args.call->IsNone() )
		//no call was specified...
        robjs->SetItem( 1, new PyNone );
    else
    {
		CallMachoBindObject_call boundcall;
		if( !boundcall.Decode( &args.call ) )
        {
			codelog( SERVICE__ERROR, "%s Service: %s: Failed to decode boundcall arguments", GetName(), call.client->GetName() );
			return NULL;
		}
		
		_log( SERVICE__MESSAGE, "%s Service: MachoBindObject also contains call to %s", GetName(), boundcall.method_name.c_str() );
		
        PyCallArgs sub_args( call.client, boundcall.arguments, boundcall.dict_arguments );
		
		//do the call:
		PyResult result = our_obj->Call( boundcall.method_name, sub_args );

        PyIncRef( result.ssResult );
        robjs->SetItem( 1, result.ssResult );

		//ok, now we have finished our sub-call... hooray.
	}

	//ok, we have created and bound the object requested, as well as made any sub-call we needed to do.
	//we are done.
	return robjs;
}

PyBoundObject* PyService::_CreateBoundObject( Client* c, const PyRep* bind_args )
{
	_log( SERVICE__ERROR, "%s Service: Default _CreateBoundObject called, somebody didnt read the comments."
                          " This method must be overridden if bound objects are provided by the service!", GetName());
	bind_args->Dump( SERVICE__ERROR, "  Bind Args: " );

	//this is probably going to cause us to crash:
	return NULL;
}

const char *const PyService::s_checkTimeStrings[_checkCount] = {
     "always",
     "never",
     "year",
     "6 months",
     "3 months",
     "month",
     "week",
     "day",
     "12 hours",
     "6 hours",
     "3 hours",
     "2 hours",
     "1 hour",
     "30 minutes",
     "15 minutes",
     "5 minutes",
     "1 minute",
     "30 seconds",
     "15 seconds",
     "5 seconds",
     "1 second"
};

/* this is untested... */
PyObject *PyService::_BuildCachedReturn( PySubStream** in_result, const char* sessionInfo, CacheCheckTime check )
{
	objectCaching_CachedMethodCallResult cached;
	
	PySubStream* result = *in_result;
	*in_result = NULL;		//consume it.
	
	//we need to checksum the marshaled data...
	result->EncodeData();
	if( result->data() == NULL )
    {
		_log( SERVICE__ERROR, "%s: Failed to build cached return", GetName() );

        PyDecRef( result );
		return NULL;
	}
	
	cached.call_return = result;	//this entire result is going to get cloned in the Encode(), and then destroyed when we return... what a waste...
	cached.sessionInfo = sessionInfo;
	cached.clientWhen = s_checkTimeStrings[ check ];
	
	cached.timeStamp = Win32TimeNow();
	//we can use whatever checksum we want here, as the client just remembers it and sends it back to us.
	cached.version = crc_hqx( &result->data()->content()[0], result->data()->content().size(), 0 );
	
	return cached.Encode();
}
