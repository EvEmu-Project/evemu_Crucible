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
	Author:		Luck
*/

#include "EVEServerPCH.h"
//#include "ModuleDB.h"


// ////////////////// MEffect Class ///////////////////////////

MEffect::MEffect(uint32 effectID)
{
    m_EffectID = effectID;
    _Populate(effectID);
}

MEffect::~MEffect()
{
    /* nothing to do */
}

void MEffect::_Populate(uint32 effectID)
{
	DBQueryResult *res = new DBQueryResult();
	ModuleDB::GetDgmEffects(effectID, *res);

    // First, get all general info on this effectID from the dgmEffects table:
	DBResultRow row1;
	if( !res->GetRow(row1) )
		sLog.Error("MEffect","Could not populate effect information for effectID: %u from the 'dgmEffects' table", effectID);
	else
	{
		//get all the data from the query
		m_EffectID = effectID;
		m_EffectName = row1.GetText(0);
		m_EffectCategory = row1.GetInt(1);
		m_PreExpression = row1.GetInt(2);
		m_PostExpression = row1.GetInt(3);
		if( !row1.IsNull(4) )
			m_Description = row1.GetText(4);
		if( !row1.IsNull(5) )
			m_Guid = row1.GetText(5);
		if( !row1.IsNull(6) )
			m_IconID = row1.GetInt(6);
		m_IsOffensive = row1.GetInt(7);
		m_IsAssistance = row1.GetInt(8);
		if( !row1.IsNull(9) )
			m_DurationAttributeID = row1.GetInt(9);
		if( !row1.IsNull(10) )
			m_TrackingSpeedAttributeID = row1.GetInt(10);
		if( !row1.IsNull(11) )
			m_DischargeAttributeID = row1.GetInt(11);
		if( !row1.IsNull(12) )
			m_RangeAttributeID = row1.GetInt(12);
		if( !row1.IsNull(13) )
			m_FalloffAttributeID = row1.GetInt(13);
		if( !row1.IsNull(14) )
			m_DisallowAutoRepeat = row1.GetInt(14);
		m_Published = row1.GetInt(15);
		if( !row1.IsNull(16) )
			m_DisplayName = row1.GetText(16);
		m_IsWarpSafe = row1.GetInt(17);
		m_RangeChance = row1.GetInt(18);
		m_ElectronicChance = row1.GetInt(19);
		m_PropulsionChance = row1.GetInt(20);
		if( !row1.IsNull(21) )
			m_Distribution = row1.GetInt(21);
		if( !row1.IsNull(22) )
			m_SfxName = row1.GetText(22);
		if( !row1.IsNull(23) )
			m_NpcUsageChanceAttributeID = row1.GetInt(23);
		if( !row1.IsNull(24) )
			m_NpcActivationChanceAttributeID = row1.GetInt(24);
		if( !row1.IsNull(25) )
			m_FittingUsageChanceAttributeID = row1.GetInt(25);
	}

	// Next, get the info from the dgmEffectsInfo table:
	ModuleDB::GetDgmEffectsInfo(effectID, *res);

	DBResultRow row2;

	// Initialize the new tables
	m_TargetAttributeIDs = new int[res->GetRowCount()];
	m_SourceAttributeIDs = new int[res->GetRowCount()];
	m_CalculationTypeIDs = new int[res->GetRowCount()];
	m_ReverseCalculationTypeIDs = new int[res->GetRowCount()];

	int count = 0;

	while( res->GetRow(row2) )
	{
		m_TargetAttributeIDs[count] = row2.GetInt(0);
		m_SourceAttributeIDs[count] = row2.GetInt(1);
		m_CalculationTypeIDs[count] = row2.GetInt(2);
		m_ReverseCalculationTypeIDs[count] = row2.GetInt(3);
		count++;
	}

    if( count == 0 )
        sLog.Error("MEffect","Could not populate effect information for effectID: %u from the 'dgmEffectsInfo' table as the SQL query returned ZERO rows", effectID);

	m_numOfIDs = count;

	// Finally, get the info for this effectID from the dgmEffectsActions table:
	ModuleDB::GetDgmEffectsActions(effectID, *res);

	DBResultRow row3;

    if( !(res->GetRow(row3)) )
        sLog.Error("MEffect","Could not populate effect information for effectID: %u from 'dgmEffectsActions table", effectID);
    else
    {
        m_EffectAppliedWhenID = row3.GetInt(0);
        m_EffectAppliedTargetID = row3.GetInt(1);
        m_EffectApplicationTypeID = row3.GetInt(2);
        m_StackingPenaltyAppliedID = row3.GetInt(3);
        m_NullifyOnlineEffectEnable = row3.GetInt(4);
        m_NullifiedOnlineEffectID = row3.GetInt(5);
    }

	delete res;
	res = NULL;
}


// ////////////////////// ModuleEffects Class ////////////////////////////

ModuleEffects::ModuleEffects(uint32 typeID) : m_typeID( typeID ), m_Cached( false )
{
    m_typeID = 0;
	m_HighPower = m_MediumPower = m_LowPower = m_Cached = false;

    _populate(typeID);
}

ModuleEffects::~ModuleEffects()
{
	//delete arrays

	//null ptrs
}

//useful accessors - probably a better way to do this, but at least it's fast
bool ModuleEffects::isHighSlot()
{
	if( m_Cached )
		return m_HighPower;
	else
	{
	/*	for(uint32 i = 0; i < m_EffectCount; i++)
		{
			if( m_EffectIDs[i] == effectHiPower )
			{
				m_HighPower = true;
				m_MediumPower = false;
				m_LowPower = false;
				m_Cached = true; //cache the result
				return true;
			}
		}*/
	}

    return false;
}

bool ModuleEffects::isMediumSlot()
{
	if( m_Cached )
		return m_MediumPower;
	else
	{
	/*	for(uint32 i = 0; i < m_EffectCount; i++)
		{
			if( m_EffectIDs[i] == effectMedPower )
			{
				m_HighPower = false;
				m_MediumPower = true;
				m_LowPower = false;
				m_Cached = true;  //cache the result
				return true;
			}
		}*/
	}

    return false;
}

bool ModuleEffects::isLowSlot()
{
	if( m_Cached )
		return m_LowPower;
	else
	{
	/*	for(uint32 i = 0; i < m_EffectCount; i++)
		{
			if( m_EffectIDs[i] == effectLoPower )
			{
				m_HighPower = false;
				m_MediumPower = false;
				m_LowPower = true;
				m_Cached = true; //cache the result
				return true;
			}
		}*/
	}

    return false;
}

bool ModuleEffects::HasEffect(uint32 effectID)
{
    std::map<uint32, MEffect *>::const_iterator cur, end;

    if( m_OnlineEffects.find(effectID) != m_OnlineEffects.end() )
        return true;

    if( m_ActiveEffects.find(effectID) != m_ActiveEffects.end() )
        return true;

    if( m_OverloadEffects.find(effectID) != m_OverloadEffects.end() )
        return true;

    return false;
}

MEffect * ModuleEffects::GetEffect(uint32 effectID)
{
    // WARNING: This function MUST be defined!
    return NULL;
}

//this will need to be reworked to implement a singleton architecture...i'll do it later -luck


// ////////////////// PRIVATE MEMBERS /////////////////////////

void ModuleEffects::_populate(uint32 typeID)
{
	//first get list of all of the effects associated with the typeID
	DBQueryResult *res = new DBQueryResult();
	ModuleDB::GetDgmTypeEffectsInformation(typeID, *res);

	//counter
    MEffect * mEffectPtr;
    mEffectPtr = NULL;
    m_defaultEffect = NULL;     // Set this to NULL until the default effect is found, if there is any
    uint32 effectID;
    uint32 isDefault;

	//go through and populate each effect
	DBResultRow row;
	while( res->GetRow(row) )
	{
        effectID = row.GetInt(0);
        isDefault = row.GetInt(1);
        switch( effectID )
        {
            case 11:    // loPower
            case 12:    // hiPower
            case 13:    // medPower
                // We do not need to make MEffect objects these effectIDs, since they do nothing
                mEffectPtr = NULL;
                break;

            default:
                mEffectPtr = new MEffect( effectID );
                break;
        }
        
        if( isDefault > 0 )
            m_defaultEffect = mEffectPtr;

        // This switch is assuming that all entries in 'dgmEffectsInfo' for this effectID are applied during the same module state,
        // which should really be the case anyway, for every effectID, so we just check index 0 of the effectIDs list of attributes
        // that are modified by this effect for which module state during which the effect is active:
        if( mEffectPtr != NULL )
        {
            switch( mEffectPtr->GetModuleStateWhenEffectApplied(0) )
            {
                case EFFECT_ONLINE:
                    m_OnlineEffects.insert(std::pair<uint32, MEffect *>(effectID,mEffectPtr));
                    break;
                case EFFECT_ACTIVE:
                    m_ActiveEffects.insert(std::pair<uint32, MEffect *>(effectID,mEffectPtr));
                    break;
                case EFFECT_OVERLOAD:
                    m_OverloadEffects.insert(std::pair<uint32, MEffect *>(effectID,mEffectPtr));
                    break;
                default:
                    sLog.Error("ModuleEffects::_populate()", "Illegal value '%u' obtained from the 'effectAppliedInState' field of the 'dgmEffectsInfo' table", mEffectPtr->GetModuleStateWhenEffectApplied(0));
                    break;
            }
        }
	}

	//cleanup
	delete res;
	res = NULL;

}
