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
#include "EVEServerPCH.h"

PyCallable_Make_InnerDispatcher(RamProxyService)

RamProxyService::RamProxyService(PyServiceMgr *mgr)
: PyService(mgr, "ramProxy"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(RamProxyService, GetJobs2);
    PyCallable_REG_CALL(RamProxyService, AssemblyLinesSelect);
    PyCallable_REG_CALL(RamProxyService, AssemblyLinesGet);
    PyCallable_REG_CALL(RamProxyService, InstallJob);
    PyCallable_REG_CALL(RamProxyService, CompleteJob);
	PyCallable_REG_CALL(RamProxyService, GetRelevantCharSkills);
	PyCallable_REG_CALL(RamProxyService, AssemblyLinesSelectPublic);
}

RamProxyService::~RamProxyService() {
    delete m_dispatch;
}

PyResult RamProxyService::Handle_AssemblyLinesSelectPublic(PyCallArgs &call) {

	sLog.Debug("Server", "Called AsemblyLinesSelectPublic Stub.");

	return new PyList;
}

PyResult RamProxyService::Handle_GetRelevantCharSkills(PyCallArgs &call) {

	sLog.Debug("Server", "Called GetRelevantCharSkills Stub.");

	return NULL;
}

PyResult RamProxyService::Handle_GetJobs2(PyCallArgs &call) {
    Call_GetJobs2 args;
    if(!args.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "Failed to decode call args.");
        return NULL;
    }

    if((uint32)args.ownerID == call.client->GetCorporationID()) {
        if((call.client->GetCorpRole() & corpRoleFactoryManager) != corpRoleFactoryManager) {
            // I'm afraid we don't have proper error in our DB ...
            call.client->SendInfoModalMsg("You cannot view your corporation's jobs because you do not possess the role \"Factory Manager\".");
            return NULL;
        }
    }

    return(m_db.GetJobs2(args.ownerID, args.completed, args.fromDate, args.toDate));
}

PyResult RamProxyService::Handle_AssemblyLinesSelect(PyCallArgs &call) {
    Call_AssemblyLinesSelect args;

    if(!args.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "Unable to decode args.");
        return NULL;
    }

    // unfinished
    if(args.filter == "region")
        return(m_db.AssemblyLinesSelectPublic(call.client->GetRegionID()));
    else if(args.filter == "char")
        return(m_db.AssemblyLinesSelectPersonal(call.client->GetCharacterID()));
    else if(args.filter == "corp")
        return(m_db.AssemblyLinesSelectCorporation(call.client->GetCorporationID()));
    else if(args.filter == "alliance") {
//      return(m_db.AssemblyLinesSelectAlliance(...));
        call.client->SendInfoModalMsg("Alliances are not implemented yet.");
        return NULL;
    } else {
        _log(SERVICE__ERROR, "Unknown filter '%s'.", args.filter.c_str());
        return NULL;
    }
}

PyResult RamProxyService::Handle_AssemblyLinesGet(PyCallArgs &call) {
    Call_SingleIntegerArg arg;  // containerID

    if(!arg.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "Unable to decode args.");
        return NULL;
    }

    return(m_db.AssemblyLinesGet(arg.arg));
}

PyResult RamProxyService::Handle_InstallJob(PyCallArgs &call) {
    Call_InstallJob args;
    if(!args.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "Failed to decode args.");
        return NULL;
    }

    // load installed item
    InventoryItemRef installedItem = m_manager->item_factory.GetItem( args.installedItemID );
    if( !installedItem )
        return NULL;

    // if output flag not set, put it where it was
    if(args.flagOutput == flagAutoFit)
        args.flagOutput = installedItem->flag();

    // decode path to BOM location
    PathElement pathBomLocation;
    if( !pathBomLocation.Decode( args.bomPath->GetItem(0) ) ) {
        _log(SERVICE__ERROR, "Failed to decode BOM location.");
        return NULL;
    }

    // verify call
    _VerifyInstallJob_Call( args, (InventoryItemRef)installedItem, pathBomLocation, call.client );

    // this calculates some useful multipliers ... Rsp_InstallJob is used as container ...
    Rsp_InstallJob rsp;
    if(!_Calculate(args, (InventoryItemRef)installedItem, call.client, rsp))
        return NULL;

    // I understand sent maxJobStartTime as a limit, so this checks whether it's in limit
    if(rsp.maxJobStartTime > call.byname["maxJobStartTime"]->AsInt()->value())
        throw(PyException(MakeUserError("RamCannotGuaranteeStartTime")));

    // query required items for activity
    std::vector<RequiredItem> reqItems;
    if(!m_db.GetRequiredItems(installedItem->typeID(), (EVERamActivity)args.activityID, reqItems))
        return NULL;

    // if 'quoteOnly' is 1 -> send quote, if 0 -> install job
    if(call.byname["quoteOnly"]->AsInt()->value())
    {
        _EncodeBillOfMaterials(reqItems, rsp.materialMultiplier, rsp.charMaterialMultiplier, args.runs, rsp.bom);
        _EncodeMissingMaterials(reqItems, pathBomLocation, call.client, rsp.materialMultiplier, rsp.charMaterialMultiplier, args.runs, rsp.missingMaterials);

        return rsp.Encode();
    }
    else
    {
        // verify install
        _VerifyInstallJob_Install(rsp, pathBomLocation, reqItems, args.runs, call.client);

        // now we are sure everything from the client side is right, we can start it ...

        // calculate proper start time
        uint64 beginProductionTime = Win32TimeNow();
        if(beginProductionTime < (uint32)rsp.maxJobStartTime)
            beginProductionTime = rsp.maxJobStartTime;

        // register our job
        if( !m_db.InstallJob(
            args.isCorpJob ? call.client->GetCorporationID() : call.client->GetCharacterID(),
            call.client->GetCharacterID(),
            args.installationAssemblyLineID,
            installedItem->itemID(),
            beginProductionTime,
            beginProductionTime + uint64(rsp.productionTime) * Win32Time_Second,
            args.description.c_str(),
            args.runs,
            (EVEItemFlags)args.flagOutput,
            pathBomLocation.locationID,
            args.licensedProductionRuns ) )
        {
            return NULL;
        }

        // do some activity-specific actions
        switch(args.activityID) {
            case ramActivityManufacturing: {
                // decrease licensed production runs
                BlueprintRef bp = BlueprintRef::StaticCast( installedItem );
                if(!bp->infinite())
                    bp->AlterLicensedProductionRunsRemaining(-1);
            }
        }

        // pay for assembly lines, move the item away
        call.client->AddBalance(-rsp.cost);
        installedItem->Move( installedItem->locationID(), flagFactoryBlueprint );

        // query all items contained in "Bill of Materials" location
        std::vector<InventoryItemRef> items;
        _GetBOMItems( pathBomLocation, items );

        std::vector<RequiredItem>::iterator cur, end;
        cur = reqItems.begin();
        end = reqItems.end();
        for(; cur != end; cur++) {
            if(cur->isSkill)
                continue;       // not interested

            // calculate needed quantity
            uint32 qtyNeeded = static_cast<uint32>(ceil(cur->quantity * rsp.materialMultiplier * args.runs));
            if(cur->damagePerJob == 1.0)
                qtyNeeded = static_cast<uint32>(ceil(qtyNeeded * rsp.charMaterialMultiplier));   // skill multiplier is applied only on fully consumed materials

            std::vector<InventoryItemRef>::iterator curi, endi;
            curi = items.begin();
            endi = items.end();
            // consume required materials
            for(; curi != endi; curi++) {
                if((*curi)->typeID() == cur->typeID && (*curi)->ownerID() == call.client->GetCharacterID()) {
                    if(qtyNeeded >= (*curi)->quantity()) {
                        qtyNeeded -= (*curi)->quantity();
                        (*curi)->Delete();
                    } else {
                        (*curi)->AlterQuantity(-(int32)qtyNeeded);
                        break;  // we are done, stop searching
                    }
                }
            }
        }

        return NULL;
    }
}

PyResult RamProxyService::Handle_CompleteJob(PyCallArgs &call) {
    Call_CompleteJob args;

    if(!args.Decode(&call.tuple)) {
        _log(CLIENT__ERROR, "Failed to decode args.");
        return NULL;
    }

    _VerifyCompleteJob(args, call.client);

    // hundreds of variables to allocate ... maybe we can make struct for GetJobProperties and InstallJob?
    uint32 installedItemID, ownerID, runs, licensedProductionRuns;
    EVEItemFlags outputFlag;
    EVERamActivity activity;
    if(!m_db.GetJobProperties(args.jobID, installedItemID, ownerID, outputFlag, runs, licensedProductionRuns, activity))
        return NULL;

    // return item
    InventoryItemRef installedItem = m_manager->item_factory.GetItem( installedItemID );
    if( !installedItem )
        return NULL;
    installedItem->Move( installedItem->locationID(), outputFlag );

    std::vector<RequiredItem> reqItems;
    if( !m_db.GetRequiredItems( installedItem->typeID(), activity, reqItems ) )
        return NULL;

    // return materials which weren't completely consumed
    std::vector<RequiredItem>::iterator cur, end;
    cur = reqItems.begin();
    end = reqItems.end();
    for(; cur != end; cur++) {
        if(!cur->isSkill && cur->damagePerJob != 1.0) {
            uint32 quantity = static_cast<uint32>(cur->quantity * runs * (1.0 - cur->damagePerJob));
            if(quantity == 0)
                continue;

            ItemData idata(
                cur->typeID,
                ownerID,
                0, //temp location
                outputFlag,
                quantity
            );

            InventoryItemRef item = m_manager->item_factory.SpawnItem( idata );
            if( !item )
                return NULL;

            item->Move(args.containerID, outputFlag);
        }
    }

    // if not cancelled, realize result of activity
    if(!args.cancel) {
        switch(activity) {
            /*
             * Manufacturing
             */
            case ramActivityManufacturing: {
                BlueprintRef bp = BlueprintRef::StaticCast( installedItem );

                ItemData idata(
                    bp->productTypeID(),
                    ownerID,
                    0,  // temp location
                    outputFlag,
                    bp->productType().portionSize() * runs
                );

                InventoryItemRef item = m_manager->item_factory.SpawnItem( idata );
                if( !item )
                    return NULL;

                item->Move(args.containerID, outputFlag);
            } break;
            /*
             * Time productivity research
             */
            case ramActivityResearchingTimeProductivity: {
                BlueprintRef bp = BlueprintRef::StaticCast( installedItem );

                bp->AlterProductivityLevel( runs );
            } break;
            /*
             * Material productivity research
             */
            case ramActivityResearchingMaterialProductivity: {
                BlueprintRef bp = BlueprintRef::StaticCast( installedItem );

                bp->AlterMaterialLevel( runs) ;
            } break;
            /*
             * Copying
             */
            case ramActivityCopying: {
                BlueprintRef bp = BlueprintRef::StaticCast( installedItem );

                ItemData idata(
                    installedItem->typeID(),
                    ownerID,
                    0, //temp location
                    outputFlag,
                    runs
                );
                BlueprintData bdata(
                    true,
                    bp->materialLevel(),
                    bp->productivityLevel(),
                    licensedProductionRuns
                );

                BlueprintRef copy = m_manager->item_factory.SpawnBlueprint( idata, bdata );
                if( !copy )
                    return NULL;

                copy->Move(args.containerID, outputFlag);
            } break;
            /*
             * The rest is unsupported
             */
            case ramActivityResearchingTechnology:
            case ramActivityDuplicating:
            case ramActivityReverseEngineering:
            case ramActivityInvention:
            default: {
                _log(SERVICE__ERROR, "Activity %u is currently unsupported.", activity);
            } break;
        }
    }

    // regardless on success of this, we will return NULL, so there's no condition here
    m_db.CompleteJob(args.jobID, args.cancel ? ramCompletedStatusAbort : ramCompletedStatusDelivered);

    return NULL;
}

/*
    UNKNOWN/NOT IMPLEMENTED EXCEPTIONS:
    ************************************
    RamRemoteInstalledItemImpounded             - impound of installedItem
    RamInstallJob_InstalledItemChanged          - some cache expiration??
    RamInstalledItemMustBeInInstallation        - where to use?
    RamStationIsNotConstructed                  - station building not implemented
    RamAccessDeniedWrongAlliance                - alliances not implemented
*/

void RamProxyService::_VerifyInstallJob_Call(const Call_InstallJob &args, InventoryItemRef installedItem, const PathElement &bomLocation, Client *const c) {
    // ACTIVITY CHECK
    // ***************

    const ItemType *productType;
    switch(args.activityID) {
        /*
         * Manufacturing
         */
        case ramActivityManufacturing: {
            if(installedItem->categoryID() != EVEDB::invCategories::Blueprint)
                throw(PyException(MakeUserError("RamActivityRequiresABlueprint")));

            BlueprintRef bp = BlueprintRef::StaticCast( installedItem );

            if(!bp->infinite() && (bp->licensedProductionRunsRemaining() - args.runs) < 0)
                throw(PyException(MakeUserError("RamTooManyProductionRuns")));

            productType = &bp->productType();
            break;
        }
        /*
         * Time/Material Research
         */
        case ramActivityResearchingMaterialProductivity:
        case ramActivityResearchingTimeProductivity: {
            if(installedItem->categoryID() != EVEDB::invCategories::Blueprint)
                throw(PyException(MakeUserError("RamActivityRequiresABlueprint")));

            BlueprintRef bp = BlueprintRef::StaticCast( installedItem );

            if(bp->copy())
                throw(PyException(MakeUserError("RamCannotResearchABlueprintCopy")));

            productType = &bp->type();
            break;
        }
        /*
         * Copying
         */
        case ramActivityCopying: {
            if(installedItem->categoryID() != EVEDB::invCategories::Blueprint)
                throw(PyException(MakeUserError("RamActivityRequiresABlueprint")));

            BlueprintRef bp = BlueprintRef::StaticCast( installedItem );

            if(bp->copy())
                throw(PyException(MakeUserError("RamCannotCopyABlueprintCopy")));

            productType = &bp->type();
            break;
        }
        /*
         * The rest
         */
        case ramActivityResearchingTechnology:
        case ramActivityDuplicating:
        case ramActivityReverseEngineering:
        case ramActivityInvention: /* {
            if(installedItem->categoryID() != EVEDB::invCategories::Blueprint)
                throw(PyException(MakeUserError("RamActivityRequiresABlueprint")));

            Blueprint *bp = (Blueprint *)installedItem;

            if(!bp->copy())
                throw(PyException(MakeUserError("RamCannotInventABlueprintOriginal")));

            uint32 productTypeID = m_db.GetTech2Blueprint(installedItem->typeID());
            if(productTypeID == NULL)
                throw(PyException(MakeUserError("RamInventionNoOutput")));

            productType = m_manager->item_factory.type(productTypeID);
            break;
        } */
        default: {
            // not supported
            throw(PyException(MakeUserError("RamActivityInvalid")));
            //throw(PyException(MakeUserError("RamNoKnownOutputType")));
        }
    }

    if(!m_db.IsProducableBy(args.installationAssemblyLineID, productType->groupID()))
        throw(PyException(MakeUserError("RamBadEndProductForActivity")));

    // JOBS CHECK
    // ***********
    if(args.activityID == ramActivityManufacturing) {
        uint32 jobCount = m_db.CountManufacturingJobs(c->GetCharacterID());
        if(c->GetChar()->GetAttribute(AttrManufactureSlotLimit).get_int() <= jobCount) {
            std::map<std::string, PyRep *> exceptArgs;
            exceptArgs["current"] = new PyInt(jobCount);
            exceptArgs["max"] = c->GetChar()->GetAttribute(AttrManufactureSlotLimit).GetPyObject();
            throw(PyException(MakeUserError("MaxFactorySlotUsageReached", exceptArgs)));
        }
    } else {
        uint32 jobCount = m_db.CountResearchJobs(c->GetCharacterID());
        if(c->GetChar()->GetAttribute(AttrMaxLaborotorySlots).get_int() <= jobCount) {
            std::map<std::string, PyRep *> exceptArgs;
            exceptArgs["current"] = new PyInt(jobCount);
            exceptArgs["max"] = c->GetChar()->GetAttribute(AttrMaxLaborotorySlots).GetPyObject();
            throw(PyException(MakeUserError("MaxResearchFacilitySlotUsageReached", exceptArgs)));
        }
    }

    // INSTALLATION CHECK
    // *******************

    uint32 regionID = m_db.GetRegionOfContainer(args.installationContainerID);
    if(regionID == 0)
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
        if(minCharSec > c->GetSecurityRating())
            throw(PyException(MakeUserError("RamAccessDeniedSecStatusTooLow")));

        if(maxCharSec < c->GetSecurityRating())
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
//      if(...)
            throw(PyException(MakeUserError("RamAccessDeniedWrongAlliance")));
    } else if((restrictionMask & ramRestrictByCorp) == ramRestrictByCorp) {
        if(ownerID != c->GetCorporationID())
            throw(PyException(MakeUserError("RamAccessDeniedWrongCorp")));
    }

    if(args.isCorpJob) {
        if((c->GetCorpRole() & corpRoleFactoryManager) != corpRoleFactoryManager)
            throw(PyException(MakeUserError("RamCannotInstallForCorpByRoleFactoryManager")));

        if(args.activityID == ramActivityManufacturing) {
            if((c->GetCorpRole() & corpRoleCanRentFactorySlot) != corpRoleCanRentFactorySlot)
                throw(PyException(MakeUserError("RamCannotInstallForCorpByRole")));
        } else {
            if((c->GetCorpRole() & corpRoleCanRentResearchSlot) != corpRoleCanRentResearchSlot)
                throw(PyException(MakeUserError("RamCannotInstallForCorpByRole")));
        }
    }

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
    if(    (installedItem->flag() == flagCorpSecurityAccessGroup2 && (c->GetCorpRole() & corpRoleHangarCanTake2) != corpRoleHangarCanTake2)
        || (installedItem->flag() == flagCorpSecurityAccessGroup3 && (c->GetCorpRole() & corpRoleHangarCanTake3) != corpRoleHangarCanTake3)
        || (installedItem->flag() == flagCorpSecurityAccessGroup4 && (c->GetCorpRole() & corpRoleHangarCanTake4) != corpRoleHangarCanTake4)
        || (installedItem->flag() == flagCorpSecurityAccessGroup5 && (c->GetCorpRole() & corpRoleHangarCanTake5) != corpRoleHangarCanTake5)
        || (installedItem->flag() == flagCorpSecurityAccessGroup6 && (c->GetCorpRole() & corpRoleHangarCanTake6) != corpRoleHangarCanTake6)
        || (installedItem->flag() == flagCorpSecurityAccessGroup7 && (c->GetCorpRole() & corpRoleHangarCanTake7) != corpRoleHangarCanTake7)
    )
            throw(PyException(MakeUserError("RamAccessDeniedToBOMHangar")));

    // large location check
    if(IsStation(args.installationContainerID)) {
        if(/*args.isCorpJob && */installedItem->flag() == flagCargoHold)
            throw(PyException(MakeUserError("RamCorpInstalledItemNotInCargo")));

        if(installedItem->locationID() != (uint32)args.installationContainerID) {
            if((uint32)args.installationContainerID == c->GetLocationID()) {
                std::map<std::string, PyRep *> exceptArgs;
                exceptArgs["location"] = new PyString(m_db.GetStationName(args.installationContainerID));

                if(args.isCorpJob)
                    throw(PyException(MakeUserError("RamCorpInstalledItemWrongLocation", exceptArgs)));
                else
                    throw(PyException(MakeUserError("RamInstalledItemWrongLocation", exceptArgs)));
            } else
                throw(PyException(MakeUserError("RamRemoteInstalledItemNotInStation")));
        } else {
            if(args.isCorpJob) {
                if(installedItem->flag() < flagCorpSecurityAccessGroup2 || installedItem->flag() > flagCorpSecurityAccessGroup7) {
                    if((uint32)args.installationContainerID == c->GetLocationID()) {
                        std::map<std::string, PyRep *> exceptArgs;
                        exceptArgs["location"] = new PyString(m_db.GetStationName(args.installationContainerID));

                        throw(PyException(MakeUserError("RamCorpInstalledItemWrongLocation", exceptArgs)));
                    } else
                        throw(PyException(MakeUserError("RamRemoteInstalledItemNotInOffice")));
                }
            } else {
                if(installedItem->flag() != flagHangar) {
                    if((uint32)args.installationInvLocationID == c->GetLocationID()) {
                        std::map<std::string, PyRep *> exceptArgs;
                        exceptArgs["location"] = new PyString(m_db.GetStationName(args.installationContainerID));

                        throw(PyException(MakeUserError("RamInstalledItemWrongLocation", exceptArgs)));
                    } else {
                        throw(PyException(MakeUserError("RamRemoteInstalledItemInStationNotHangar")));
                    }
                }
            }
        }
    } else if((uint32)args.installationContainerID == c->GetShipID()) {
        if(c->GetChar()->flag() != flagPilot)
            throw(PyException(MakeUserError("RamAccessDeniedNotPilot")));

        if(installedItem->locationID() != (uint32)args.installationContainerID)
            throw(PyException(MakeUserError("RamInstalledItemMustBeInShip")));
    } else {
        // here should be stuff around POS, but I dont certainly know how it should work, so ...
        // RamInstalledItemBadLocationStructure
        // RamInstalledItemInStructureNotInContainer
        // RamInstalledItemInStructureUnknownLocation
    }

    // BOM LOCATION CHECK
    // *******************

    // corp hangar permission
    if(    (bomLocation.flag == flagCorpSecurityAccessGroup2 && (c->GetCorpRole() & corpRoleHangarCanTake2) != corpRoleHangarCanTake2)
        || (bomLocation.flag == flagCorpSecurityAccessGroup3 && (c->GetCorpRole() & corpRoleHangarCanTake3) != corpRoleHangarCanTake3)
        || (bomLocation.flag == flagCorpSecurityAccessGroup4 && (c->GetCorpRole() & corpRoleHangarCanTake4) != corpRoleHangarCanTake4)
        || (bomLocation.flag == flagCorpSecurityAccessGroup5 && (c->GetCorpRole() & corpRoleHangarCanTake5) != corpRoleHangarCanTake5)
        || (bomLocation.flag == flagCorpSecurityAccessGroup6 && (c->GetCorpRole() & corpRoleHangarCanTake6) != corpRoleHangarCanTake6)
        || (bomLocation.flag == flagCorpSecurityAccessGroup7 && (c->GetCorpRole() & corpRoleHangarCanTake7) != corpRoleHangarCanTake7)
    )
            throw(PyException(MakeUserError("RamAccessDeniedToBOMHangar")));
}

void RamProxyService::_VerifyInstallJob_Install(const Rsp_InstallJob &rsp, const PathElement &pathBomLocation, const std::vector<RequiredItem> &reqItems, const uint32 runs, Client *const c) {
    // MONEY CHECK
    // ************
    if(rsp.cost > c->GetBalance()) {
        std::map<std::string, PyRep *> args;
        args["amount"] = new PyFloat(rsp.cost);
        args["balance"] = new PyFloat(c->GetBalance());

        throw(PyException(MakeUserError("NotEnoughMoney", args)));
    }

    // PRODUCTION TIME CHECK
    // **********************
    if((uint32)rsp.productionTime > ramProductionTimeLimit) {
        std::map<std::string, PyRep *> args;
        args["productionTime"] = new PyInt(rsp.productionTime);
        args["limit"] = new PyInt(ramProductionTimeLimit);

        throw(PyException(MakeUserError("RamProductionTimeExceedsLimits")));
    }

    // SKILLS & ITEMS CHECK
    // *********************
    std::vector<InventoryItemRef> skills, items;

    // get skills ...
    std::set<EVEItemFlags> flags;
    flags.insert(flagSkill);
    flags.insert(flagSkillInTraining);
    c->GetChar()->FindByFlagSet(flags, skills);

    // ... and items
    _GetBOMItems( pathBomLocation, items );

    std::vector<RequiredItem>::const_iterator cur, end;
    cur = reqItems.begin();
    end = reqItems.end();
    for(; cur != end; cur++) {
        // check skill (quantity is required level)
        if(cur->isSkill) {
            /* Commented out until we get skills working some different way ...
            if(GetSkillLevel(skills, cur->typeID) < cur->quantity) {
                std::map<std::string, PyRep *> args;
                args["item"] = new PyString(
                    m_manager->item_factory.type(cur->typeID)->name().c_str()
                );
                args["skillLevel"] = new PyInt(cur->quantity);

                throw(PyException(MakeUserError("RamNeedSkillForJob", args)));
            }*/
        } else {
            // check materials

            // calculate needed quantity
            uint32 qtyNeeded = static_cast<uint32>(ceil(cur->quantity * rsp.materialMultiplier * runs));
            if(cur->damagePerJob == 1.0)
                qtyNeeded = static_cast<uint32>(ceil(qtyNeeded * rsp.charMaterialMultiplier));   // skill multiplier is applied only on fully consumed materials

            std::vector<InventoryItemRef>::iterator curi, endi;
            curi = items.begin();
            endi = items.end();
            for(; curi != endi; curi++) {
                if(    (*curi)->typeID() == cur->typeID
                    && (*curi)->ownerID() == c->GetCharacterID()
                ) {
                    if((*curi)->quantity() < qtyNeeded)
                        qtyNeeded -= (*curi)->quantity();
                    else
                        break;
                }
            }

            if(qtyNeeded > 0) {
                std::map<std::string, PyRep *> args;
                args["item"] = new PyString(
                    m_manager->item_factory.GetType(cur->typeID)->name().c_str()
                );

                throw(PyException(MakeUserError("RamNeedMoreForJob", args)));
            }
        }
    }
}

void RamProxyService::_VerifyCompleteJob(const Call_CompleteJob &args, Client *const c) {
    if((uint32)args.containerID == c->GetShipID())
        if(c->GetLocationID() != (uint32)args.containerID || c->GetChar()->flag() != flagPilot)
            throw(PyException(MakeUserError("RamCompletionMustBeInShip")));

    uint32 ownerID;
    uint64 endProductionTime;
    EVERamCompletedStatus status;
    EVERamRestrictionMask restrictionMask;
    if(!m_db.GetJobVerifyProperties(args.jobID, ownerID, endProductionTime, restrictionMask, status))
        throw(PyException(MakeUserError("RamCompletionNoSuchJob")));

    if(ownerID != c->GetCharacterID()) {
        if(ownerID == c->GetCorporationID()) {
            if((c->GetCorpRole() & corpRoleFactoryManager) != corpRoleFactoryManager)
                throw(PyException(MakeUserError("RamCompletionAccessDeniedByCorpRole")));
        } else  // alliances not implemented
            throw(PyException(MakeUserError("RamCompletionAccessDenied")));
    }

    if(status != ramCompletedStatusInProgress)
        throw(PyException(MakeUserError("RamCompletionJobCompleted")));

    if(!args.cancel && endProductionTime > Win32TimeNow())
        throw(PyException(MakeUserError("RamCompletionInProduction")));
}

bool RamProxyService::_Calculate(const Call_InstallJob &args, InventoryItemRef installedItem, Client *const c, Rsp_InstallJob &into) {
    if(!m_db.GetAssemblyLineProperties(args.installationAssemblyLineID, into.materialMultiplier, into.timeMultiplier, into.installCost, into.usageCost))
        return false;

    const ItemType *productType;
    // perform some activity-specific actions
    switch(args.activityID) {
        /*
         * Manufacturing
         */
        case ramActivityManufacturing: {
            BlueprintRef bp = BlueprintRef::StaticCast( installedItem );

            productType = &bp->productType();

            into.productionTime = bp->type().productionTime();

            into.materialMultiplier *= bp->materialMultiplier();
            into.timeMultiplier *= bp->timeMultiplier();

            into.charMaterialMultiplier = c->GetChar()->GetAttribute(AttrManufactureCostMultiplier).get_float();
            into.charTimeMultiplier = c->GetChar()->GetAttribute(AttrManufactureTimeMultiplier).get_float();

            switch(productType->race()) {
                case raceCaldari:       into.charTimeMultiplier *= double(c->GetChar()->GetAttribute(AttrCaldariTechTimePercent).get_int()) / 100.0; break;
                case raceMinmatar:      into.charTimeMultiplier *= double(c->GetChar()->GetAttribute(AttrMinmatarTechTimePercent).get_int()) / 100.0; break;
                case raceAmarr:         into.charTimeMultiplier *= double(c->GetChar()->GetAttribute(AttrAmarrTechTimePercent).get_int()) / 100.0; break;
                case raceGallente:      into.charTimeMultiplier *= double(c->GetChar()->GetAttribute(AttrGallenteTechTimePercent).get_int()) / 100.0; break;
                case raceJove:          break;
                case racePirate:        break;
            }
            break;
        }
        /*
         * Time productivity research
         */
        case ramActivityResearchingTimeProductivity: {
            BlueprintRef bp = BlueprintRef::StaticCast( installedItem );

            productType = &installedItem->type();

            into.productionTime = bp->type().researchProductivityTime();
            into.charMaterialMultiplier = double(c->GetChar()->GetAttribute(AttrResearchCostPercent).get_int()) / 100.0;
            into.charTimeMultiplier = c->GetChar()->GetAttribute(AttrManufacturingTimeResearchSpeed).get_float();
            break;
        }
        /*
         * Material productivity research
         */
        case ramActivityResearchingMaterialProductivity: {
            BlueprintRef bp = BlueprintRef::StaticCast( installedItem );

            productType = &installedItem->type();

            into.productionTime = bp->type().researchMaterialTime();
            into.charMaterialMultiplier = double(c->GetChar()->GetAttribute(AttrResearchCostPercent).get_int()) / 100.0;
            into.charTimeMultiplier = c->GetChar()->GetAttribute(AttrMineralNeedResearchSpeed).get_float();
            break;
        }
        /*
         * Copying
         */
        case ramActivityCopying: {
            BlueprintRef bp = BlueprintRef::StaticCast( installedItem );

            productType = &installedItem->type();

            // no ceil() here on purpose
            into.productionTime = (bp->type().researchCopyTime() / bp->type().maxProductionLimit()) * args.licensedProductionRuns;

            into.charMaterialMultiplier = double(c->GetChar()->GetAttribute(AttrResearchCostPercent).get_int()) / 100.0;
            into.charTimeMultiplier = c->GetChar()->GetAttribute(AttrCopySpeedPercent).get_float();
            break;
        }
        default: {
            productType = &installedItem->type();

            into.charMaterialMultiplier = 1.0;
            into.charTimeMultiplier = 1.0;
            break;
        }
    }

    if(!m_db.MultiplyMultipliers(args.installationAssemblyLineID, productType->groupID(), into.materialMultiplier, into.timeMultiplier))
        return false;

    // calculate the remaining things
    into.productionTime *= static_cast<int32>(into.timeMultiplier * into.charTimeMultiplier * args.runs);
    into.usageCost *= ceil(into.productionTime / 3600.0);
    into.cost = into.installCost + into.usageCost;

    // I "hope" this is right, simple tells client how soon will his job be started
    // Unfortunately, rounding done on client's side causes showing "Start time: 0 seconds" when he has to wait less than minute
    // I have no idea how to avoid this ...
    into.maxJobStartTime = m_db.GetNextFreeTime(args.installationAssemblyLineID);

    return true;
}

void RamProxyService::_EncodeBillOfMaterials(const std::vector<RequiredItem> &reqItems, double materialMultiplier, double charMaterialMultiplier, uint32 runs, BillOfMaterials &into)
{
    PySafeDecRef( into.extras.lines );
    into.extras.lines = new PyList;
    PySafeDecRef( into.wasteMaterials.lines );
    into.wasteMaterials.lines = new PyList;
    PySafeDecRef( into.rawMaterials.lines );
    into.rawMaterials.lines = new PyList;

    std::vector<RequiredItem>::const_iterator cur, end;
    cur = reqItems.begin();
    end = reqItems.end();
    for(; cur != end; cur++) {
        // if it's skill, insert it into special dict for skills
        if(cur->isSkill) {
            into.skills[cur->typeID] = new PyInt(cur->quantity);
            continue;
        }

        // otherwise, make line for material list
        MaterialList_Line line;
        line.requiredTypeID = cur->typeID;
        line.quantity = static_cast<int32>(ceil(cur->quantity * materialMultiplier * runs));
        line.damagePerJob = cur->damagePerJob;
        line.isSkillCheck = false;  // no idea what is this for
        line.requiresHP = false;    // no idea what is this for

        // and this is thing I'm not sure about ... if I understood it well, "Extra material" is everything not fully consumed,
        // "Raw material" is everything fully consumed and "Waste Material" is amount of material wasted ...
        if(line.damagePerJob < 1.0) {
            into.extras.lines->AddItem( line.Encode() );
        } else {
            // if there are losses, make line for waste material list
            if(charMaterialMultiplier > 1.0) {
                MaterialList_Line wastage( line );  // simply copy origial line ...
                wastage.quantity = static_cast<int32>(ceil(wastage.quantity * (charMaterialMultiplier - 1.0))); // ... and calculate proper quantity

                into.wasteMaterials.lines->AddItem( wastage.Encode() );
            }
            into.rawMaterials.lines->AddItem( line.Encode() );
        }
    }
}

void RamProxyService::_EncodeMissingMaterials(const std::vector<RequiredItem> &reqItems, const PathElement &bomLocation, Client *const c, double materialMultiplier, double charMaterialMultiplier, int32 runs, std::map<int32, PyRep *> &into) {
    //query out what we need
    std::vector<InventoryItemRef> skills, items;

    //get the skills
    c->GetChar()->FindByFlag(flagSkill, skills);
    c->GetChar()->FindByFlag(flagSkillInTraining, skills);

    //get the items
    _GetBOMItems( bomLocation, items );

    //now do the check
    std::vector<RequiredItem>::const_iterator cur, end;
    cur = reqItems.begin();
    end = reqItems.end();
    for(; cur != end; cur++) {
        uint32 qtyReq = cur->quantity;
        if(!cur->isSkill) {
            qtyReq = static_cast<uint32>(ceil(qtyReq * materialMultiplier * runs));
            if(cur->damagePerJob == 1.0)
                qtyReq = static_cast<uint32>(ceil(qtyReq * charMaterialMultiplier));
        }

        std::vector<InventoryItemRef>::const_iterator curi, endi;
        curi = (cur->isSkill ? skills.begin() : items.begin());
        endi = (cur->isSkill ? skills.end() : items.end());
        for(; curi != endi && qtyReq > 0; curi++) {
            if((*curi)->typeID() == cur->typeID && (*curi)->ownerID() == c->GetCharacterID()) {
                if(cur->isSkill)
                    qtyReq -= std::min((uint32)qtyReq, (uint32)(*curi)->GetAttribute(AttrSkillLevel).get_int());
                else
                    qtyReq -= std::min((uint32)qtyReq, (uint32)(*curi)->GetAttribute(AttrQuantity).get_int());
            }
        }

        if(qtyReq > 0)
            into[cur->typeID] = new PyInt(qtyReq);
    }
}

void RamProxyService::_GetBOMItems(const PathElement &bomLocation, std::vector<InventoryItemRef> &into)
{
    Inventory *inventory = m_manager->item_factory.GetInventory( bomLocation.locationID );
    if( inventory != NULL )
        inventory->FindByFlag( (EVEItemFlags)bomLocation.flag, into );
}

