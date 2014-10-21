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
    Author:        Reve
*/

#include "eve-server.h"

#include "EntityList.h"
#include "system/SystemBubble.h"
#include "system/Damage.h"
#include "system/SystemManager.h"
#include "PyServiceMgr.h"
#include "ship/modules/mining_modules/MiningLaser.h"

MiningLaser::MiningLaser( InventoryItemRef item, ShipRef ship )
{
    m_Item = item;
    m_Ship = ship;
    m_Effects = new ModuleEffects(m_Item->typeID());
    m_ShipAttrComp = new ModifyShipAttributesComponent(this, ship);
	m_ActiveModuleProc = new ActiveModuleProcessingComponent(item, this, ship, m_ShipAttrComp);

	m_chargeRef = InventoryItemRef();		// Ensure ref is NULL
	m_chargeLoaded = false;

	m_ModuleState = MOD_UNFITTED;
	m_ChargeState = MOD_UNLOADED;

	//m_IsInitialCycle = true;
}

MiningLaser::~MiningLaser()
{

}

void MiningLaser::Process()
{
	m_ActiveModuleProc->Process();
}

void MiningLaser::Load(InventoryItemRef charge)
{
	ActiveModule::Load(charge);
	m_ChargeState = MOD_LOADED;
}

void MiningLaser::Unload()
{
	ActiveModule::Unload();
	m_ChargeState = MOD_UNLOADED;
}

void MiningLaser::Repair()
{

}

void MiningLaser::Overload()
{

}

void MiningLaser::DeOverload()
{

}

void MiningLaser::DestroyRig()
{

}

void MiningLaser::Activate(SystemEntity * targetEntity)
{
	// Test if module is Ice Harvester I (typeID 16728) or Ice Harvester II (typeID 22229) AND the target is in groupID 465 'Ice',
	// otherwise, just test if target is in the categoryID 25 'Asteroid' or using Gas Harvester AND the target is in groupID 711 'Harvestable Cloud'
	if( ((getItem()->typeID() == 16278 || getItem()->typeID() == 22229) && (targetEntity->Item()->groupID() == EVEDB::invGroups::Ice))
		|| (targetEntity->Item()->categoryID() == EVEDB::invCategories::Asteroid)
		|| ((targetEntity->Item()->groupID() == EVEDB::invGroups::Harvestable_Cloud) && (getItem()->groupID() == EVEDB::invGroups::Gas_Cloud_Harvester)))
	{
		// We either have a chargeless mining laser OR our charged mining laser has no charge, check groupID for charged laser types:
		uint32 typeID = getItem()->typeID();
		if( typeID == 24305 || typeID == 17912 )
		{
			// Charged mining laser:
			// + Modulated Strip Miner II
			// + Modulated Deep Core Strip Miner II
			if( m_chargeRef != NULL )
			{
				m_targetEntity = targetEntity;
				m_targetID = targetEntity->Item()->itemID();
				// Activate active processing component timer:
				m_ActiveModuleProc->ActivateCycle();
				m_ModuleState = MOD_ACTIVATED;
				//m_IsInitialCycle = true;
				_ShowCycle();
			}
			else
			{
				sLog.Error( "MiningLaser::Activate()", "ERROR: Cannot find charge that is supposed to be loaded into this module!" );
				throw PyException( MakeCustomError( "ERROR!  Cannot find charge that is supposed to be loaded into this module!" ) );
			}
		}
		else
		{
			// Non-charged mining laser:
			m_targetEntity = targetEntity;
			m_targetID = targetEntity->Item()->itemID();
			// Activate active processing component timer:
			m_ActiveModuleProc->ActivateCycle();
			m_ModuleState = MOD_ACTIVATED;
			//m_IsInitialCycle = true;
			_ShowCycle();
		}
	}
	else
	{
		sLog.Error( "MiningLaser::Activate()", "ERROR: Cannot activate mining laser on non-asteroid target!" );
		throw PyException( MakeCustomError( "ERROR!  Cannot activate mining laser on non-asteroid target!" ) );
	}
}

void MiningLaser::Deactivate() 
{
	m_ModuleState = MOD_DEACTIVATING;
	m_ActiveModuleProc->DeactivateCycle();
}

void MiningLaser::DoCycle()
{
	if( m_ActiveModuleProc->ShouldProcessActiveCycle() )
	{
		// Check to see if our target is still in this bubble or has left or been destroyed:
		if( !(m_Ship->GetOperator()->GetSystemEntity()->Bubble()->GetEntity(m_targetID)) )
		{
			// Target has left our bubble or been destroyed, deactivate this module:
			Deactivate();
			return;
		}

		_ShowCycle();

		// Actually pull in the ore
		_ProcessCycle();
	}
}

void MiningLaser::StopCycle(bool abort)
{
	if( !abort )
	{
		// On end of last cycle, make sure we pull in one more batch of ore:
		_ProcessCycle();
	}

	Notify_OnGodmaShipEffect shipEff;
	shipEff.itemID = m_Item->itemID();
	shipEff.effectID = effectMiningLaser;
	shipEff.when = Win32TimeNow();
	shipEff.start = 0;
	shipEff.active = 0;

	PyList* env = new PyList;
	env->AddItem(new PyInt(shipEff.itemID));
	env->AddItem(new PyInt(m_Ship->ownerID()));
	env->AddItem(new PyInt(m_Ship->itemID()));
	env->AddItem(new PyInt(m_targetID));
	env->AddItem(new PyNone);
	env->AddItem(new PyNone);
	env->AddItem(new PyInt(shipEff.effectID));

	shipEff.environment = env;
	shipEff.startTime = shipEff.when;
	shipEff.duration = 1.0;		//m_ActiveModuleProc->GetRemainingCycleTimeMS();		// At least, I'm assuming this is the remaining time left in the cycle
	shipEff.repeat = new PyInt(0);
	shipEff.randomSeed = new PyNone;
	shipEff.error = new PyNone;

	PyList* events = new PyList;
	events->AddItem(shipEff.Encode());

	Notify_OnMultiEvent multi;
	multi.events = events;

	PyTuple* tmp = multi.Encode();

	m_Ship->GetOperator()->SendDogmaNotification("OnMultiEvent", "clientID", &tmp);

	// Create Special Effect:
	uint32 chargeID = 0;
	if( m_chargeLoaded )
		chargeID = m_chargeRef->itemID();

	std::string effectsString;
	if (getItem()->groupID() == EVEDB::invGroups::Gas_Cloud_Harvester)
		effectsString = "effects.CloudMining";
	else
		effectsString = "effects.Mining";
	m_Ship->GetOperator()->GetDestiny()->SendSpecialEffect
	(
		m_Ship,
		m_Item->itemID(),
		m_Item->typeID(),
		m_targetID,
		chargeID,
		effectsString,
		0,
		0,
		0,
		1.0,
		0
	);

	m_ActiveModuleProc->DeactivateCycle();
}

void MiningLaser::_ProcessCycle()
{
	// Retrieve ore from target Asteroid and put into flagCargoHold
	InventoryItemRef asteroidRef = m_targetEntity->Item();
	InventoryItemRef moduleRef = this->getItem();
	uint32 remainingOreUnits = asteroidRef->GetAttribute(AttrQuantity).get_int();
	double oreUnitVolume = asteroidRef->GetAttribute(AttrVolume).get_float();
	double oreUnitsToPull = 0.0;
	double remainingCargoVolume = 0.0;

	// Calculate how many units of ore to pull from the asteroid on this cycle:
	//oreUnitsToPull = asteroidRef->GetAttribute(AttrMiningAmount).get_float() / oreUnitVolume;
	oreUnitsToPull = moduleRef->GetAttribute(AttrMiningAmount).get_float() / oreUnitVolume;
	if( m_chargeRef != NULL )
	{
		// Use mining crystal charge to multiply ore amount taken:
		if( moduleRef->HasAttribute(AttrSpecialisationAsteroidYieldMultiplier) )
			oreUnitsToPull *= moduleRef->GetAttribute(AttrSpecialisationAsteroidYieldMultiplier).get_float();
		else
		{
			sLog.Error( "MiningLaser::DoCycle()", "" );
			oreUnitsToPull = 0.0;
		}
	}
	oreUnitsToPull = floor(oreUnitsToPull);

	if( oreUnitsToPull > remainingOreUnits )
		oreUnitsToPull = remainingOreUnits;

	if( oreUnitsToPull > 0.0 )
	{
		if( remainingOreUnits > 0 )
		{
			// Verify room left in flagCargoHold
			remainingCargoVolume = m_Ship->GetRemainingVolumeByFlag(flagCargoHold);

			if( remainingCargoVolume >= oreUnitVolume )
			{
				// There's room for at least one unit of ore, let's find out how many we can put into cargo hold,
				// then make a stack for them:
				if( remainingCargoVolume < (oreUnitsToPull * oreUnitVolume) )
				{
					oreUnitsToPull =  floor(remainingCargoVolume / oreUnitVolume);
				}

				ItemData idata(
					asteroidRef->typeID(),
					m_Ship->ownerID(),
					0, //temp location
					flagCargoHold,
					oreUnitsToPull
				);

				InventoryItemRef ore = m_Ship->GetOperator()->GetDestiny()->GetSystemManager()->GetServiceMgr()->item_factory.SpawnItem( idata );
				if( ore )
				{
					//ore->Move(m_Ship->itemID(), flagCargoHold);
					m_Ship->AddItem(flagCargoHold, ore);
					// Finally, reduce the amount of ore in the asteroid by how much we took out:
					asteroidRef->SetAttribute( AttrQuantity, (remainingOreUnits - oreUnitsToPull) );
					remainingOreUnits -= oreUnitsToPull;
				}
				else
				{
					sLog.Error( "MiningLaser::DoCycle()", "ERROR: Could not create ore stack for '%s' ship (id %u)!", m_Ship->itemName().c_str(), m_Ship->itemID() );
					ore->Delete();
				}
			}
			else
			{
				// Not enough cargo space, so module should deactivate and not pull anymore ore from the asteroid

				// nothing to do here yet, it seems
			}
		}

		remainingCargoVolume = m_Ship->GetRemainingVolumeByFlag(flagCargoHold);
		if( (remainingCargoVolume < oreUnitVolume) || (remainingOreUnits == 0) )
		{
			// Asteroid is empty OR cargo hold is entirely full, either way, DEACTIVATE module immediately!
			m_ModuleState = MOD_DEACTIVATING;
			m_ActiveModuleProc->AbortCycle();
		}

		if( remainingOreUnits == 0 )
		{
			// Asteroid is empty now, so remove it
			m_targetEntity->Bubble()->Remove(m_targetEntity);
			m_targetEntity->Item()->Delete();
		}
	}
	else
		sLog.Warning( "MiningLaser::DoCycle()", "Somehow MiningLaser could not extract ore from current target asteroid '%s' (id %u)", m_targetEntity->Item()->itemName().c_str(), m_targetEntity->GetID() );
}

void MiningLaser::_ShowCycle()
{
	// Create Destiny Updates:
	Notify_OnGodmaShipEffect shipEff;
	shipEff.itemID = m_Item->itemID();
	shipEff.effectID = effectMiningLaser;		// From EVEEffectID::
	shipEff.when = Win32TimeNow();
	shipEff.start = 1;
	shipEff.active = 1;

	PyList* env = new PyList;
	env->AddItem(new PyInt(shipEff.itemID));
	env->AddItem(new PyInt(m_Ship->ownerID()));
	env->AddItem(new PyInt(m_Ship->itemID()));
	env->AddItem(new PyInt(m_targetID));
	env->AddItem(new PyNone);
	env->AddItem(new PyNone);
	env->AddItem(new PyInt(shipEff.effectID));

	shipEff.environment = env;
	shipEff.startTime = shipEff.when;
	shipEff.duration = m_Item->GetAttribute(AttrDuration).get_float();
	shipEff.repeat = new PyInt(1000);
	shipEff.randomSeed = new PyNone;
	shipEff.error = new PyNone;

	PyTuple* tmp = new PyTuple(3);
	//tmp->SetItem(1, dmgMsg.Encode());
	tmp->SetItem(1, shipEff.Encode());

	std::vector<PyTuple*> events;
	//events.push_back(dmgMsg.Encode());
	events.push_back(shipEff.Encode());

	std::vector<PyTuple*> updates;
	//updates.push_back(dmgChange.Encode());

	m_Ship->GetOperator()->GetDestiny()->SendDestinyUpdate(updates, events, false);

	// Create Special Effect:
	uint32 chargeID = 0;
	if( m_chargeLoaded )
		chargeID = m_chargeRef->itemID();

	std::string effectsString;
	if (getItem()->groupID() == EVEDB::invGroups::Gas_Cloud_Harvester)
		effectsString = "effects.CloudMining";
	else
		effectsString = "effects.Mining";
	m_Ship->GetOperator()->GetDestiny()->SendSpecialEffect
	(
		m_Ship,
		m_Item->itemID(),
		m_Item->typeID(),
		m_targetID,
		chargeID,
		effectsString,
		0,
		1,
		1,
		m_Item->GetAttribute(AttrDuration).get_float(),
		1000
	);
}