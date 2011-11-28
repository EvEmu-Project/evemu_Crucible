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

#ifndef MODULE_EFFECTS_H
#define MODULE_EFFECTS_H

//class contained by all modules that is populated on construction of the module
//this will contain all information about the effects of the module

#include "ModuleDB.h"


class ModuleEffects
{
public:
	ModuleEffects(uint32 typeID) : m_TypeID( typeID ), m_Cached( false )
	{
		_populate(typeID);
	}

	~ModuleEffects()
	{
		//delete arrays
		delete[] m_Effects;
		delete[] m_EffectIDs;

		//null ptrs
		m_EffectIDs = NULL;
		m_Effects = NULL;
	}

	//useful accessors - probably a better way to do this, but at least it's fast
	bool isHighSlot()
	{
		if( m_Cached )
			return m_HighPower;
		else
		{
			for(uint32 i = 0; i < m_EffectCount; i++)
			{
				if( m_EffectIDs[i] == effectHiPower )
				{
					m_HighPower = true;
					m_MediumPower = false;
					m_LowPower = false;
					m_Cached = true; //cache the result
					return true;
				}
			}
		}

        return false;
	}

	bool isMediumSlot()
	{
		if( m_Cached )
			return m_MediumPower;
		else
		{
			for(uint32 i = 0; i < m_EffectCount; i++)
			{
				if( m_EffectIDs[i] == effectMedPower )
				{
					m_HighPower = false;
					m_MediumPower = true;
					m_LowPower = false;
					m_Cached = true;  //cache the result
					return true;
				}
			}
		}

        return false;
	}

	bool isLowSlot()
	{
		if( m_Cached )
			return m_LowPower;
		else
		{
			for(uint32 i = 0; i < m_EffectCount; i++)
			{
				if( m_EffectIDs[i] == effectLoPower )
				{
					m_HighPower = false;
					m_MediumPower = false;
					m_LowPower = true;
					m_Cached = true; //cache the result
					return true;
				}
			}
		}

        return false;
	}

	//this will need to be reworked to implement a singleton architecture...i'll do it later -luck

	//this class uses a system to set the "active effect" that you want to get information about
	//returns false if the effect specified is not found
	bool SetActiveEffect(uint32 effectID)
	{
		//iterate through the effects to find the one we want
		for(uint32 i = 0; i < m_EffectCount; i++)
		{
			if(m_EffectIDs[i] == effectID)
			{
				m_SelectedEffect = i;
				return true;
			}

		}

		return false;
	}

	bool SetDefaultEffectAsActive()
	{
		//iterate through the effects to find the one we want
		if(m_DefaultEffect != 0)
		{
			m_SelectedEffect = m_DefaultEffect;
			return true;
		}

		return false;
	}

	//accessors for selected effect
	uint32 GetEffectID()										{ return m_Effects[m_SelectedEffect].m_EffectID; }
	std::string GetEffectName()									{ return m_Effects[m_SelectedEffect].m_EffectName; }
	uint32 GetEffectCategory()									{ return m_Effects[m_SelectedEffect].m_EffectCategory; }
	uint32 GetPreExpression()									{ return m_Effects[m_SelectedEffect].m_PreExpression; }
	uint32 GetPostExpression()									{ return m_Effects[m_SelectedEffect].m_PostExpression; }
	std::string GetDescription()								{ return m_Effects[m_SelectedEffect].m_Description; }
	std::string GetGuid()										{ return m_Effects[m_SelectedEffect].m_Guid; }
	uint32 GetIconID()											{ return m_Effects[m_SelectedEffect].m_IconID; }
	bool GetIsOffensive()										{ return m_Effects[m_SelectedEffect].m_IsOffensive == 1; }
	bool GetIsAssistance()										{ return m_Effects[m_SelectedEffect].m_IsAssistance == 1; }
	uint32 GetDurationAttributeID()								{ return m_Effects[m_SelectedEffect].m_DurationAttributeID; }
	uint32 GetTrackingSpeedAttributeID()						{ return m_Effects[m_SelectedEffect].m_TrackingSpeedAttributeID; }
	uint32 GetDischargeAttributeID()							{ return m_Effects[m_SelectedEffect].m_DischargeAttributeID; }
	uint32 GetRangeAttributeID()								{ return m_Effects[m_SelectedEffect].m_RangeAttributeID; }
	uint32 GetFalloffAttributeID()								{ return m_Effects[m_SelectedEffect].m_FalloffAttributeID; }
	bool GetDisallowAutoRepeat()								{ return m_Effects[m_SelectedEffect].m_DisallowAutoRepeat == 1; }
	bool GetIsPublished()										{ return m_Effects[m_SelectedEffect].m_Published == 1; }
	std::string GetDisplayName()								{ return m_Effects[m_SelectedEffect].m_DisplayName; }
	bool GetIsWarpSafe()										{ return m_Effects[m_SelectedEffect].m_IsWarpSafe == 1; }
	bool GetRangeChance()										{ return m_Effects[m_SelectedEffect].m_RangeChance == 1; }
	bool GetPropulsionChance()									{ return m_Effects[m_SelectedEffect].m_PropulsionChance == 1; }
	bool GetElectronicChance()									{ return m_Effects[m_SelectedEffect].m_ElectronicChance == 1; }
	uint32 GetDistribution()									{ return m_Effects[m_SelectedEffect].m_Distribution; }
	std::string GetSfxName()									{ return m_Effects[m_SelectedEffect].m_DisplayName; }
	uint32 GetNpcUsageChanceAttributeID()						{ return m_Effects[m_SelectedEffect].m_NpcUsageChanceAttributeID; }
	uint32 GetNpcActivationChanceAttributeID()					{ return m_Effects[m_SelectedEffect].m_NpcActivationChanceAttributeID; }
	uint32 GetFittingUsageChanceAttributeID()					{ return m_Effects[m_SelectedEffect].m_FittingUsageChanceAttributeID; }

	//accessors for the effects targetAttributeID, sourceAttributeID and calculation type
	uint32 GetSizeOfAttributeList()								{ return m_Effects[m_SelectedEffect].m_numOfIDs; }
	uint32 GetTargetAttributeID(uint32 count)					{ return m_Effects[m_SelectedEffect].m_TargetAttributeIDs[count]; }
	uint32 GetSourceAttributeID(uint32 count)					{ return m_Effects[m_SelectedEffect].m_SourceAttributeIDs[count]; }
	EVECalculationType GetCalculationType(uint32 count)			{ return (EVECalculationType)m_Effects[m_SelectedEffect].m_CalculationTypeID[count];}
	EVECalculationType GetReverseCalculationType(uint32 count)	{ return (EVECalculationType)m_Effects[m_SelectedEffect].m_ReverseCalculationTypeID[count];}

private:

	class MEffect
	{
	public:
		MEffect() { /* nothing to do */ }

		~MEffect() { /* nothing to do */ }

		void Populate(uint32 effectID)
		{
			DBQueryResult *res = new DBQueryResult();
			ModuleDB::GetDgmEffects(effectID, *res);

			DBResultRow row1;
			if( !res->GetRow(row1) )
				sLog.Error("MEffect","Could not populate effect information for effectID: %u", effectID);
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

			//next get the info from the dgmEffectsInfo table
			ModuleDB::GetDgmEffectsInfo(effectID, *res);

			DBResultRow row2;

			//initialize the new tables
			m_TargetAttributeIDs = new int[res->GetRowCount()];
			m_SourceAttributeIDs = new int[res->GetRowCount()];
			m_CalculationTypeID = new int[res->GetRowCount()];
			m_ReverseCalculationTypeID = new int[res->GetRowCount()];

			//counter
			int count = 0;

			while( res->GetRow(row2) )
			{
				m_TargetAttributeIDs[count] = row2.GetInt(1);
				m_SourceAttributeIDs[count] = row2.GetInt(2);
				m_CalculationTypeID[count] = row2.GetInt(3);
				m_ReverseCalculationTypeID[count] = row2.GetInt(4);
				count++;
			}

			m_numOfIDs = count;

			delete res;
			res = NULL;

		}

		int * m_TargetAttributeIDs;
		int * m_SourceAttributeIDs;
		int * m_CalculationTypeID;
		int * m_ReverseCalculationTypeID;
		int m_numOfIDs;

		int m_EffectID;
		std::string m_EffectName;
		int m_EffectCategory;
		int m_PreExpression;
		int m_PostExpression;
		std::string m_Description;
		std::string m_Guid;
		int m_IconID;
		int m_IsOffensive;
		int m_IsAssistance;
		int m_DurationAttributeID;
		int m_TrackingSpeedAttributeID;
		int m_DischargeAttributeID;
		int m_RangeAttributeID;
		int m_FalloffAttributeID;
		int m_DisallowAutoRepeat;
		int m_Published;
		std::string m_DisplayName;
		int m_IsWarpSafe;
		int m_RangeChance;
		int m_ElectronicChance;
		int m_PropulsionChance;
		int m_Distribution;
		std::string m_SfxName;
		int m_NpcUsageChanceAttributeID;
		int m_NpcActivationChanceAttributeID;
		int m_FittingUsageChanceAttributeID;

	};

	void _populate(uint32 typeID)
	{
		//first get all of the effects associated with the typeID
		DBQueryResult *res = new DBQueryResult();
		ModuleDB::GetDgmTypeEffectsInformation(typeID, *res);

		//initialize our container to the correct size
		m_Effects = new MEffect[res->GetRowCount()];
		m_EffectIDs = new int[res->GetRowCount()];

		//counter
		int i = 0;

		//go through and populate each effect
		DBResultRow row;
		while( res->GetRow(row) )
		{
			//add new stuff to the arrays
			m_EffectIDs[i] = row.GetInt(0);
			m_Effects[i] = *new MEffect();

			//check if this is the default effect
			if( row.GetInt(1) )
				m_DefaultEffect = i;

			//populate the new MEffect
			m_Effects[i].Populate(row.GetInt(0));

			i++; //increment
		}

		m_EffectCount = i;

		//cleanup
		delete res;
		res = NULL;

	}

	//data members
	int m_TypeID;
	int *m_EffectIDs;
	MEffect * m_Effects;
	int m_DefaultEffect;

	//internal counters
	uint32 m_EffectCount;
	uint32 m_SelectedEffect;

	//cached stuff
	bool m_HighPower, m_MediumPower, m_LowPower, m_Cached;

};

#endif /* MODULE_EFFECTS_H */
