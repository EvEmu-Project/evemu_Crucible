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

EntityList::EntityList() : m_services( NULL ) {}
EntityList::~EntityList() {
	{
	    client_list::iterator cur, end;
	    cur = m_clients.begin();
	    end = m_clients.end();
	    for(; cur != end; cur++)
        {
		    delete *cur;
	    }
	}

	{
	    system_list::iterator cur, end;
	    cur = m_systems.begin();
	    end = m_systems.end();
	    for(; cur != end; cur++)
        {
		    delete cur->second;
	    }
	}
}

void EntityList::Add(Client **client) {
	if(client == NULL || *client == NULL)
		return;
	
	m_clients.push_back(*client);
	*client = NULL;
}

void EntityList::Process()
{
	Client *active_client = NULL;
	client_list::iterator client_cur = m_clients.begin();
	client_list::iterator client_end = m_clients.end();
    client_list::iterator client_tmp;

	while(client_cur != client_end)
	{
		active_client = *client_cur;
		if(!active_client->ProcessNet())
		{
            sLog.Log("Entity List", "Destroying client for account %u", active_client->GetAccountID());
            SafeDelete(active_client);

            client_tmp = client_cur++;
			m_clients.erase( client_tmp );
		}
		else
		{
			client_cur++;
		}
	}
	
	SystemManager *active_system = NULL;
	bool destiny = DestinyManager::IsTicActive();

    /* capt: I wonder what this stuff should do... its spamming the console... */
	//if( destiny == true )
	//{
        //sLog.Log("Entity List | Destiny Trace", "Triggering destiny tick for stamp %u", DestinyManager::GetStamp());
	//}
		
	//first process any systems, watching for deletion.
	system_list::iterator cur, end, tmp;
	cur = m_systems.begin();
	end = m_systems.end();
	while(cur != end)
	{
		active_system = cur->second;
		//if it is destiny time, process it first.
		if(destiny)
		{
			active_system->ProcessDestiny();
		}
		
		if(!active_system->Process())
        {
            sLog.Log("Entity List", "Destroying system");
			tmp = cur++;
			delete cur->second;
			m_systems.erase(tmp);
		}
        else
        {
			cur++;
		}
	}
	if( destiny == true )
	{
		DestinyManager::TicCompleted();
	}
}

Client *EntityList::FindCharacter(uint32 char_id) const {
	//could likely improve this with a map, but its a little more work since
	//clients are added to the list before we know who they are.
	
	client_list::const_iterator cur, end;
	cur = m_clients.begin();
	end = m_clients.end();
	for(; cur != end; cur++) {
		if((*cur)->GetCharacterID() == char_id)
			return(*cur);
	}
	return NULL;
}

Client *EntityList::FindCharacter(const char *name) const {
	//could likely improve this with a map, but its a little more work since
	//clients are added to the list before we know who they are.
	
	client_list::const_iterator cur, end;
	cur = m_clients.begin();
	end = m_clients.end();
	for(; cur != end; cur++) {
		CharacterRef c = (*cur)->GetChar();

		if( c ) {
			if( c->itemName() == name )
				return *cur;
		}
	}
	return NULL;
}

Client *EntityList::FindByShip(uint32 ship_id) const {
	//could likely improve this with a map, but its a little more work since
	//clients are added to the list before we know who they are.
	
	client_list::const_iterator cur, end;
	cur = m_clients.begin();
	end = m_clients.end();
	for(; cur != end; cur++) {
		if((*cur)->GetShipID() == ship_id)
			return(*cur);
	}
	return NULL;
}

Client *EntityList::FindAccount(uint32 account_id) const {
	//could likely improve this with a map, but its a little more work since
	//clients are added to the list before we know who they are.
	
	client_list::const_iterator cur, end;
	cur = m_clients.begin();
	end = m_clients.end();
	for(; cur != end; cur++) {
		if((*cur)->GetAccountID() == account_id)
			return(*cur);
	}
	return NULL;
}

void EntityList::FindByStationID(uint32 stationID, std::vector<Client *> &result) const {
	//this could likely be done better

	client_list::const_iterator cur, end;
	cur = m_clients.begin();
	end = m_clients.end();
	for(; cur != end; cur++) {
		if((*cur)->GetStationID() == stationID)
			result.push_back(*cur);
	}
}

void EntityList::FindByRegionID(uint32 regionID, std::vector<Client *> &result) const {
	//this could likely be done better

	client_list::const_iterator cur, end;
	cur = m_clients.begin();
	end = m_clients.end();
	for(; cur != end; cur++) {
		if((*cur)->GetRegionID() == regionID)
			result.push_back(*cur);
	}
}

void EntityList::Broadcast(const char *notifyType, const char *idType, PyTuple **payload) const {
	//build a little notification out of it.
	EVENotificationStream notify;
	notify.remoteObject = 1;
	notify.args = *payload;
	*payload = NULL;	//consumed

	//now sent it to the client
	PyAddress dest;
	dest.type = PyAddress::Broadcast;
	dest.service = notifyType;
	dest.bcast_idtype = idType;
	Broadcast(dest, notify);
}

void EntityList::Broadcast(const PyAddress &dest, EVENotificationStream &noti) const {
	client_list::const_iterator cur, end;
	cur = m_clients.begin();
	end = m_clients.end();
	for(; cur != end; cur++) {
		(*cur)->SendNotification(dest, noti);
	}
}

void EntityList::Multicast(const character_set &cset, const PyAddress &dest, EVENotificationStream &noti) const {
	//this could likely be done better

	std::vector<Client *> result;
	GetClients(cset, result);

	std::vector<Client *>::iterator cur, end;
	cur = result.begin();
	end = result.end();
	for(; cur != end; cur++) {
		(*cur)->SendNotification(dest, noti);
	}
}

//in theory this could be written in therms of the more generic
//MulticastTarget function, but this is much more efficient.
void EntityList::Multicast( const char* notifyType, const char* idType, PyTuple** payload, NotificationDestination target, uint32 target_id, bool seq )
{
    PyTuple* p = *payload;
    *payload = NULL;

	std::list<Client*>::const_iterator cur, end;
	cur = m_clients.begin();
	end = m_clients.end();
	for(; cur != end; cur++)
    {
		switch( target )
        {
		case NOTIF_DEST__LOCATION:
			if( (*cur)->GetLocationID() != target_id )
				continue;
			break;
		case NOTIF_DEST__CORPORATION:
			if( (*cur)->GetCorporationID() != target_id )
				continue;
			break;
		}

		PyTuple* temp = new PyTuple( *p );
		(*cur)->SendNotification( notifyType, idType, &temp, seq );
	}

    PyDecRef( p );
}

void EntityList::Multicast(const char *notifyType, const char *idType, PyTuple **in_payload, const MulticastTarget &mcset, bool seq)
{
	// consume payload
	PyTuple *payload = *in_payload;
	*in_payload = NULL;

	//cache all these locally to avoid calling empty all the time.
	const bool chars_empty = mcset.characters.empty();
	const bool locs_empty = mcset.locations.empty();
	const bool corps_empty = mcset.corporations.empty();

	if( !chars_empty || !locs_empty || !corps_empty )
	{
		std::list<Client *>::const_iterator cur, end;
		cur = m_clients.begin();
		end = m_clients.end();
		for(; cur != end; cur++)
		{
			if(	  !chars_empty
			     && mcset.characters.find((*cur)->GetCharacterID()) != mcset.characters.end() )
			{
				//found, carry on...
			}
			else if(   !locs_empty
					  && mcset.locations.find((*cur)->GetLocationID()) != mcset.locations.end() )
			{
				//found, carry on...
			}
			else if(   !corps_empty
					  && mcset.corporations.find((*cur)->GetCorporationID()) != mcset.corporations.end() )
			{
				//found, carry on...
			}
			else
			{
				//not found in any of the above sets.
				continue;
			}

			PyTuple *temp = new PyTuple( *payload );
			(*cur)->SendNotification( notifyType, idType, &temp, seq );
		}
	}

	PyDecRef( payload );
}

void EntityList::Multicast(const character_set &cset, const char *notifyType, const char *idType, PyTuple **in_payload, bool seq) const {
	std::vector<Client *> result;
	GetClients(cset, result);

	int num_remaining = result.size();
	
	std::vector<Client *>::iterator cur, end;
	cur = result.begin();
	end = result.end();
	PyTuple *payload;
	for(; cur != end; cur++, num_remaining--) {
		//keep a counter to eliminate an extra copy of in_payload
		if(num_remaining < 2) {
			payload = *in_payload;
			*in_payload = NULL;
		} else {
			if(*in_payload == NULL)
				payload = NULL;
			else
				payload = (PyTuple *) (*in_payload)->Clone();
		}
		
		(*cur)->SendNotification(notifyType, idType, &payload, seq);
	}
}

void EntityList::Unicast(uint32 charID, const char *notifyType, const char *idType, PyTuple **payload, bool seq) {
	//this could be implemented more efficiently, but I dont feel like it right now.
	character_set cset;
	cset.insert(charID);
	Multicast(cset, notifyType, idType, payload, seq);
}

void EntityList::GetClients(const character_set &cset, std::vector<Client *> &result) const {
	//this could likely be done better

	character_set::const_iterator res;
	client_list::const_iterator cur, end;
	cur = m_clients.begin();
	end = m_clients.end();
	for(; cur != end; cur++) {
		res = cset.find((*cur)->GetCharacterID());
		if(res != cset.end()) {
			result.push_back(*cur);
		}
	}
}

SystemManager *EntityList::FindOrBootSystem(uint32 systemID) {
	system_list::iterator res;
	res = m_systems.find(systemID);
	if(res != m_systems.end())
		return(res->second);

    sLog.Log("Entity List", "Booting system %u", systemID);
/*	
    ItemData idata(
        5,
        1,
		systemID,
		flagAutoFit,
        1
	);
*/
	SystemManager *mgr = new SystemManager(systemID, *m_services);//, idata);
	if(!mgr->BootSystem()) {
		delete mgr;
		return NULL;
	}
	
	m_systems[systemID] = mgr;
	return mgr;
}
