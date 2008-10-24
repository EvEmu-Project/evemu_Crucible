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

PyResult Command_summon(Client *who, CommandDB *db, PyServiceMgr *services, const Seperator &args) {
	if(!args.IsNumber(1)) {
		throw(PyException(MakeCustomError("Argument 1 must be type ID.")));
	}
	uint32 qty = 1;
	if(args.IsNumber(2))
		qty = atoi(args.arg[2]);
	_log(COMMAND__MESSAGE, "Summon %s %lu times", args.arg[1], qty);

	//summon into their cargo hold unless they are docked in a station,
	//then stick it in their hangar instead.
	EVEItemFlags flag = flagCargoHold;
	uint32 locationID = who->GetShipID();
	if(IsStation(who->GetLocationID())) {
		flag = flagHangar;
		locationID = who->GetStationID();
	}
	
	InventoryItem *i;
	i = services->item_factory->Spawn(
		atoi(args.arg[1]),
		qty,
		who->GetCharacterID(),
		0,	//temp location.
		flag
		);
	if(i == NULL)
		throw(PyException(MakeCustomError("Unable to spawn item of type %s.", args.arg[1])));

	//Move to location
	i->Move( locationID, flag, true );
	//we dont need our reference anymore...
	i->Release();

	return(new PyRepString("Summon successfull."));
}

PyResult Command_search(Client *who, CommandDB *db, PyServiceMgr *services, const Seperator &args) {
	//an empty query is a bad idea.
	if(args.argplus[1][0] == '\0')
		throw(PyException(MakeCustomError("Usage: /search [text]")));

	std::map<uint32, std::string> matches;
	if(!db->ItemSearch(args.argplus[1], matches))
		throw(PyException(MakeCustomError("Failed to query DB.")));

	std::string result;
	char buf[16];
	snprintf(buf, sizeof(buf), "%lu", matches.size());
	result = buf;
	result += " matches found.<br>";
	
	std::map<uint32, std::string>::iterator cur, end;
	cur = matches.begin();
	end = matches.end();
	for(; cur != end; cur++) {
		snprintf(buf, sizeof(buf), "%lu: ", cur->first);
		result += buf;
		result += cur->second;
		result += "<br>";
	}
	
	if(matches.size() > 10) {
		//send the results in an evemail.
		std::string subject("Search results for ");
		subject += args.argplus[1];
		who->SelfEveMail(subject.c_str(), result.c_str());
		return(new PyRepString("Results sent via evemail."));
	} else
		return(new PyRepString(result));
}

PyResult Command_translocate(Client *who, CommandDB *db, PyServiceMgr *services, const Seperator &args) {
	_log(COMMAND__MESSAGE, "Translocate to %s", args.arg[1]);
	if(!args.IsNumber(1))
		throw(PyException(MakeCustomError("Argument 1 should be an entity ID")));

	uint32 loc = atoi(args.arg[1]);
	
	GPoint p(0.0f, 1000000.0f, 0.0f);	//when going to a system, who knows where to stick them... could find a stargate and stick them near it I guess...
	
	if(!IsStation(loc) && !IsSolarSystem(loc)) {
		Client *tgt;
		tgt = services->entity_list->FindCharacter(loc);
		if(tgt == NULL)
			tgt = services->entity_list->FindByShip(loc);
		if(tgt == NULL)
			throw(PyException(MakeCustomError("Unable to find location %d", loc)));
		loc = tgt->GetLocationID();
		p = tgt->GetPosition();
	}

	who->MoveToLocation(loc, p);
	return(new PyRepString("Translocation successfull."));
}


PyResult Command_tr(Client *who, CommandDB *db, PyServiceMgr *services, const Seperator &args) {
	if(std::string("me") != args.arg[1])
		throw(PyException(MakeCustomError("Translocate (/TR) to non-me who '%s' is not supported yet.", args.arg[1])));
	
	if(!args.IsNumber(2))
		throw(PyException(MakeCustomError("Argument 1 should be an entity ID")));

	_log(COMMAND__MESSAGE, "Translocate to %s", args.arg[2]);

	uint32 loc = atoi(args.arg[2]);
	GPoint p(0.0f, 1000000.0f, 0.0f);	//when going to a system, who knows where to stick them... could find a stargate and stick them near it I guess...
	
	if(!IsStation(loc) && !IsSolarSystem(loc)) {
		Client *tgt;
		tgt = services->entity_list->FindCharacter(loc);
		if(tgt == NULL)
			tgt = services->entity_list->FindByShip(loc);
		if(tgt == NULL)
			throw(PyException(MakeCustomError("Unable to find location %d", loc)));
		loc = tgt->GetLocationID();
		p = tgt->GetPosition();
	}

	who->MoveToLocation(loc, p);
	return(new PyRepString("Translocation successfull."));
}

PyResult Command_giveisk(Client *who, CommandDB *db, PyServiceMgr *services, const Seperator &args) {
	if(!args.IsNumber(1))
		throw(PyException(MakeCustomError("Argument 1 should be an entity ID (0=self)")));

	if(!args.IsNumber(2))
		throw(PyException(MakeCustomError("Argument 2 should be an amount of ISK")));

	uint32 entity = atoi(args.arg[1]);
	double amount = strtod(args.arg[2], NULL);
	
	Client *tgt;
	if(entity == 0) {
		tgt = who;
	} else {
		tgt = services->entity_list->FindCharacter(entity);
		if(tgt == NULL)
			throw(PyException(MakeCustomError("Unable to find character %lu", entity)));
	}
	
	tgt->AddBalance(amount);
	return(new PyRepString("Operation successfull."));
}

PyResult Command_pop(Client *who, CommandDB *db, PyServiceMgr *services, const Seperator &args) {
	if(args.argnum != 3)
		throw(PyException(MakeCustomError("Bad arguments.")));

	//CustomNotify: notify
	//ServerMessage: msg
	//CustomError: error
	
	Notify_OnRemoteMessage n;
	n.msgType = args.arg[1];
	n.args[ args.arg[2] ] = new PyRepString(args.argplus[3]);
	PyRepTuple *tmp = n.FastEncode();
	
	who->SendNotification("OnRemoteMessage", "charid", &tmp);

	return(new PyRepString("Message sent."));
}

PyResult Command_goto(Client *who, CommandDB *db, PyServiceMgr *services, const Seperator &args) {
	if(args.argnum != 3 || !args.IsNumber(1) || !args.IsNumber(2) || !args.IsNumber(3))
		throw(PyException(MakeCustomError("Arguments must be 3 numbers")));

	GPoint p(	atof(args.arg[1]),
				atof(args.arg[2]),
				atof(args.arg[3]) );
	
	_log(COMMAND__MESSAGE, "%s: Goto (%.13f, %.13f, %.13f)", who->GetName(), p.x, p.y, p.z);
	
	who->MoveToPosition(p);

	return(new PyRepString("Goto successfull."));
}

PyResult Command_spawn(Client *who, CommandDB *db, PyServiceMgr *services, const Seperator &args) {
	_log(COMMAND__MESSAGE, "Spawn %s", args.arg[1]);
	if(!args.IsNumber(1))
		throw(PyException(MakeCustomError("Argument 1 should be an item type ID")));

	if(!who->IsInSpace())
		throw(PyException(MakeCustomError("You must be in space to spawn things.")));
	
	InventoryItem *i;
	i = services->item_factory->SpawnSingleton(
		atoi(args.arg[1]),
		who->GetCorporationID(),	//owner
		who->GetLocationID(),
		flagAutoFit
		);
	if(i == NULL)
		throw(PyException(MakeCustomError("Unable to spawn item of type %s.", args.arg[1])));

	//hackin it...
	GPoint loc(who->GetPosition());
	loc.x += 1500;

	SystemManager *sys = who->System();
	NPC *it = new NPC(sys, services, i, who->GetCorporationID(), who->GetAllianceID(), loc);
	sys->AddNPC(it);

	return(new PyRepString("Spawn successfull."));
}

PyResult Command_location(Client *who, CommandDB *db, PyServiceMgr *services, const Seperator &args) {
	if (!who->IsInSpace())
		throw(PyException(MakeCustomError("You're not in space")));

	char reply[64];
	GPoint loc(who->GetPosition());
	snprintf(reply, 64, "x: %lf, y: %lf, z: %lf", loc.x, loc.y, loc.z);

	return(new PyRepString(reply));
}

PyResult Command_clearlog(Client *who, CommandDB *db, PyServiceMgr *services, const Seperator &args) {
	memory_log.clear();

	return(new PyRepString("Memory log cleared."));
}

PyResult Command_getlog(Client *who, CommandDB *db, PyServiceMgr *services, const Seperator &args) {
	std::string evemail;

	std::list<std::string>::const_iterator cur, end;
	cur = memory_log.begin();
	end = memory_log.end();
	for(; cur != end; cur++) {
		evemail += *cur;
		evemail += "<br>";
	}

	who->SelfEveMail("Server logs", "Server log of length %lu/%lu records (%lu bytes):<br><br>%s", memory_log.size(), memory_log_limit, evemail.size(), evemail.c_str());

	return(new PyRepString("Log sent via evemail."));
}

// new command to modify blueprint's attributes, we have to give it blueprint's itemID ... isn't much comfortable, but I don't know about better solution
PyResult Command_setbpattr(Client *who, CommandDB *db, PyServiceMgr *services, const Seperator &args) {
	if(!args.IsNumber(1))
		throw(PyException(MakeCustomError("Argument 1 must be blueprint ID. (got %s)", args.arg[1])));

	if(*args.arg[2] != '0' && *args.arg[2] != '1')
		throw(PyException(MakeCustomError("Argument 2 must be 0 (not copy) or 1 (copy). (got %s)", args.arg[2])));

	if(!args.IsNumber(3))
		throw(PyException(MakeCustomError("Argument 3 must be material level. (got %s)", args.arg[3])));

	if(!args.IsNumber(4))
		throw(PyException(MakeCustomError("Argument 4 must be productivity level. (got %s)", args.arg[4])));

	if(!args.IsNumber(5))
		throw(PyException(MakeCustomError("Argument 5 must be remaining licensed production runs. (got %s)", args.arg[5])));


	BlueprintProperties bp = {
		atoi(args.arg[2]),
		atoi(args.arg[3]),
		atoi(args.arg[4]),
		atoi(args.arg[5])
	};

	if(!db->SetBlueprintProperties(atoi(args.arg[1]), bp))
		throw(PyException(MakeCustomError("Failed to change attributes.")));

	return(new PyRepString("Properties modified."));
}

PyResult Command_state(Client *who, CommandDB *db, PyServiceMgr *services, const Seperator &args) {
	if(!who->IsInSpace())
		throw(PyException(MakeCustomError("You must be in space.")));

	DestinyManager *destiny = who->Destiny();
	if(destiny == NULL)
		throw(PyException(MakeCustomError("You have no destiny manager.")));

	destiny->SendSetState(who->Bubble());

	return(new PyRepString("Update sent."));
}











