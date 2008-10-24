/*  EVEmu: EVE Online Server Emulator

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY except by those people which sell it, which
  are required to give you total support for your newly bought product;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "EvemuPCH.h"


PyService::PyService(PyServiceMgr *mgr, const char *serviceName)
: PyCallable(mgr, serviceName)
{
}

PyService::~PyService()
{
}

bool PyService::IsPacketFor(const PyPacket *packet) const {
	return(packet->dest.service == GetName());	//assume one of these is an std::string
}

//overload this to hack in our special bind routines at the service level
PyResult PyService::Call(PyCallStream &call, PyCallArgs &args) {
	if(call.method == "MachoBindObject") {
		_log(SERVICE__CALLS, "%s Service: handling MachoBindObject request directly", GetName());
		return(Handle_MachoBindObject(args));
	} else if(call.method == "MachoResolveObject"){
		_log(SERVICE__CALLS, "%s Service: handling MachoResolveObject request directly", GetName());
		return(Handle_MachoResolveObject(args));
	}

	return(PyCallable::Call(call, args));
}


/*                                                                              
 * For now, we are going to keep it simple by trying to use only a single
 * node ID, this may have to change some day.
 *
*/

PyResult PyService::Handle_MachoResolveObject(PyCallArgs &call) {
/*	CallMachoResolveObject args;
	if(!args.Decode(!call.packet)) {
		_log(CLIENT__ERROR, "Failed to decode params for MachoResolveObject.");
		return(NULL);
	}
*/
	//returns nodeID
	_log(CLIENT__MESSAGE, "%s Service: MachoResolveObject requested, returning %lu", GetName(), m_manager->GetNodeID());
	return(new PyRepInteger(m_manager->GetNodeID()));
}


PyResult PyService::Handle_MachoBindObject(PyCallArgs &call) {
	CallMachoBindObject args;
	if(!args.Decode(&call.tuple)) {
		codelog(SERVICE__ERROR, "%s Service: %s: Failed to decode arguments", GetName(), call.client->GetName());
		return(NULL);
	}

	_log(SERVICE__MESSAGE, "%s Service: %s: Processing MachoBindObject", GetName(), call.client->GetName());
	
	//first we need to get our implementation to actually create the object
	//which they are trying to bind to.
	PyBoundObject *our_obj;
	our_obj = _CreateBoundObject(call.client, args.bindParams);
	if(our_obj == NULL) {
		_log(SERVICE__ERROR, "%s Service: %s: Unable to create bound object for:", GetName(), call.client->GetName());
		args.bindParams->Dump(SERVICE__ERROR, "    ");
		return(NULL);
	}

	PyRepTuple *robjs = new PyRepTuple(2);
	//now we register 
	robjs->items[0] = m_manager->BindObject(call.client, our_obj);

	if(args.call->CheckType(PyRep::None)) {
		//no call was specified...
		robjs->items[1] = new PyRepNone();
	} else {
		CallMachoBindObject_call boundcall;
		if(!boundcall.Decode(&args.call)) {
			codelog(SERVICE__ERROR, "%s Service: %s: Failed to decode boundcall arguments", GetName(), call.client->GetName());
			return(NULL);
		}
		
		_log(SERVICE__MESSAGE, "%s Service: MachoBindObject also contains call to %s", GetName(), boundcall.method_name.c_str());

		//we have to build our own call stream:
		PyCallStream sub_call;
		sub_call.remoteObject = 1;
		sub_call.method = boundcall.method_name;
		sub_call.arg_tuple = NULL;	//stored directly in the args below
		sub_call.arg_dict = NULL;

		//grr.. ownership transfer.
		PyRepDict *tmp_dict = new PyRepDict();
		tmp_dict->items = boundcall.dict_arguments.items;
		boundcall.dict_arguments.items.clear();
		
		PyCallArgs sub_args(call.client, &boundcall.arguments, &tmp_dict);
		
		PyResult result = our_obj->Call(sub_call, sub_args);

		robjs->items[1] = result.ssResult.hijack();

		//ok, now we have finished our sub-call... hooray.
	}

	//ok, we have created and bound the object requested, as well as made any sub-call we needed to do.
	//we are done.
	return(robjs);
}

PyBoundObject *PyService::_CreateBoundObject(Client *c, const PyRep *bind_args) {
	_log(SERVICE__ERROR, "%s Service: Default _CreateBoundObject called, somebody didnt read the comments. This method must be overridden if bound objects are provided by the service!", GetName());
	bind_args->Dump(stdout, "  Bind Args: ");
	//this is prolly going to cause us to crash:
	return(NULL);
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
PyRepObject *PyService::_BuildCachedReturn(PyRepSubStream **in_result, const char *sessionInfo, CacheCheckTime check) {
	objectCaching_CachedMethodCallResult cached;
	
	PyRepSubStream *result = *in_result;
	*in_result = NULL;		//consume it.
	
	//we need to checksum the marshaled data...
	result->EncodeData();
	if(result->data == NULL) {
		_log(SERVICE__ERROR, "%s: Failed to build cached return", GetName());
		delete result;
		return(NULL);
	}
	
	cached.call_return = result;	//this entire result is going to get cloned in the Encode(), and then destroyed when we return... what a waste...
	cached.sessionInfo = sessionInfo;
	cached.clientWhen = s_checkTimeStrings[check];
	
	cached.timeStamp = Win32TimeNow();
	//we can use whatever checksum we want here, as the client just remembers it and sends it back to us.
	cached.version = CalcCRC16(result->data, result->length, 0);
	
	return(cached.Encode());
}
















