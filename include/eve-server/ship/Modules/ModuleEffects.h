

#ifndef MODULE_EFFECTS_H
#define MODULE_EFFECTS_H

//class contained by all modules that is populated on construction of the module
//this will contain all information about the effects of the module

#include "ModuleDB.h"


class ModuleEffects
{
public:
	ModuleEffects(uint32 typeID) : m_TypeID( typeID )
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

private:

	class MEffect
	{
	public:
		MEffect() { }

		void Populate(uint32 effectID)
		{
			DBQueryResult res = ModuleDB::GetDgmEffectsInformation(effectID);

			DBResultRow row;
			if( !res.GetRow(row) )
				sLog.Error("MEffect","Could not populate effect information for effectID: %u", effectID);
			else
			{
				//get all the data from the query
				m_EffectID = effectID;
				m_EffectName = row.GetText(0);
				m_EffectCategory = row.GetInt(1);
				m_PreExpression = row.GetInt(2);
				m_PostExpression = row.GetInt(3);
				if( !row.IsNull(4) )
					m_Description = row.GetText(4);
				if( !row.IsNull(5) )
					m_Guid = row.GetText(5);
				if( !row.IsNull(6) )
					m_IconID = row.GetInt(6);
				m_IsOffensive = row.GetInt(7);
				m_IsAssistance = row.GetInt(8);
				if( !row.IsNull(9) )
					m_DurationAttributeID = row.GetInt(9);
				if( !row.IsNull(10) )
					m_TrackingSpeedAttributeID = row.GetInt(10);
				if( !row.IsNull(11) )
					m_DischargeAttributeID = row.GetInt(11);
				if( !row.IsNull(12) )
					m_RangeAttributeID = row.GetInt(12);
				if( !row.IsNull(13) )
					m_FalloffAttributeID = row.GetInt(13);
				m_DisallowAutoRepeat = row.GetInt(14);
				m_Published = row.GetInt(15);
				if( !row.IsNull(16) )
					m_DisplayName = row.GetText(16);
				m_IsWarpSafe = row.GetInt(17);
				m_RangeChance = row.GetInt(18);
				m_ElectronicChance = row.GetInt(19);
				m_PropulsionChance = row.GetInt(20);
				if( !row.IsNull(21) )
					m_Distribution = row.GetInt(21);
				if( !row.IsNull(22) )
					m_SfxName = row.GetText(22);
				if( !row.IsNull(23) )
					m_NpcUsageChanceAttributeID = row.GetInt(23);
				if( !row.IsNull(24) )
					m_NpcActivationChanceAttributeID = row.GetInt(24);
				if( !row.IsNull(25) )
					m_FittingUsageChanceAttributeID = row.GetInt(25);

			}
		}

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
		DBQueryResult res = ModuleDB::GetDgmTypeEffectsInformation(typeID);

		//initialize our container to the correct size
		m_Effects = new MEffect[res.GetRowCount()];
		m_EffectIDs = new int[res.GetRowCount()];

		//counter
		int i = 0;

		//go through and populate each effect
		DBResultRow row;
		while( res.GetRow(row) )
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

	}

	int m_TypeID;
	int *m_EffectIDs;
	MEffect * m_Effects;
	int m_DefaultEffect;

};

#endif /* MODULE_EFFECTS_H */