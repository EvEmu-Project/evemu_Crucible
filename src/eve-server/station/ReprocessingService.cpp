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
    Author:     Zhur
    Rewrite:    Allan
*/

/**  @todo  update this entire file...  */
/*
 * # Manufacturing Logging:
 * MANUF__ERROR
 * MANUF__WARNING
 * MANUF__MESSAGE
 * MANUF__INFO
 * MANUF__DEBUG
 * MANUF__TRACE
 * MANUF__DUMP
 */


#include "eve-server.h"

#include "PyServiceCD.h"
#include "packets/Manufacturing.h"
#include "manufacturing/RamMethods.h"
#include "station/ReprocessingService.h"
#include "Station.h"
#include "system/SystemManager.h"

PyCallable_Make_InnerDispatcher(ReprocessingService)
PyCallable_Make_InnerDispatcher(ReprocessingServiceBound)

ReprocessingService::ReprocessingService(PyServiceMgr *mgr)
: PyService(mgr, "reprocessingSvc"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);
}

ReprocessingService::~ReprocessingService() {
    delete m_dispatch;
}

PyBoundObject *ReprocessingService::CreateBoundObject(Client *pClient, const PyRep *bind_args) {
    if (!bind_args->IsInt()) {
        _log(SERVICE__ERROR, "%s: Non-integer bind argument '%s'", pClient->GetName(), bind_args->TypeString());
        return nullptr;
    }

    uint32 stationID = bind_args->AsInt()->value();
    if (!sDataMgr.IsStation(stationID)) {
        _log(SERVICE__ERROR, "%s: Expected stationID, but got %u.", pClient->GetName(), stationID);
        return nullptr;
    }

    return new ReprocessingServiceBound(m_manager, m_db, stationID);
}


ReprocessingServiceBound::ReprocessingServiceBound(PyServiceMgr *mgr, ReprocessingDB& db, uint32 stationID)
: PyBoundObject(mgr),
m_dispatch(new Dispatcher(this)),
m_db(db),
m_stationCorpID(0),
m_staEfficiency(0.0f),
m_tax(0.0f)
{
    _SetCallDispatcher(m_dispatch);

    m_strBoundObjectName = "ReprocessingServiceBound";

    PyCallable_REG_CALL(ReprocessingServiceBound, GetOptionsForItemTypes);
    PyCallable_REG_CALL(ReprocessingServiceBound, GetReprocessingInfo);
    PyCallable_REG_CALL(ReprocessingServiceBound, GetQuote);
    PyCallable_REG_CALL(ReprocessingServiceBound, GetQuotes);
    PyCallable_REG_CALL(ReprocessingServiceBound, Reprocess);

    m_stationRef = sItemFactory.GetStationRef(stationID);
    if (m_stationRef.get() != nullptr)
        m_stationRef->GetRefineData(m_stationCorpID, m_staEfficiency, m_tax);
}

ReprocessingServiceBound::~ReprocessingServiceBound() {
    delete m_dispatch;
}

void ReprocessingServiceBound::Release() {
    //I hate this statement
    delete this;
}

PyResult ReprocessingServiceBound::Handle_GetOptionsForItemTypes(PyCallArgs &call) {
    _log(MANUF__INFO, "%s: Calling GetOptionsForItemTypes().", call.client->GetName());
    call.Dump(MANUF__DUMP);

    Call_GetOptionsForItemTypes args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        call.client->SendErrorMsg("Internal Server Error.  Ref: ServerError 01588.");
        return nullptr;
    }

    Rsp_GetOptionsForItemTypes      rsp;
    Rsp_GetOptionsForItemTypes_Arg  arg;

    for (auto cur : args.typeIDs) {
        arg.isRecyclable = sDataMgr.IsRecyclable(cur.first);
        arg.isRefinable = sDataMgr.IsRefinable(cur.first);
        rsp.typeIDs[cur.first] = arg.Encode();
    }

    return rsp.Encode();
}

PyResult ReprocessingServiceBound::Handle_GetReprocessingInfo(PyCallArgs &call) {
    Client *pClient = call.client;
    Rsp_GetReprocessingInfo rsp;
        rsp.standing = GetStanding(pClient);
        rsp.tax = CalcTax( rsp.standing );
        rsp.yield = m_staEfficiency;
        rsp.combinedyield = CalcReprocessingEfficiency(pClient);
    return rsp.Encode();
}

PyResult ReprocessingServiceBound::Handle_GetQuote(PyCallArgs &call) {
    Call_SingleIntegerArg arg;    // itemID
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        call.client->SendErrorMsg("Internal Server Error.  Ref: ServerError 01588.");
        return nullptr;
    }

    return GetQuote(arg.arg, call.client);
}

PyResult ReprocessingServiceBound::Handle_GetQuotes(PyCallArgs &call) {
    // why shipID here?  processing in cap indy ships?
     Call_GetQuotes args;
     if (!args.Decode(&call.tuple)) {
         codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
         call.client->SendErrorMsg("Internal Server Error.  Ref: ServerError 01588.");
         return nullptr;
     }

    Rsp_GetQuotes rsp;
    for (auto cur : args.itemIDs) {
        PyRep* quote = GetQuote(cur, call.client);
        if (quote != nullptr)
            rsp.quotes[cur] = quote;
    }

    return rsp.Encode();
}

PyResult ReprocessingServiceBound::Handle_Reprocess(PyCallArgs &call) {
    if (!sDataMgr.IsStation(call.client->GetLocationID())) {
        _log(MANUF__WARNING, "Character %s tried to reprocess, but isn't is station.", call.client->GetName());
        return nullptr;
    }

    _log(MANUF__INFO, "%s: Calling Reprocess().", call.client->GetName());
    call.Dump(MANUF__DUMP);

    Call_Reprocess args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        call.client->SendErrorMsg("Internal Server Error.  Ref: ServerError 01588.");
        return nullptr;
    }

    if (args.ownerID == 0)  // should never hit.
        args.ownerID = call.client->GetCharacterID();

    if (args.flag == flagNone)  // should never hit.
        args.flag = flagHangar;

    if (args.ownerID == call.client->GetCorporationID()) {
        if ((call.client->GetCorpRole() & Corp::Role::FactoryManager) != Corp::Role::FactoryManager) {
            _log(MANUF__WARNING, "%s(%u) doesnt have FactoryManager role to access materials for reprocessing.", \
                        call.client->GetName(), call.client->GetCharacterID());
            call.client->SendErrorMsg("You do not have the role \'Factory Manager\' which is required to access factory services on behalf of a corporation.");
            //throw error here...dunno the format yet.
            return nullptr;
        }

        sRamMthd.HangarRolesCheck(call.client, args.flag);
    }

    InventoryItemRef iRef(nullptr);
    double tax = CalcTax(GetStanding(call.client));
    for (auto cur : args.items)  {
        iRef = sItemFactory.GetItemRef(cur);
        if (iRef.get() == nullptr)
            continue;

        // this should never happen, but for sure ...
        if (iRef->type().portionSize() > iRef->quantity()) {
            throw UserError ("QuantityLessThanMinimumPortion")
                    .AddTypeName ("typename", iRef->typeID ())
                    .AddAmount ("portion", iRef->type ().portionSize ());
        }

        float efficiency = CalcReprocessingEfficiency( call.client, iRef );

        // dont hit db for this shit...we kinda have to....dont have this data in static shit.
        std::vector<Recoverable> recoverables;
        if ( !m_db.GetRecoverables( iRef->typeID(), recoverables ) )
            continue;

        std::vector<Recoverable>::iterator itr = recoverables.begin();
        for (; itr != recoverables.end(); ++itr) {
            uint32 full = itr->amountPerBatch * iRef->quantity() / iRef->type().portionSize();
            uint32 quantity = uint32(full * efficiency * (1.0f - tax) );
            if (quantity == 0)
                continue;

            ItemData idata(itr->typeID, args.ownerID, locTemp, flagNone, quantity);
            InventoryItemRef iRef2 = sItemFactory.SpawnItem( idata );
            if (iRef2.get() == nullptr)
                continue;

            // update this for corp usage
            iRef2->Move(m_stationRef->GetID(), (EVEItemFlags)args.flag, true);
        }

        uint32 qtyLeft = iRef->quantity() % iRef->type().portionSize();
        if (qtyLeft) {
            iRef->SetQuantity(qtyLeft, true);
        } else {
            iRef->Move(iRef->locationID(), flagJunkyardReprocessed, true);
            m_stationRef->RemoveItem(iRef);
            iRef->Delete();
        }
    }

    return nullptr;
}

float ReprocessingServiceBound::CalcReprocessingEfficiency(const Client* pClient, InventoryItemRef item) const {
    /* formula is:
        reprocessingEfficiency = 0.375
        *(1 + 0.02 * RefiningSkill)
        *(1 + 0.04 * RefineryEfficiencySkill)
        *(1 + 0.05 * OreProcessingSkill)
    */
    /** @todo  check for implants here ... once they're working  */
    CharacterRef cRef = pClient->GetChar();
    double efficiency =  (0.375f
                        * (1 + (0.02f * cRef->GetSkillLevel(EvESkill::Refining)))             // 2% lvl
                        * (1 + (0.04f * cRef->GetSkillLevel(EvESkill::RefineryEfficiency)))); // 4% lvl

    if (item.get() != nullptr) {
        uint32 specificSkill = item->GetAttribute(AttrReprocessingSkillType).get_int();
        if (specificSkill) {
            efficiency *= (1 + 0.05f * cRef->GetSkillLevel(specificSkill));
        } else {
            efficiency *= (1 + 0.04f * cRef->GetSkillLevel(EvESkill::ScrapmetalProcessing));    // use Scrapmetal Processing as default
        }
    }

    efficiency += m_staEfficiency;

    if (efficiency > 1.0f)
        efficiency = 1.05f;  // should be 1.0 max

    return efficiency;
}

PyRep *ReprocessingServiceBound::GetQuote(uint32 itemID, Client* pClient) {
    InventoryItemRef iRef = sItemFactory.GetItemRef( itemID );
    if (iRef.get() == nullptr)
        return nullptr;    // No action as GetQuote is also called for reprocessed items (probably for check)

    // update this for corp items
    if (iRef->ownerID() == pClient->GetCorporationID()) {
        /** @todo update this for item location - need to verify corp roles are being set correctly  */
        int64 roles = pClient->GetRolesAtAll();
        //roles = pClient->GetRolesAtBase() | pClient->GetRolesAtAll();
        //roles = pClient->GetRolesAtHQ() | pClient->GetRolesAtAll();
        //roles = pClient->GetRolesAtOther() | pClient->GetRolesAtAll();
        if ((pClient->GetCorpRole() & Corp::Role::FactoryManager) != Corp::Role::FactoryManager) {
            _log(MANUF__WARNING, "%s(%u) doesnt have FactoryManager role to access materials for reprocessing.", \
                    pClient->GetName(), pClient->GetCharacterID());
            pClient->SendErrorMsg("You do not have the role \'Factory Manager\' which is required to access factory services on behalf of a corporation.");
            //throw error here...dunno the format yet.
            return nullptr;
        }

        // this throw "access denied to bom hangar" on error.  probably wrong for this application
        sRamMthd.HangarRolesCheck(pClient, iRef->flag());
    } else if (iRef->ownerID() != pClient->GetCharacterID()) {
        _log(SERVICE__ERROR, "Character %u tried to reprocess item %u of character %u.", pClient->GetCharacterID(), iRef->itemID(), iRef->ownerID());
        pClient->SendErrorMsg("The requested item is not yours.");
        return nullptr;
    }

    if (iRef->quantity() < iRef->type().portionSize()) {
        throw UserError ("QuantityLessThanMinimumPortion")
                .AddTypeName ("typename", iRef->typeID ())
                .AddAmount ("portion", iRef->type ().portionSize ());
    }

    std::vector<Recoverable> recoverables;
    if (!m_db.GetRecoverables( iRef->typeID(), recoverables))
        return nullptr;

    Rsp_GetQuote quote;
    quote.lines = new PyList();
    quote.leftOvers = iRef->quantity() % iRef->type().portionSize();
    quote.quantityToProcess = iRef->quantity() - quote.leftOvers;
    quote.playerStanding = GetStanding(pClient);

    double tax = CalcTax( quote.playerStanding );
    double efficiency = CalcReprocessingEfficiency(pClient, iRef);

    for (auto cur :recoverables) {
        uint32 ratio = cur.amountPerBatch * quote.quantityToProcess / iRef->type().portionSize();
        Rsp_GetQuote_Recoverables_Line line;
            line.typeID			= cur.typeID;
            line.client			= uint32(efficiency * (1.0f - tax)   * ratio);
            line.station		= uint32(efficiency * tax           * ratio);
            line.unrecoverable	= ratio - line.client - line.station;
        quote.lines->AddItem( line.Encode() );
    }

    return quote.Encode();
}

/** @todo  should this be moved to standings code?   yes!! */
float ReprocessingServiceBound::GetStanding(const Client* pClient) const
{
    float standing = StandingDB::GetStanding(m_stationCorpID, pClient->GetCharacterID());
    if (standing < 0.0f) {
        standing += ((10.0f + standing) * 0.04f * pClient->GetChar()->GetSkillLevel(EvESkill::Diplomacy));
    } else {
        standing += ((10.0f - standing) * 0.04f * pClient->GetChar()->GetSkillLevel(EvESkill::Connections));
    }

    return EvE::max(standing, StandingDB::GetStanding(m_stationCorpID, pClient->GetCorporationID()));
}

// this should be moved to eve math or eve calc's or w/e
float ReprocessingServiceBound::CalcTax(float standing) const {
    //EvEMath::Refine::StationTaxesForReprocessing(standing);
    float tax = m_tax - 0.75f / 100.0f * standing;
    if (tax < 0.0f)
        tax = 0.0f;
    return tax;
}

