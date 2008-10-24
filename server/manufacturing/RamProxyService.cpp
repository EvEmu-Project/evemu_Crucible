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

PyCallable_Make_InnerDispatcher(RamProxyService)

RamProxyService::RamProxyService(PyServiceMgr *mgr, DBcore *db)
: PyService(mgr, "ramProxy"),
  m_dispatch(new Dispatcher(this)),
  m_db(db)
{
	_SetCallDispatcher(m_dispatch);

	PyCallable_REG_CALL(RamProxyService, GetJobs2);
	PyCallable_REG_CALL(RamProxyService, AssemblyLinesSelect);
	PyCallable_REG_CALL(RamProxyService, AssemblyLinesGet);
	PyCallable_REG_CALL(RamProxyService, InstallJob);
	PyCallable_REG_CALL(RamProxyService, CompleteJob);
}

RamProxyService::~RamProxyService() {
	delete m_dispatch;
}

PyResult RamProxyService::Handle_GetJobs2(PyCallArgs &call) {
	Call_GetJobs2 args;
	if(!args.Decode(&call.tuple)) {
		_log(SERVICE__ERROR, "Failed to decode call args.");
		return(NULL);
	}

	if(args.ownerID == call.client->GetCorporationID())
		if((call.client->GetCorpInfo().corprole & corpRoleFactoryManager) != corpRoleFactoryManager) {
			// I'm afraid we don't have right error in our DB ...
			call.client->SendInfoModalMsg("You cannot view your corporation's jobs because you do not possess the role \"Factory Manager\".");
			return(NULL);
		}

	return(m_db.GetJobs2(args.ownerID, args.completed, args.fromDate, args.toDate));
}

PyResult RamProxyService::Handle_AssemblyLinesSelect(PyCallArgs &call) {
	Call_AssemblyLinesSelect args;

	if(!args.Decode(&call.tuple)) {
		_log(SERVICE__ERROR, "Unable to decode args.");
		return(NULL);
	}

	// unfinished
	if(args.filter == "region")
		return(m_db.AssemblyLinesSelectPublic(call.client->GetRegionID()));
	else if(args.filter == "char")
		return(m_db.AssemblyLinesSelectPersonal(call.client->GetCharacterID()));
	else if(args.filter == "corp")
		return(m_db.AssemblyLinesSelectCorporation(call.client->GetCorporationID()));
	else if(args.filter == "alliance") {
//		return(m_db.AssemblyLinesSelectAlliance(...));
		call.client->SendInfoModalMsg("Alliances are not implemented yet.");
		return(NULL);
	} else {
		_log(SERVICE__ERROR, "Unknown filter '%s'.", args.filter.c_str());
		return(NULL);
	}
}

PyResult RamProxyService::Handle_AssemblyLinesGet(PyCallArgs &call) {
	Call_SingleIntegerArg arg;	// containerID

	if(!arg.Decode(&call.tuple)) {
		_log(SERVICE__ERROR, "Unable to decode args.");
		return(NULL);
	}

	return(m_db.AssemblyLinesGet(arg.arg));
}

PyResult RamProxyService::Handle_InstallJob(PyCallArgs &call) {
	Call_InstallJob args;
	if(!args.Decode(&call.tuple)) {
		_log(SERVICE__ERROR, "Failed to decode args.");
		return(NULL);
	}

	// load installed item
	InventoryItem *installedItem = m_manager->item_factory->Load(args.installedItemID, false);
	if(installedItem == NULL)
		return(NULL);

	// if output flag not set, put it where it was
	if(args.flagOutput == flagAutoFit)
		args.flagOutput = installedItem->flag();

	// decode path to BOM location
	PathElement pathBomLocation;
	if(!pathBomLocation.Decode(&args.bomPath.items[0])) {
		_log(SERVICE__ERROR, "Failed to decode BOM location.");
		installedItem->Release();
		return(NULL);
	}

	// verify call
	try {
		_VerifyInstallJob_Call(args, installedItem, pathBomLocation, call.client);
	} catch(...) {
		installedItem->Release();
		throw;
	}

	// this calculates some useful multipliers ... Rsp_InstallJob is used as container ...
	Rsp_InstallJob rsp;
	if(!_Calculate(args, installedItem, call.client, rsp)) {
		installedItem->Release();
		return(NULL);
	}

	// I understand sent maxJobStartTime as a limit, so this checks whether it's in limit
	if(rsp.maxJobStartTime > ((PyRepInteger *)call.byname["maxJobStartTime"])->value) {
		installedItem->Release();
		throw(PyException(MakeUserError("RamCannotGuaranteeStartTime")));
	}

	// query required items for activity
	std::vector<RequiredItem> reqItems;
	if(!m_db.GetRequiredItems(installedItem->typeID(), (EVERamActivity)args.activityID, reqItems)) {
		installedItem->Release();
		return(NULL);
	}

	// if 'quoteOnly' is 1 -> send quote, if 0 -> install job
	if(((PyRepInteger *)call.byname["quoteOnly"])->value) {
		installedItem->Release();	// not needed anymore
		_EncodeBillOfMaterials(reqItems, rsp.materialMultiplier, rsp.charMaterialMultiplier, args.runs, rsp.bom);
		_EncodeMissingMaterials(reqItems, pathBomLocation, call.client, rsp.materialMultiplier, rsp.charMaterialMultiplier, args.runs, rsp.missingMaterials);
		return(rsp.Encode());
	} else {
		// verify install
		try {
			_VerifyInstallJob_Install(rsp, pathBomLocation, reqItems, args.runs, call.client);
		} catch(...) {
			installedItem->Release();
			throw;
		}

		// now we are sure we have all requirements, we can start it ...

		// if blueprint, load properties
		BlueprintProperties bp;
		if(installedItem->categoryID() == EVEDB::invCategories::Blueprint) {
			if(!m_db.GetBlueprintProperties(installedItem->itemID(), bp)) {
				installedItem->Release();
				return(NULL);
			}
		} else {
			bp.copy = false;
			bp.materialLevel = bp.productivityLevel = bp.licensedProductionRunsRemaining = 0;
		}

		// if manufacturing from BPC, decrease licensed runs
		if(args.activityID == ramActivityManufacturing && bp.copy) {
			bp.licensedProductionRunsRemaining -= args.runs;
			if(!m_db.SetBlueprintProperties(installedItem->itemID(), bp)) {
				installedItem->Release();
				return(NULL);
			}
		}

		// load location where are located all materials
		InventoryItem *bomLocation = m_manager->item_factory->Load(pathBomLocation.locationID, true);
		if(bomLocation == NULL) {
			installedItem->Release();
			return(NULL);
		}

		// calculate proper start time
		uint64 beginProductionTime = Win32TimeNow();
		if(beginProductionTime < rsp.maxJobStartTime)
			beginProductionTime = rsp.maxJobStartTime;

		// register our job
		if(!m_db.InstallJob(args.isCorpJob ? call.client->GetCorporationID() : call.client->GetCharacterID(), call.client->GetCharacterID(),
							args.installationAssemblyLineID, installedItem->itemID(), beginProductionTime, beginProductionTime + uint64(rsp.productionTime) * 10000000L,
							args.description.c_str(), args.runs, (EVEItemFlags)args.flagOutput, bomLocation->locationID(), args.licensedProductionRuns))
		{
			if(args.activityID == ramActivityManufacturing && bp.copy) {
				bp.licensedProductionRunsRemaining += args.runs;
				m_db.SetBlueprintProperties(installedItem->itemID(), bp);
			}

			bomLocation->Release();
			installedItem->Release();
			return(NULL);
		}

		// pay for assembly lines, move the blueprint away
		call.client->AddBalance(-rsp.cost);
		installedItem->ChangeFlag(flagFactoryBlueprint);
		installedItem->Release();	//  not needed anymore

		// query all items contained in "Bill of Materials" location
		std::vector<InventoryItem *> items;
		bomLocation->FindByFlag((EVEItemFlags)pathBomLocation.flag, items);
		bomLocation->Release();		// not needed anymore

		std::vector<RequiredItem>::iterator cur, end;
		cur = reqItems.begin();
		end = reqItems.end();

		for(; cur != end; cur++) {
			if(cur->isSkill)
				continue;		// not interested

			// calculate needed quantity
			uint32 qtyNeeded = ceil(cur->quantity * rsp.materialMultiplier * args.runs);
			if(cur->damagePerJob == 1.0)
				qtyNeeded = ceil(qtyNeeded * rsp.charMaterialMultiplier);	// skill multiplier is applied only on fully consumed materials

			std::vector<InventoryItem *>::iterator curi, endi;
			curi = items.begin();
			endi = items.end();

			// remove required materials
			for(; curi != endi; curi++) {
				if((*curi)->typeID() == cur->typeID && (*curi)->ownerID() == call.client->GetCharacterID()) {
					if(qtyNeeded >= (*curi)->quantity()) {
						InventoryItem *i = (*curi)->Ref();
						qtyNeeded -= i->quantity();
						i->Delete();
					} else {
						(*curi)->AlterQuantity(-(int32)qtyNeeded);
						break;	// we are done, stop searching
					}
				}
			}
		}

		return(NULL);
	}
}

PyResult RamProxyService::Handle_CompleteJob(PyCallArgs &call) {
	Call_CompleteJob args;

	if(!args.Decode(&call.tuple)) {
		_log(CLIENT__ERROR, "Failed to decode args.");
		return(NULL);
	}

	_VerifyCompleteJob(args, call.client);

	// hundreds of variables to allocate ... maybe we can make struct for GetJobProperties and InstallJob?
	uint32 installedItemID, ownerID, runs, licensedProductionRuns;
	EVEItemFlags outputFlag;
	EVERamActivity activity;
	if(!m_db.GetJobProperties(args.jobID, installedItemID, ownerID, outputFlag, runs, licensedProductionRuns, activity))
		return(NULL);

	// return blueprint
	InventoryItem *installedItem = m_manager->item_factory->Load(installedItemID, false);
	if(installedItem == NULL)
		return(NULL);
	installedItem->ChangeFlag(outputFlag);

	std::vector<RequiredItem> reqItems;
	if(!m_db.GetRequiredItems(installedItem->typeID(), activity, reqItems)) {
		installedItem->Release();
		return(NULL);
	}

	// return materials which weren't completely consumed
	std::vector<RequiredItem>::iterator cur, end;
	cur = reqItems.begin();
	end = reqItems.end();
	for(; cur != end; cur++) {
		if(!cur->isSkill && cur->damagePerJob != 1.0) {
			uint32 quantity = floor(cur->quantity * runs * (1.0 - cur->damagePerJob));
			if(quantity == 0)
				continue;
			InventoryItem *item = m_manager->item_factory->Spawn(cur->typeID, quantity, ownerID, 0, outputFlag);
			if(item == NULL) {
				installedItem->Release();
				return(NULL);
			}
			item->Move(args.containerID, outputFlag);
			item->Release();
		}
	}

	// if not cancelled, realize result of activity
	if(!args.cancel)
		switch(activity) {
			case ramActivityManufacturing: {
				uint32 productTypeID = m_db.GetBlueprintProduct(installedItem->typeID());
				if(productTypeID == NULL) {
					installedItem->Release();
					return(NULL);
				}
				uint32 portionSize = m_db.GetPortionSize(productTypeID);
				if(portionSize == NULL) {
					installedItem->Release();
					return(NULL);
				}
				InventoryItem *item = m_manager->item_factory->Spawn(productTypeID, portionSize * runs, ownerID, 0, outputFlag);
				if(item == NULL) {
					installedItem->Release();
					return(NULL);
				}
				item->Move(args.containerID, outputFlag);
				item->Release();
				break;
			}
			case ramActivityResearchingTimeProductivity:
			case ramActivityResearchingMaterialProductivity: {
				BlueprintProperties bp;
				if(!m_db.GetBlueprintProperties(installedItem->itemID(), bp)) {
					installedItem->Release();
					return(NULL);
				}

				if(activity == ramActivityResearchingTimeProductivity)
					bp.productivityLevel += runs;
				else if(activity == ramActivityResearchingMaterialProductivity)
					bp.materialLevel += runs;

				if(!m_db.SetBlueprintProperties(installedItem->itemID(), bp)) {
					installedItem->Release();
					return(NULL);
				}
				break;
			}
			case ramActivityCopying: {
				BlueprintProperties bp;
				if(!m_db.GetBlueprintProperties(installedItem->itemID(), bp)) {
					installedItem->Release();
					return(NULL);
				}
				bp.copy = true;
				bp.licensedProductionRunsRemaining = licensedProductionRuns;

				InventoryItem *copy;
				for(uint32 c = 0; c < runs; c++) {
					// I think blueprints are always singleton ... BTW I can't understand why SpawnSigleton requires more args than Spawn
					copy = m_manager->item_factory->SpawnSingleton(installedItem->typeID(), ownerID, 0, outputFlag, installedItem->itemName().c_str(), GPoint());
					if(copy == NULL)
						continue;
					m_db.SetBlueprintProperties(copy->itemID(), bp);	// when this fails, nothing we can do about it, blueprint is already spawned
					copy->Move(args.containerID, outputFlag);
					copy->Release();
				}
			}
		}

	installedItem->Release();

	// regardless on success of this, we will return NULL, so there's no condition here
	m_db.CompleteJob(args.jobID, args.cancel ? ramCompletedStatusAbort : ramCompletedStatusDelivered);

	return(NULL);
}

/*
	UNKNOWN/NOT IMPLEMENTED EXCEPTIONS:
	************************************
	RamRemoteInstalledItemImpounded				- impound of installedItem
	RamInstallJob_InstalledItemChanged			- some cache expiration??
	RamInstalledItemMustBeInInstallation		- where to use?
	RamStationIsNotConstructed					- station building not implemented
	RamAccessDeniedWrongAlliance				- alliances not implemented
*/

void RamProxyService::_VerifyInstallJob_Call(const Call_InstallJob &args, const InventoryItem *const installedItem, const PathElement &bomLocation, Client *const c) {
	// ACTIVITY CHECK
	// ***************

	// anything else is not supported
	if( args.activityID != ramActivityManufacturing &&
		args.activityID != ramActivityResearchingTimeProductivity &&
		args.activityID != ramActivityResearchingMaterialProductivity &&
		args.activityID != ramActivityCopying)
	{
		throw(PyException(MakeUserError("RamActivityInvalid")));
	}

	// enumerate activities again as there are activities which doesnt require blueprint
	if((args.activityID == ramActivityManufacturing ||
		args.activityID == ramActivityResearchingTimeProductivity ||
		args.activityID == ramActivityResearchingMaterialProductivity ||
		args.activityID == ramActivityCopying) &&
		installedItem->categoryID() != EVEDB::invCategories::Blueprint)
	{
		throw(PyException(MakeUserError("RamActivityRequiresABlueprint")));
	}

	// JOBS CHECK
	// ***********

	if(args.activityID == ramActivityManufacturing) {
		uint32 jobCount = m_db.CountManufacturingJobs(c->GetCharacterID());
		if(c->Item()->manufactureSlotLimit() <= jobCount) {
			std::map<std::string, PyRep *> exceptArgs;
			exceptArgs["current"] = new PyRepInteger(jobCount);
			exceptArgs["max"] = new PyRepInteger(c->Item()->manufactureSlotLimit());
			throw(PyException(MakeUserError("MaxFactorySlotUsageReached", exceptArgs)));
		}
	} else {
		uint32 jobCount = m_db.CountResearchJobs(c->GetCharacterID());
		if(c->Item()->maxLaborotorySlots() <= jobCount) {
			std::map<std::string, PyRep *> exceptArgs;
			exceptArgs["current"] = new PyRepInteger(jobCount);
			exceptArgs["max"] = new PyRepInteger(c->Item()->maxLaborotorySlots());
			throw(PyException(MakeUserError("MaxResearchFacilitySlotUsageReached", exceptArgs)));
		}
	}

	// INSTALLATION CHECK
	// *******************

	uint32 regionID = m_db.GetRegionOfContainer(args.installationContainerID);
	if(regionID == NULL)
		throw(PyException(MakeUserError("RamIsNotAnInstallation")));

	if(c->GetRegionID() != regionID)
		throw(PyException(MakeUserError("RamRangeLimitationRegion")));

	// RamStructureNotInSpace
	// RamStructureNotIsSolarsystem
	// RamRangeLimitation
	// RamRangeLimitationJumps
	// RamRangeLimitationJumpsNoSkill

	// ASSEMBLY LINE CHECK
	// *********************

	uint32 ownerID;
	double minCharSec, maxCharSec;
	EVERamRestrictionMask restrictionMask;
	EVERamActivity activity;

	// get properties
	if(!m_db.GetAssemblyLineVerifyProperties(args.installationAssemblyLineID, ownerID, minCharSec, maxCharSec, restrictionMask, activity))
		throw(PyException(MakeUserError("RamInstallationHasNoDefaultContent")));

	// check validity of activity
	if(activity < ramActivityManufacturing || activity > ramActivityInvention)
		throw(PyException(MakeUserError("RamAssemblyLineHasNoActivity")));

	// check security rating if required
	if((restrictionMask & ramRestrictBySecurity) == ramRestrictBySecurity) {
		if(minCharSec > c->GetChar().securityRating)
			throw(PyException(MakeUserError("RamAccessDeniedSecStatusTooLow")));

		if(maxCharSec < c->GetChar().securityRating)
			throw(PyException(MakeUserError("RamAccessDeniedSecStatusTooHigh")));

		// RamAccessDeniedCorpSecStatusTooHigh
		// RamAccessDeniedCorpSecStatusTooLow
	}

	// check standing if required
	if((restrictionMask & ramRestrictByStanding) == ramRestrictByStanding) {
		// RamAccessDeniedCorpStandingTooLow
		// RamAccessDeniedStandingTooLow
	}

	if((restrictionMask & ramRestrictByAlliance) == ramRestrictByAlliance) {
//		if(...)
			throw(PyException(MakeUserError("RamAccessDeniedWrongAlliance")));
	} else if((restrictionMask & ramRestrictByCorp) == ramRestrictByCorp) {
		if(ownerID != c->GetCorporationID())
			throw(PyException(MakeUserError("RamAccessDeniedWrongCorp")));
	}

	if(args.isCorpJob) {
		if((c->GetCorpInfo().corprole & corpRoleFactoryManager) != corpRoleFactoryManager)
			throw(PyException(MakeUserError("RamCannotInstallForCorpByRoleFactoryManager")));

		if(args.activityID == ramActivityManufacturing) {
			if((c->GetCorpInfo().corprole & corpRoleCanRentFactorySlot) != corpRoleCanRentFactorySlot)
				throw(PyException(MakeUserError("RamCannotInstallForCorpByRole")));
		} else {
			if((c->GetCorpInfo().corprole & corpRoleCanRentResearchSlot) != corpRoleCanRentResearchSlot)
				throw(PyException(MakeUserError("RamCannotInstallForCorpByRole")));
		}
	}

	// PRODUCT CHECK
	// **************

	uint32 productTypeID;
	switch(args.activityID) {
		case ramActivityManufacturing:
			productTypeID = m_db.GetBlueprintProduct(installedItem->typeID());
			if(productTypeID == NULL)
				throw(PyException(MakeUserError("RamNoKnownOutputType")));
			break;

		case ramActivityInvention:
			productTypeID = m_db.GetTech2Blueprint(installedItem->typeID());
			if(productTypeID == NULL)
				throw(PyException(MakeUserError("RamInventionNoOutput")));
			break;

		case ramActivityResearchingTimeProductivity:
		case ramActivityResearchingMaterialProductivity:
		case ramActivityCopying:
		case ramActivityDuplicating:
			productTypeID = installedItem->typeID();
			break;

		case ramActivityNone:
		case ramActivityResearchingTechnology:
		case ramActivityReverseEngineering:
		default:
			throw(PyException(MakeUserError("RamNoKnownOutputType")));
	}

	if(!m_db.IsProducableBy(args.installationAssemblyLineID, m_db.GetGroup(productTypeID)))
		throw(PyException(MakeUserError("RamBadEndProductForActivity")));

	// INSTALLED ITEM CHECK
	// *********************

	// ownership
	if(args.isCorpJob) {
		if(installedItem->ownerID() != c->GetCorporationID())
			throw(PyException(MakeUserError("RamCannotInstallItemForAnotherCorp")));
	} else {
		if(installedItem->ownerID() != c->GetCharacterID())
			throw(PyException(MakeUserError("RamCannotInstallItemForAnother")));
	}

	// corp hangar permission
	if( (installedItem->flag() == flagCorpSecurityAccessGroup2 && (c->GetCorpInfo().corprole & corpRoleHangarCanTake2) != corpRoleHangarCanTake2) ||
		(installedItem->flag() == flagCorpSecurityAccessGroup3 && (c->GetCorpInfo().corprole & corpRoleHangarCanTake3) != corpRoleHangarCanTake3) ||
		(installedItem->flag() == flagCorpSecurityAccessGroup4 && (c->GetCorpInfo().corprole & corpRoleHangarCanTake4) != corpRoleHangarCanTake4) ||
		(installedItem->flag() == flagCorpSecurityAccessGroup5 && (c->GetCorpInfo().corprole & corpRoleHangarCanTake5) != corpRoleHangarCanTake5) ||
		(installedItem->flag() == flagCorpSecurityAccessGroup6 && (c->GetCorpInfo().corprole & corpRoleHangarCanTake6) != corpRoleHangarCanTake6) ||
		(installedItem->flag() == flagCorpSecurityAccessGroup7 && (c->GetCorpInfo().corprole & corpRoleHangarCanTake7) != corpRoleHangarCanTake7))
			throw(PyException(MakeUserError("RamAccessDeniedToBOMHangar")));

	// special checks for blueprints
	BlueprintProperties bp;
	if(m_db.GetBlueprintProperties(installedItem->itemID(), bp)) {
		if(bp.copy &&
			(args.activityID == ramActivityResearchingTimeProductivity || args.activityID == ramActivityResearchingMaterialProductivity))
				throw(PyException(MakeUserError("RamCannotResearchABlueprintCopy")));

		if(bp.copy && args.activityID == ramActivityCopying)
			throw(PyException(MakeUserError("RamCannotCopyABlueprintCopy")));

		if(!bp.copy && args.activityID == ramActivityInvention)
			throw(PyException(MakeUserError("RamCannotInventABlueprintOriginal")));

		if(bp.licensedProductionRunsRemaining - args.runs < 0)
			throw(PyException(MakeUserError("RamTooManyProductionRuns")));
	}

	// large location check
	if(IsStation(args.installationContainerID)) {
		if(/*args.isCorpJob && */installedItem->flag() == flagCargoHold)
			throw(PyException(MakeUserError("RamCorpInstalledItemNotInCargo")));

		if(installedItem->locationID() != args.installationContainerID) {
			if(args.installationContainerID == c->GetLocationID()) {
				std::map<std::string, PyRep *> exceptArgs;
				exceptArgs["location"] = new PyRepString(m_db.GetStationName(args.installationContainerID));

				if(args.isCorpJob)
					throw(PyException(MakeUserError("RamCorpInstalledItemWrongLocation", exceptArgs)));
				else
					throw(PyException(MakeUserError("RamInstalledItemWrongLocation", exceptArgs)));
			} else
				throw(PyException(MakeUserError("RamRemoteInstalledItemNotInStation")));
		} else {
			if(args.isCorpJob) {
				if(installedItem->flag() < flagCorpSecurityAccessGroup2 || installedItem->flag() > flagCorpSecurityAccessGroup7) {
					if(args.installationContainerID == c->GetLocationID()) {
						std::map<std::string, PyRep *> exceptArgs;
						exceptArgs["location"] = new PyRepString(m_db.GetStationName(args.installationContainerID));

						throw(PyException(MakeUserError("RamCorpInstalledItemWrongLocation", exceptArgs)));
					} else
						throw(PyException(MakeUserError("RamRemoteInstalledItemNotInOffice")));
				}
			} else {
				if(installedItem->flag() != flagHangar) {
					if(args.installationInvLocationID == c->GetLocationID()) {
						std::map<std::string, PyRep *> exceptArgs;
						exceptArgs["location"] = new PyRepString(m_db.GetStationName(args.installationContainerID));

						throw(PyException(MakeUserError("RamInstalledItemWrongLocation", exceptArgs)));
					} else {
						throw(PyException(MakeUserError("RamRemoteInstalledItemInStationNotHangar")));
					}
				}
			}
		}
	} else {
		if(args.installationContainerID == c->GetShipID()) {
			if(c->Item()->flag() != flagPilot)
				throw(PyException(MakeUserError("RamAccessDeniedNotPilot")));

			if(installedItem->locationID() != args.installationContainerID)
				throw(PyException(MakeUserError("RamInstalledItemMustBeInShip")));
		} else {
			// here should be stuff around POS, but I dont certainly know how it should work, so ...
			// RamInstalledItemBadLocationStructure
			// RamInstalledItemInStructureNotInContainer
			// RamInstalledItemInStructureUnknownLocation
		}
	}

	// BOM LOCATION CHECK
	// *******************

	// corp hangar permission
	if( (bomLocation.flag == flagCorpSecurityAccessGroup2 && (c->GetCorpInfo().corprole & corpRoleHangarCanTake2) != corpRoleHangarCanTake2) ||
		(bomLocation.flag == flagCorpSecurityAccessGroup3 && (c->GetCorpInfo().corprole & corpRoleHangarCanTake3) != corpRoleHangarCanTake3) ||
		(bomLocation.flag == flagCorpSecurityAccessGroup4 && (c->GetCorpInfo().corprole & corpRoleHangarCanTake4) != corpRoleHangarCanTake4) ||
		(bomLocation.flag == flagCorpSecurityAccessGroup5 && (c->GetCorpInfo().corprole & corpRoleHangarCanTake5) != corpRoleHangarCanTake5) ||
		(bomLocation.flag == flagCorpSecurityAccessGroup6 && (c->GetCorpInfo().corprole & corpRoleHangarCanTake6) != corpRoleHangarCanTake6) ||
		(bomLocation.flag == flagCorpSecurityAccessGroup7 && (c->GetCorpInfo().corprole & corpRoleHangarCanTake7) != corpRoleHangarCanTake7))
			throw(PyException(MakeUserError("RamAccessDeniedToBOMHangar")));
}

void RamProxyService::_VerifyInstallJob_Install(const Rsp_InstallJob &rsp, const PathElement &pathBomLocation, const std::vector<RequiredItem> &reqItems, const uint32 runs, Client *const c) {
	// MONEY CHECK
	// ************
	if(rsp.cost > c->GetBalance()) {
		std::map<std::string, PyRep *> args;
		args["amount"] = new PyRepReal(rsp.cost);
		args["balance"] = new PyRepReal(c->GetBalance());

		throw(PyException(MakeUserError("NotEnoughMoney", args)));
	}

	// PRODUCTION TIME CHECK
	// **********************
	if(rsp.productionTime > ramProductionTimeLimit) {
		std::map<std::string, PyRep *> args;
		args["productionTime"] = new PyRepInteger(rsp.productionTime);
		args["limit"] = new PyRepInteger(ramProductionTimeLimit);

		throw(PyException(MakeUserError("RamProductionTimeExceedsLimits")));
	}

	// SKILLS & ITEMS CHECK
	// *********************
	std::vector<const InventoryItem *> skills, items;

	// get skills ...
	std::set<EVEItemFlags> flags;
	flags.insert(flagSkill);
	flags.insert(flagSkillInTraining);
	c->Item()->FindByFlagSet(flags, skills);

	// ... and items
	InventoryItem *bomLocation = m_manager->item_factory->Load(pathBomLocation.locationID, true);
	if(bomLocation == NULL)
		throw(PyException(NULL));
	bomLocation->FindByFlag((EVEItemFlags)pathBomLocation.flag, items);
	bomLocation->Release();	// not needed anymore

	std::vector<RequiredItem>::const_iterator cur, end;
	cur = reqItems.begin();
	end = reqItems.end();
	for(; cur != end; cur++) {
		// check skill (quantity is required level)
		if(cur->isSkill) {
			if(GetSkillLevel(skills, cur->typeID) < cur->quantity) {
				std::map<std::string, PyRep *> args;
				args["item"] = new PyRepString(m_db.GetTypeName(cur->typeID));
				args["skillLevel"] = new PyRepInteger(cur->quantity);

				throw(PyException(MakeUserError("RamNeedSkillForJob", args)));
			}
		} else {
			// check materials

			// calculate needed quantity
			uint32 qtyNeeded = ceil(cur->quantity * rsp.materialMultiplier * runs);
			if(cur->damagePerJob == 1.0)
				qtyNeeded = ceil(qtyNeeded * rsp.charMaterialMultiplier);	// skill multiplier is applied only on fully consumed materials

			std::vector<const InventoryItem *>::iterator curi, endi;
			curi = items.begin();
			endi = items.end();

			for(; curi != endi && qtyNeeded > 0; curi++)
				if((*curi)->typeID() == cur->typeID && (*curi)->ownerID() == c->GetCharacterID())
					qtyNeeded = (*curi)->quantity() > qtyNeeded ? 0 : qtyNeeded - (*curi)->quantity();

			if(qtyNeeded > 0) {
				std::map<std::string, PyRep *> args;
				args["item"] = new PyRepString(m_db.GetTypeName(cur->typeID));

				throw(PyException(MakeUserError("RamNeedMoreForJob", args)));
			}
		}
	}
}

void RamProxyService::_VerifyCompleteJob(const Call_CompleteJob &args, Client *const c) {
	if(args.containerID == c->GetShipID())
		if(c->GetLocationID() != args.containerID || c->Item()->flag() != flagPilot)
			throw(PyException(MakeUserError("RamCompletionMustBeInShip")));

	uint32 ownerID;
	uint64 endProductionTime;
	EVERamCompletedStatus status;
	EVERamRestrictionMask restrictionMask;
	if(!m_db.GetJobVerifyProperties(args.jobID, ownerID, endProductionTime, restrictionMask, status))
		throw(PyException(MakeUserError("RamCompletionNoSuchJob")));

	if(ownerID != c->GetCharacterID()) {
		if(ownerID == c->GetCorporationID()) {
			if((c->GetCorpInfo().corprole & corpRoleFactoryManager) != corpRoleFactoryManager)
				throw(PyException(MakeUserError("RamCompletionAccessDeniedByCorpRole")));
		} else	// alliances not implemented
			throw(PyException(MakeUserError("RamCompletionAccessDenied")));
	}

	if(status != 0)
		throw(PyException(MakeUserError("RamCompletionJobCompleted")));

	if(!args.cancel && endProductionTime > Win32TimeNow())
		throw(PyException(MakeUserError("RamCompletionInProduction")));
}

bool RamProxyService::_Calculate(const Call_InstallJob &args, const InventoryItem *const installedItem, Client *const c, Rsp_InstallJob &into) {
	if(!m_db.GetAssemblyLineProperties(args.installationAssemblyLineID, into.materialMultiplier, into.timeMultiplier, into.installCost, into.usageCost))
		return(false);

	into.productionTime = m_db.GetBlueprintProductionTime(installedItem->typeID(), (EVERamActivity)args.activityID);
	if(!into.productionTime)
		return(false);

	uint32 productGroupID = installedItem->groupID();	//default
	// perform some activity-specific actions
	switch(args.activityID) {
		case ramActivityManufacturing: {
			uint32 productTypeID = m_db.GetBlueprintProduct(installedItem->typeID());
			productGroupID = m_db.GetGroup(productTypeID);

			double wasteFactor;
			uint32 materialLevel, productivityLevel, productivityModifier;
			if(!m_db.GetAdditionalBlueprintProperties(installedItem->itemID(), materialLevel, wasteFactor, productivityLevel, productivityModifier))
				return(false);

			into.materialMultiplier *= 1.0 + (wasteFactor / (1 + materialLevel));
			into.timeMultiplier *= (into.productionTime - (1.0 - (1.0 / (1 + productivityLevel))) * productivityModifier) / into.productionTime;

			into.charMaterialMultiplier = c->Item()->manufactureCostMultiplier();
			into.charTimeMultiplier = c->Item()->manufactureTimeMultiplier();

			switch(m_db.GetRace(productTypeID)) {
				case raceCaldari:	into.charTimeMultiplier *= double(c->Item()->caldariTechTimePercent()) / 100.0; break;
				case raceMinmatar:	into.charTimeMultiplier *= double(c->Item()->minmatarTechTimePercent()) / 100.0; break;
				case raceAmarr:		into.charTimeMultiplier *= double(c->Item()->amarrTechTimePercent()) / 100.0; break;
				case raceGallente:	into.charTimeMultiplier *= double(c->Item()->gallenteTechTimePercent()) / 100.0; break;
			}
			break;
		}
		case ramActivityResearchingTimeProductivity: {
			into.charMaterialMultiplier = double(c->Item()->researchCostPercent()) / 100.0;
			into.charTimeMultiplier = c->Item()->manufacturingTimeResearchSpeed();
			break;
		}
		case ramActivityResearchingMaterialProductivity: {
			into.charMaterialMultiplier = double(c->Item()->researchCostPercent()) / 100.0;
			into.charTimeMultiplier = c->Item()->mineralNeedResearchSpeed();
			break;
		}
		case ramActivityCopying: {
			uint32 maxProductionLimit = m_db.GetMaxProductionLimit(installedItem->typeID());
			if(!maxProductionLimit)
				return(false);

			// no ceil() here on purpose
			into.productionTime = (into.productionTime / maxProductionLimit) * args.licensedProductionRuns;
			
			into.charMaterialMultiplier = double(c->Item()->researchCostPercent()) / 100.0;
			into.charTimeMultiplier = c->Item()->copySpeedPercent();
			break;
		}
		default: {
			into.charMaterialMultiplier = 1.0;
			into.charTimeMultiplier = 1.0;
			break;
		}
	}

	if(productGroupID != NULL)
		if(!m_db.MultiplyMultipliers(args.installationAssemblyLineID, productGroupID, into.materialMultiplier, into.timeMultiplier))
			return(false);

	// calculate the remaining things
	into.productionTime *= into.timeMultiplier * into.charTimeMultiplier * args.runs;
	into.usageCost *= ceil(into.productionTime / 3600.0);
	into.cost = into.installCost + into.usageCost;

	// I "hope" this is right, simple tells client how soon will his job be started
	// Unfortunately, rounding done on client's side causes showing "Start time: 0 seconds" when he has to wait less than minute
	// I have no idea how to avoid this ...
	into.maxJobStartTime = m_db.GetNextFreeTime(args.installationAssemblyLineID);

	return(true);
}

void RamProxyService::_EncodeBillOfMaterials(const std::vector<RequiredItem> &reqItems, double materialMultiplier, double charMaterialMultiplier, uint32 runs, BillOfMaterials &into) {
	std::vector<RequiredItem>::const_iterator cur, end;
	cur = reqItems.begin();
	end = reqItems.end();
	for(; cur != end; cur++) {
		// if it's skill, insert it into special dict for skills
		if(cur->isSkill) {
			into.skills[cur->typeID] = new PyRepInteger(cur->quantity);
			continue;
		}

		// otherwise, make line for material list
		MaterialList_Line line;
		line.requiredTypeID = cur->typeID;
		line.quantity = ceil(cur->quantity * materialMultiplier * runs);
		line.damagePerJob = cur->damagePerJob;
		line.isSkillCheck = false;	// no idea what is this for
		line.requiresHP = false;	// no idea what is this for

		// and this is thing I'm not sure about ... if I understood it well, "Extra material" is everything not fully consumed,
		// "Raw material" is everything fully consumed and "Waste Material" is amount of material wasted ...
		if(line.damagePerJob < 1.0) {
			into.extras.lines.add(line.Encode());
		} else {
			// if there are losses, make line for waste material list
			if(charMaterialMultiplier > 1.0) {
				MaterialList_Line wastage;
				wastage.CloneFrom(&line);		// simply copy origial line ...
				wastage.quantity = ceil(wastage.quantity * (charMaterialMultiplier - 1.0));	// ... and calculate proper quantity

				into.wasteMaterials.lines.add(wastage.Encode());
			}
			into.rawMaterials.lines.add(line.Encode());
		}
	}
}

void RamProxyService::_EncodeMissingMaterials(const std::vector<RequiredItem> &reqItems, const PathElement &bomLocation, Client *const c, double materialMultiplier, double charMaterialMultiplier, uint32 runs, std::map<uint32, PyRep *> &into) {
	//query out what we need
	std::vector<const InventoryItem *> skills, items;

	//get the skills
	c->Item()->FindByFlag(flagSkill, skills);
	c->Item()->FindByFlag(flagSkillInTraining, skills);

	//get the items
	InventoryItem *bomContainer = m_manager->item_factory->Load(bomLocation.locationID, true);
	if(bomContainer == NULL)
		return;
	bomContainer->FindByFlag(EVEItemFlags(bomLocation.flag), items);
	bomContainer->Release();

	//now do the check
	std::vector<RequiredItem>::const_iterator cur, end;
	cur = reqItems.begin();
	end = reqItems.end();
	for(; cur != end; cur++) {
		uint32 qtyReq = cur->quantity;
		if(!cur->isSkill) {
			qtyReq = ceil(qtyReq * materialMultiplier * runs);
			if(cur->damagePerJob == 1.0)
				qtyReq = ceil(qtyReq * charMaterialMultiplier);
		}
		std::vector<const InventoryItem *>::const_iterator curi, endi;
		curi = cur->isSkill ? skills.begin() : items.begin();
		endi = cur->isSkill ? skills.end() : items.end();
		for(; curi != endi && qtyReq > 0; curi++) {
			if((*curi)->typeID() == cur->typeID && (*curi)->ownerID() == c->GetCharacterID()) {
				if(cur->isSkill)
					qtyReq -= std::min((uint32)qtyReq, (uint32)(*curi)->skillLevel());
				else
					qtyReq -= std::min((uint32)qtyReq, (uint32)(*curi)->quantity());
			}
		}
		if(qtyReq > 0)
			into[cur->typeID] = new PyRepInteger(qtyReq);
	}
}

