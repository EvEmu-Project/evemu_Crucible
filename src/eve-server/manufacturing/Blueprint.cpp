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
    Rewrite:    Allan
*/

/** @todo  load the bp material list from invTypeMaterials */

#include "eve-server.h"

#include "packets/Manufacturing.h"
#include "manufacturing/Blueprint.h"
#include <limits>

/*
 * BlueprintType
 */
BlueprintType::BlueprintType(uint16 _id, const Inv::TypeData& _data, const BlueprintType* _parentBlueprintType,
                             const ItemType& _productType, const EvERam::bpTypeData& _tData)
: ItemType(_id, _data),
  m_parentBlueprintType(_parentBlueprintType),
  m_productType(_productType)
{   // asserts for data consistency
    assert(_tData.productTypeID == m_productType.id());
    if (m_parentBlueprintType != nullptr)
        assert(_tData.parentBlueprintTypeID == m_parentBlueprintType->id());

    m_data = _tData;
}

BlueprintType *BlueprintType::Load(uint16 typeID)
{
    return ItemType::Load<BlueprintType>(typeID);
}

/*
 * Blueprint
 */
Blueprint::Blueprint(uint32 _blueprintID, const BlueprintType& _bpType, const ItemData& _data, EvERam::bpData& _bpData)
: InventoryItem(_blueprintID, _bpType, _data),
m_bpType(_bpType)
{
    // data consistency asserts
    assert(_bpType.categoryID() == EVEDB::invCategories::Blueprint);
    m_data = _bpData;
}

BlueprintRef Blueprint::Load(uint32 blueprintID) {
    return InventoryItem::Load<Blueprint>(blueprintID);
}

BlueprintRef Blueprint::Spawn(ItemData& data, EvERam::bpData& bdata) {
    DBTransaction transaction;
    if (!transaction.Started()) return BlueprintRef(nullptr);
    uint32 blueprintID(Blueprint::CreateItemID(data, bdata));
    if (blueprintID == 0)
        return BlueprintRef(nullptr);
    BlueprintRef bRef = Blueprint::Load(blueprintID);
    if (!bRef) return BlueprintRef(nullptr);
    sItemFactory.AddItem(bRef);
    DBTransaction::OnRollback([bRef]() { bRef->DiscardUncommitted(); });
    if (!transaction.Commit()) return BlueprintRef(nullptr);
    return bRef;
}

uint32 Blueprint::CreateItemID(ItemData& data, EvERam::bpData& bdata) {
    // make sure it's a blueprint type
    const BlueprintType* bpType = sItemFactory.GetBlueprintType(data.typeID);
    if (bpType == nullptr)
        return 0;

    // make the blueprintID
    uint32 blueprintID(InventoryItem::CreateItemID(data));
    if (blueprintID == 0)
        return 0;

    // insert blueprint data into DB
    if (!FactoryDB::SaveBlueprintData(blueprintID, bdata)) {
        return 0;
    }

    return blueprintID;
}

void Blueprint::Delete() {
    InventoryItem::Delete();
}

BlueprintRef Blueprint::SplitBlueprint(int32 qty_to_take, bool notify/*true*/) {
    DBTransaction transaction;
    if (!transaction.Started()) return BlueprintRef(nullptr);
    // split item
    BlueprintRef bRef = BlueprintRef::StaticCast(InventoryItem::Split(qty_to_take, notify));
    if (bRef.get() == nullptr)
        return BlueprintRef(nullptr);

    // InventoryItem::Split copies stored blueprint metadata with the source debit.
    if (!transaction.Commit()) return BlueprintRef(nullptr);
    return bRef;
}

bool Blueprint::Merge(InventoryItemRef itemRef, int32 qty, bool notify) {
    if (!itemRef || itemRef->categoryID()!=EVEDB::invCategories::Blueprint || itemRef->typeID()!=typeID() || itemRef->itemID()==m_itemID)
        return false;
    auto bpRef=BlueprintRef::StaticCast(itemRef);
    if (m_data.copy!=bpRef->copy() || m_data.mLevel!=bpRef->mLevel() || m_data.pLevel!=bpRef->pLevel() || m_data.runs!=bpRef->runs())
        return false;
    DBTransaction transaction;
    DBQueryResult result;DBResultRow row;
    if (!transaction.Started() || !sDatabase.RunQuery(result,"SELECT itemID,copy,mLevel,pLevel,runs FROM invBlueprints WHERE itemID IN (%u,%u) ORDER BY itemID FOR UPDATE",m_itemID,itemRef->itemID()))
        return false;
    unsigned count=0;
    while (result.GetRow(row)) {
        if (row.GetBool(1)!=m_data.copy || row.GetInt(2)!=m_data.mLevel || row.GetInt(3)!=m_data.pLevel || row.GetInt(4)!=m_data.runs) return false;
        ++count;
    }
    return count==2 && InventoryItem::Merge(itemRef,qty,notify) && transaction.Commit();
}

void Blueprint::StoreBlueprintData(EvERam::bpData data) {
    DBTransaction transaction;
    DBQueryResult result;DBResultRow row;
    if (!transaction.Started() || !sDatabase.RunQuery(result,"SELECT copy,mLevel,pLevel,runs FROM invBlueprints WHERE itemID=%u FOR UPDATE",m_itemID) || !result.GetRow(row) ||
        row.GetBool(0)!=m_data.copy || row.GetInt(1)!=m_data.mLevel || row.GetInt(2)!=m_data.pLevel || row.GetInt(3)!=m_data.runs || !FactoryDB::SaveBlueprintData(m_itemID,data))
        throw CustomError("Blueprint save failed; research and copy state were not changed.");
    BlueprintRef self(this);const auto before=m_data;
    DBTransaction::OnRollback([self,before]() { self->m_data=before; });
    m_data=data;
    if (!transaction.Commit()) throw CustomError("Blueprint save failed; research and copy state were not changed.");
}
void Blueprint::SetMLevel(int32 me) {
    if (me<std::numeric_limits<int8>::min() || me>std::numeric_limits<int8>::max()) throw CustomError("Blueprint material level is outside its supported range.");
    auto data=m_data;data.mLevel=me;StoreBlueprintData(data);
}
void Blueprint::SetPLevel(int32 pe) {
    if (pe<std::numeric_limits<int8>::min() || pe>std::numeric_limits<int8>::max()) throw CustomError("Blueprint productivity level is outside its supported range.");
    auto data=m_data;data.pLevel=pe;StoreBlueprintData(data);
}
void Blueprint::SetCopy(bool copy) {
    auto data=m_data;data.copy=copy;StoreBlueprintData(data);
}
void Blueprint::SetRuns(int32 runs) {
    if (runs<std::numeric_limits<int16>::min() || runs>std::numeric_limits<int16>::max()) throw CustomError("Blueprint runs are outside their supported range.");
    auto data=m_data;data.runs=runs;StoreBlueprintData(data);
}
void Blueprint::UpdateMLevel(int32 me) {
    const int64 level=static_cast<int64>(m_data.mLevel)+me;
    if (level<std::numeric_limits<int8>::min() || level>std::numeric_limits<int8>::max()) throw CustomError("Blueprint material level is outside its supported range.");
    SetMLevel(static_cast<int32>(level));
}
void Blueprint::UpdatePLevel(int32 pe) {
    const int64 level=static_cast<int64>(m_data.pLevel)+pe;
    if (level<std::numeric_limits<int8>::min() || level>std::numeric_limits<int8>::max()) throw CustomError("Blueprint productivity level is outside its supported range.");
    SetPLevel(static_cast<int32>(level));
}
void Blueprint::UpdateRuns(int32 runs) {
    const int64 total=static_cast<int64>(m_data.runs)+runs;
    if (total<std::numeric_limits<int16>::min() || total>std::numeric_limits<int16>::max()) throw CustomError("Blueprint runs are outside their supported range.");
    SetRuns(static_cast<int32>(total));
}

PyDict* Blueprint::GetBlueprintAttributes() {
    Rsp_GetBlueprintAttributes rsp;
        rsp.blueprintID = m_itemID;
        rsp.copy = m_data.copy;
        rsp.productivityLevel = m_data.pLevel;
        rsp.materialLevel = m_data.mLevel;
        rsp.licensedProductionRunsRemaining = m_data.runs;
        rsp.wastageFactor = GetME();
        rsp.productTypeID = m_bpType.productTypeID();
        rsp.manufacturingTime = m_bpType.productionTime();
        rsp.maxProductionLimit = m_bpType.maxProductionLimit();
        rsp.researchMaterialTime = m_bpType.researchMaterialTime();
        rsp.researchTechTime = m_bpType.researchTechTime();
        rsp.researchProductivityTime = m_bpType.researchProductivityTime();
        rsp.researchCopyTime = m_bpType.researchCopyTime();
    return rsp.Encode();
}

float Blueprint::GetME()
{
    float bwf(m_bpType.wasteFactor());
    if (m_data.mLevel < 0) {
        bwf /= (-m_data.mLevel);
    } else if (m_data.mLevel > 0) {
        bwf /= (1 + m_data.mLevel);
    }
    bwf /= 100.0f;
    return bwf;
}

/*
                    if activity in (const.activityManufacturing, const.activityDuplicating):
                        if material.requiredTypeID in indexedExtras and indexedExtras[material.requiredTypeID].quantity > 0:
                            extraAmount = indexedExtras[material.requiredTypeID].quantity
                            indexedExtras[material.requiredTypeID].quantity = 0
                        if activity == const.activityManufacturing:
                            blueprintWaste = float(amountRequired) * float(blueprintMaterialMultiplier)
                        characterWaste = float(amountRequired) * float(characterMaterialMultiplier) - float(amountRequired)
                        amountRequired = amountRequired + extraAmount + blueprintWaste
                        amountRequiredByPlayer = int(round(amountRequired + characterWaste))
                        amountRequired = int(round(amountRequired))
                        */
/* invention
 *
 *
 *    The chance for a successful invention is calculated by this formula:
 * Invention_Chance = Base_Chance * (1 + (0.01 * Encryption_Skill_Level)) * (1 + ((Datacore_1_Skill_Level + Datacore_2_Skill_Level) * Decryptor_Modifier
 *      where:
 *        Base Chance
 *            Modules and Ammo have a base probability of 40%
 *            Frigates, Destroyers, Freighters and Skiff have a base probability of 30%
 *            Cruisers, Industrials and Mackinaw have a base probability of 25%
 *            Battlecruisers, Battleships and Hulk have a base probability of 20%
 *            In summary, it's not difficult to get a good chance of success for modules, but progressively more difficult for ships as their size increases. This is why investing in some decryptors is a good idea for Tech II ship invention jobs.
 *        (Racial) Encryption Skill Level
 *            0.01 x Encryption Skill Level (so something between 0.01 and 0.05)
 *        Science Skills (one for each of the required datacores)
 *            0.02 x Combined Level of Advanced Science Skills (so something between 0.04 and 0.2)
 *        Decryptor (optional)
 *            See table above for probability multiplier
 *
 *
 */
