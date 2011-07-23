

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
			for(int i = 0; i < m_EffectCount; i++)
			{
				if( m_EffectIDs[i] == effectHiPower )
				{
					m_HighPower = true;
					m_MediumPower = false;
					m_LowPower = false;
					m_Cached = true; //cache the result
				}
			}
		}
	}

	bool isMediumSlot()
	{
		if( m_Cached )
			return m_MediumPower;
		else
		{
			for(int i = 0; i < m_EffectCount; i++)
			{
				if( m_EffectIDs[i] == effectMedPower )
				{
					m_HighPower = false;
					m_MediumPower = true;
					m_LowPower = false;
					m_Cached = true;  //cache the result
				}
			}
		}
	}

	bool isLowSlot()
	{
		if( m_Cached )
			return m_LowPower;
		else
		{
			for(int i = 0; i < m_EffectCount; i++)
			{
				if( m_EffectIDs[i] == effectLoPower )
				{
					m_HighPower = false;
					m_MediumPower = false;
					m_LowPower = true;
					m_Cached = true; //cache the result
				}
			}
		}
	}


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

			//counter
			int count = 0;

			while( res->GetRow(row2) )
			{
				m_TargetAttributeIDs[count] = row2.GetInt(count);
				m_SourceAttributeIDs[count] = row2.GetInt(count);
				m_CalculationTypeID[count] = row2.GetInt(count);
				count++;
			}

			m_numOfIDs = count;

			delete res;
			res = NULL;

		}

		int * m_TargetAttributeIDs;
		int * m_SourceAttributeIDs;
		int * m_CalculationTypeID;
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
	int m_EffectCount;
	int m_SelectedEffect;

	//cached stuff
	bool m_HighPower, m_MediumPower, m_LowPower, m_Cached;

};

#endif /* MODULE_EFFECTS_H */