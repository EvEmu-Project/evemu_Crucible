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
	Author:		Zhur
*/
#include "EvemuPCH.h"


PyCallable_Make_InnerDispatcher(ReprocessingService)


class ReprocessingServiceBound
: public PyBoundObject {
public:
	ReprocessingServiceBound(PyServiceMgr *mgr, ReprocessingDB *db, uint32 stationID);
	virtual ~ReprocessingServiceBound();

	PyCallable_DECL_CALL(GetOptionsForItemTypes)
	PyCallable_DECL_CALL(GetReprocessingInfo)
	PyCallable_DECL_CALL(GetQuote)
	PyCallable_DECL_CALL(GetQuotes)
	PyCallable_DECL_CALL(Reprocess)

	bool Load();
	virtual void Release();

protected:
	class Dispatcher;
	Dispatcher *const m_dispatch;

	ReprocessingDB *const m_db;

	uint32 m_stationID;
	double m_staEfficiency;
	double m_tax;
	
	double _CalcReprocessingEfficiency(const Client *client, const InventoryItem *item = NULL) const;
	PyRep *_GetQuote(uint32 itemID, const Client *c) const;
};

PyCallable_Make_InnerDispatcher(ReprocessingServiceBound)

ReprocessingService::ReprocessingService(PyServiceMgr *mgr, DBcore *db)
: PyService(mgr, "reprocessingSvc"),
  m_dispatch(new Dispatcher(this)),
  m_db(db)
{
	_SetCallDispatcher(m_dispatch);
}

ReprocessingService::~ReprocessingService() {
	delete m_dispatch;
}

PyBoundObject *ReprocessingService::_CreateBoundObject(Client *c, const PyRep *bind_args) {
	if(!bind_args->IsInteger()) {
		codelog(CLIENT__ERROR, "%s: Non-integer bind argument '%s'", c->GetName(), bind_args->TypeString());
		return NULL;
	}

	const PyRepInteger * stationID = (const PyRepInteger *) bind_args;

	if(!IsStation(stationID->value)) {
		codelog(CLIENT__ERROR, "%s: Expected stationID, but hasn't.", c->GetName());
		return NULL;
	}

	ReprocessingServiceBound *obj = new ReprocessingServiceBound(m_manager, &m_db, stationID->value);
	if(!obj->Load()) {
		_log(SERVICE__ERROR, "Failed to load static info for station %lu.", stationID->value);
		delete obj;
		return NULL;
	} else
		return(obj);
}

//******************************************************************************

ReprocessingServiceBound::ReprocessingServiceBound(PyServiceMgr *mgr, ReprocessingDB *db, uint32 stationID)
: PyBoundObject(mgr),
  m_dispatch(new Dispatcher(this)),
  m_stationID(stationID),
  m_db(db),
  m_staEfficiency(0),
  m_tax(0)
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(ReprocessingServiceBound, GetOptionsForItemTypes)
	PyCallable_REG_CALL(ReprocessingServiceBound, GetReprocessingInfo)
	PyCallable_REG_CALL(ReprocessingServiceBound, GetQuote)
	PyCallable_REG_CALL(ReprocessingServiceBound, GetQuotes)
	PyCallable_REG_CALL(ReprocessingServiceBound, Reprocess)
}

ReprocessingServiceBound::~ReprocessingServiceBound() {
}

void ReprocessingServiceBound::Release() {
	//I hate this statement
	delete this;
}

bool ReprocessingServiceBound::Load() {
	return(m_db->LoadStatic(m_stationID, m_staEfficiency, m_tax));
}

PyResult ReprocessingServiceBound::Handle_GetOptionsForItemTypes(PyCallArgs &call) {
	PyRep *result = NULL;

	Call_GetOptionsForItemTypes call_args;
	if(!call_args.Decode(&call.tuple)) {
		_log(SERVICE__ERROR, "Unable to decode args.");
		return NULL;
	}

	Rsp_GetOptionsForItemTypes		rsp;
	Rsp_GetOptionsForItemTypes_Arg	arg;

	std::map<uint32, PyRep *>::iterator cur, end;
	cur = call_args.typeIDs.begin();
	end = call_args.typeIDs.end();

	for(; cur != end; cur++) {
		arg.isRecyclable = m_db->IsRecyclable(cur->first);
		arg.isRefinable = m_db->IsRefinable(cur->first);

		rsp.typeIDs[cur->first] = arg.Encode();
	}

	result = rsp.Encode();
	return(result);
}

PyResult ReprocessingServiceBound::Handle_GetReprocessingInfo(PyCallArgs &call) {
	PyRep *result = NULL;

	Rsp_GetReprocessingInfo rsp;

	rsp.tax = m_tax;
	rsp.reputation = 0.0;	// this should be drain from DB
	rsp.yield = m_staEfficiency;
	rsp.combinedyield = _CalcReprocessingEfficiency(call.client);

	result = rsp.Encode();

	return(result);
}

PyResult ReprocessingServiceBound::Handle_GetQuote(PyCallArgs &call) {
	PyRep *result = NULL;

	Call_SingleIntegerArg call_args;	// itemID
	if(!call_args.Decode(&call.tuple)) {
		_log(SERVICE__ERROR, "Unable to decode args.");
		return NULL;
	}

	return(_GetQuote(call_args.arg, call.client));
}

PyResult ReprocessingServiceBound::Handle_GetQuotes(PyCallArgs &call) {
	PyRep *result = NULL;

	Call_GetQuotes call_arg;
	if(!call_arg.Decode(&call.tuple)) {
		_log(SERVICE__ERROR, "Unable to decode args.");
		return NULL;
	}

	Rsp_GetQuotes rsp;
	std::vector<uint32>::iterator cur, end;
	cur = call_arg.itemIDs.begin();
	end = call_arg.itemIDs.end();

	for(; cur != end; cur++) {
		PyRep *quote = NULL;
		try {
			quote = _GetQuote(*cur, call.client);
		} catch(PyException &) {
			// ignore all exceptions
			continue;
		}
		if(quote != NULL)
			rsp.quotes[*cur] = quote;
	}

	result = rsp.Encode();

	return(result);
}

PyResult ReprocessingServiceBound::Handle_Reprocess(PyCallArgs &call) {
	if(!IsStation(call.client->GetLocationID())) {
		_log(SERVICE__MESSAGE, "Character %s tried to reprocess, but isn't is station.", call.client->GetName());
		return NULL;
	}

	Call_Reprocess call_args;

	if(!call_args.Decode(&call.tuple)) {
		_log(SERVICE__ERROR, "Failed to parse args.");
		return NULL;
	}

	if(call_args.ownerID == 0)
		call_args.ownerID = call.client->GetCharacterID();

	if(call_args.flag == 0)
		call_args.flag = flagHangar;

	std::vector<uint32>::iterator cur, end;
	cur = call_args.items.begin();
	end = call_args.items.end();
	for(; cur != end; cur++) {
		InventoryItem *item = m_manager->item_factory.Load(*cur, true);
		if(item == NULL)
			continue;

		if(item->ownerID() != call_args.ownerID) {
			_log(SERVICE__ERROR, "Character %lu tried to reprocess item %lu of character %lu. Skipping.", call.client->GetCharacterID(), item->itemID(), item->ownerID());
			continue;
		}

		// this should never happen, but for sure ...
		if(item->type()->portionSize > item->quantity()) {
			std::map<std::string, PyRep *> args;
			args["typename"] = new PyRepString(item->itemName().c_str());
			args["portion"] = new PyRepInteger(item->type()->portionSize);

			item->Release();
			throw(PyException(MakeUserError("QuantityLessThanMinimumPortion", args)));
		}

		double efficiency = _CalcReprocessingEfficiency(call.client, item);
		
		std::vector<Recoverable> recoverables;
		if(!m_db->GetRecoverables(item->typeID(), recoverables)) {
			item->Release();
			continue;
		}

		std::vector<Recoverable>::iterator cur_rec, end_rec;
		cur_rec = recoverables.begin();
		end_rec = recoverables.end();
		for(; cur_rec != end_rec; cur_rec++) {
			uint32 quantity = cur_rec->amountPerBatch * efficiency * (1.0 - m_tax) * item->quantity() / item->type()->portionSize;
			if(quantity == 0)
				continue;
			InventoryItem *i = m_manager->item_factory.Spawn(
				cur_rec->typeID,
				call.client->GetCharacterID(),
				0,
				flagHangar,
				quantity);
			if(i == NULL)
				continue;

			i->Move(call.client->GetStationID(), flagHangar);
			i->Release();
		}

		uint32 qtyLeft = item->quantity() % item->type()->portionSize;
		if(qtyLeft == 0)
			item->Delete();
		else {
			item->SetQuantity(qtyLeft);
			item->Release();
		}
	}

	return NULL;
}

double ReprocessingServiceBound::_CalcReprocessingEfficiency(const Client *c, const InventoryItem *item) const {
	std::set<EVEItemFlags> flags;
	flags.insert(flagSkill);
	flags.insert(flagSkillInTraining);

	std::vector<const InventoryItem *> skills;

	c->Char()->FindByFlagSet(flags, skills);

	// formula is: reprocessingEfficiency + 0.375*(1 + 0.02*RefiningSkill)*(1 + 0.04*RefineryEfficiencySkill)*(1 + 0.05*OreProcessingSkill)
	double efficiency =	0.375*(1 + 0.02*GetSkillLevel(skills, skillRefining))*
						(1 + 0.04*GetSkillLevel(skills, skillRefineryEfficiency));

	if(item != NULL) {
		uint32 specificSkill = item->reprocessingSkillType();
		if(specificSkill != 0)
			efficiency *= 1 + 0.05*GetSkillLevel(skills, specificSkill);
		else
			efficiency *= 1 + 0.05*GetSkillLevel(skills, skillScrapmetalProcessing);	// use Scrapmetal Processing as default
	}

	efficiency += m_staEfficiency;

	if(efficiency > 1)
		efficiency = 1.0;

	return(efficiency);
}

PyRep *ReprocessingServiceBound::_GetQuote(uint32 itemID, const Client *c) const {
	InventoryItem *item = m_manager->item_factory.Load(itemID, true);
	if(item == NULL)
		return NULL;	// No action as GetQuote is also called for reprocessed items (probably for check)

	if(item->ownerID() != c->GetCharacterID()) {
		_log(SERVICE__ERROR, "Character %lu tried to reprocess item %lu of character %lu.", c->GetCharacterID(), item->itemID(), item->ownerID());
		item->Release();
		return NULL;
	}

	if(item->quantity() < item->type()->portionSize) {
		std::map<std::string, PyRep *> args;
		args["typename"] = new PyRepString(item->itemName().c_str());
		args["portion"] = new PyRepInteger(item->type()->portionSize);

		item->Release();
		throw(PyException(MakeUserError("QuantityLessThanMinimumPortion", args)));
	}

	Rsp_GetQuote res;
	res.leftOvers = item->quantity() % item->type()->portionSize;
	res.quantityToProcess = item->quantity() - res.leftOvers;
	res.playerStanding = 0.0;	// hack

	if(item->quantity() >= item->type()->portionSize) {
		std::vector<Recoverable> recoverables;
		if(!m_db->GetRecoverables(item->typeID(), recoverables)) {
			item->Release();
			return NULL;
		}

		double efficiency = _CalcReprocessingEfficiency(c, item);

		std::vector<Recoverable>::const_iterator cur, end;
		cur = recoverables.begin();
		end = recoverables.end();

		for(; cur != end; cur++) {
			Rsp_GetQuote_Recoverables_Line line;

			line.typeID =			cur->typeID;
			uint32 ratio = cur->amountPerBatch * res.quantityToProcess / item->type()->portionSize;
			line.unrecoverable =	uint32((1.0 - efficiency)			* ratio);
			line.station =			uint32(efficiency * m_tax			* ratio);
			line.client =			uint32(efficiency * (1.0 - m_tax)	* ratio);

			res.lines.add(line.Encode());
		}
	}

	item->Release();
	return(res.Encode());
}

