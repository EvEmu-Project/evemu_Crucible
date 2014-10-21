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
    Author:        AknorJaden
*/

#include "eve-server.h"

#include "EntityList.h"
#include "system/SystemBubble.h"
#include "system/Damage.h"
#include "ship/modules/weapon_modules/MissileLauncher.h"

MissileLauncher::MissileLauncher( InventoryItemRef item, ShipRef ship )
{
    m_Item = item;
    m_Ship = ship;
    m_Effects = new ModuleEffects(m_Item->typeID());
    m_ShipAttrComp = new ModifyShipAttributesComponent(this, ship);
	m_ActiveModuleProc = new ActiveModuleProcessingComponent(item, this, ship, m_ShipAttrComp);

	m_chargeRef = InventoryItemRef();		// Ensure ref is NULL
	m_chargeLoaded = false;
}

MissileLauncher::~MissileLauncher()
{

}

void MissileLauncher::Process()
{
	m_ActiveModuleProc->Process();
}

void MissileLauncher::Load(InventoryItemRef charge)
{
	ActiveModule::Load(charge);
}

void MissileLauncher::Unload()
{
	ActiveModule::Unload();
}

void MissileLauncher::Repair()
{

}

void MissileLauncher::Overload()
{

}

void MissileLauncher::DeOverload()
{

}

void MissileLauncher::DestroyRig()
{

}

void MissileLauncher::Activate(SystemEntity * targetEntity)
{
	if( this->m_chargeRef != NULL )
	{
		m_targetEntity = targetEntity;
		m_targetID = targetEntity->Item()->itemID();

		// Activate active processing component timer:
		m_ActiveModuleProc->ActivateCycle();
	}
	else
	{
		sLog.Error( "MissileLauncher::Activate()", "ERROR: Cannot find charge that is supposed to be loaded into this module!" );
		throw PyException( MakeCustomError( "ERROR!  Cannot find charge that is supposed to be loaded into this module!" ) );
	}
}

void MissileLauncher::Deactivate() 
{
	m_ModuleState = MOD_DEACTIVATING;
	m_ActiveModuleProc->DeactivateCycle();
}

void MissileLauncher::StopCycle(bool abort)
{
	// Do one last cycle:
	DoCycle();

	Notify_OnGodmaShipEffect shipEff;
	shipEff.itemID = m_Item->itemID();
	shipEff.effectID = effectMissileLaunching;
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
	m_Ship->GetOperator()->GetDestiny()->SendSpecialEffect
	(
		m_Ship,
		m_Item->itemID(),
		m_Item->typeID(),
		m_targetID,
		m_chargeRef->itemID(),
		"effects.MissileDeployment",
		1,
		0,
		0,
		1.0,
		0
	);

	m_ActiveModuleProc->DeactivateCycle();
}

void MissileLauncher::DoCycle()
{
	if( m_ActiveModuleProc->ShouldProcessActiveCycle() )
	{
		// Check to see if our target is still in this bubble or has left or been destroyed:
		if( m_Ship->GetOperator()->GetSystemEntity()->Bubble() == NULL )
		{
			// Target has left our bubble or been destroyed, deactivate this module:
			Deactivate();
			return;
		}
		else
		{
			if( !(m_Ship->GetOperator()->GetSystemEntity()->Bubble()->GetEntity(m_targetID)) )
			{
				// Target has left our bubble or been destroyed, deactivate this module:
				Deactivate();
				return;
			}
		}

		if( m_chargeRef->quantity() == 0 )
		{
			Deactivate();
			return;
		}

		_ShowCycle();

		// Create Damage action:
		//Damage( SystemEntity *_source,
        //    InventoryItemRef _weapon,
        //    double _kinetic,
        //    double _thermal,
        //    double _em,
        //    double _explosive,
        //    EVEEffectID _effect );
		double kinetic_damage = 0.0;
		double thermal_damage = 0.0;
		double em_damage = 0.0;
		double explosive_damage = 0.0;

		// This still somehow needs skill, ship, module, and implant bonuses to be applied:
		// This still somehow needs to have optimal range and falloff attributes applied as a damage modification factor:
		if( m_chargeRef->HasAttribute(AttrKineticDamage) )
			kinetic_damage = (m_Item->GetAttribute(AttrDamageMultiplier) * m_chargeRef->GetAttribute(AttrKineticDamage)).get_float();
		if( m_chargeRef->HasAttribute(AttrThermalDamage) )
			thermal_damage = (m_Item->GetAttribute(AttrDamageMultiplier) * m_chargeRef->GetAttribute(AttrThermalDamage)).get_float();
		if( m_chargeRef->HasAttribute(AttrEmDamage) )
			em_damage = (m_Item->GetAttribute(AttrDamageMultiplier) * m_chargeRef->GetAttribute(AttrEmDamage)).get_float();
		if( m_chargeRef->HasAttribute(AttrExplosiveDamage) )
			explosive_damage = (m_Item->GetAttribute(AttrDamageMultiplier) * m_chargeRef->GetAttribute(AttrExplosiveDamage)).get_float();

		Damage damageDealt
		(
			m_Ship->GetOperator()->GetSystemEntity(),
			m_Item,
			kinetic_damage,			// kinetic damage
			thermal_damage,			// thermal damage
			em_damage,				// em damage
			explosive_damage,		// explosive damage
			effectMissileLaunching	// from EVEEffectID::
		);
		
		m_targetEntity->ApplyDamage( damageDealt );

		// Reduce ammo charge by 1 unit:
		m_chargeRef->SetQuantity(m_chargeRef->quantity() - 1);
	}
}

void MissileLauncher::_ShowCycle()
{
	// Create Destiny Updates:
	Notify_OnGodmaShipEffect shipEff;
	shipEff.itemID = m_Item->itemID();
	shipEff.effectID = effectMissileLaunching;		// From EVEEffectID::
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
	shipEff.duration = m_Item->GetAttribute(AttrSpeed).get_float();
	shipEff.repeat = new PyInt(1000);
	shipEff.randomSeed = new PyNone;
	shipEff.error = new PyNone;

	PyTuple* tmp = new PyTuple(3);
	//tmp->SetItem(1, dmgMsg.Encode());
	tmp->SetItem(2, shipEff.Encode());

	std::vector<PyTuple*> events;
	//events.push_back(dmgMsg.Encode());
	events.push_back(shipEff.Encode());

	std::vector<PyTuple*> updates;
	//updates.push_back(dmgChange.Encode());

	m_Ship->GetOperator()->GetDestiny()->SendDestinyUpdate(updates, events, false);

	// Create Special Effect:
	m_Ship->GetOperator()->GetDestiny()->SendSpecialEffect
	(
		m_Ship,
		m_Item->itemID(),
		m_Item->typeID(),
		m_targetID,
		m_chargeRef->typeID(),
		"effects.MissileDeployment",
		1,
		1,
		1,
		m_Item->GetAttribute(AttrSpeed).get_float(),
		1000
	);

	// Actually Launch a missile, creating a new Destiny object for it, but we don't know how to do this yet...

	// TODO - RESEARCH
	// (packets sent by server upon missile launch)
	/*
	Packet #1
[PyObjectData Name: macho.Notification]
  [PyTuple 7 items]
    [PyInt 12]
    [PyObjectData Name: macho.MachoAddress]
      [PyTuple 4 items]
        [PyInt 1]
        [PyInt 790408]
        [PyNone]
        [PyNone]
    [PyObjectData Name: macho.MachoAddress]
      [PyTuple 4 items]
        [PyInt 4]
        [PyString "DoDestinyUpdate"]
        [PyList 0 items]
        [PyString "clientID"]
    [PyInt 5894042]
    [PyTuple 1 items]
      [PyTuple 2 items]
        [PyInt 0]
        [PySubStream 888 bytes]
          [PyTuple 2 items]
            [PyInt 0]
            [PyTuple 2 items]
              [PyInt 1]
              [PyTuple 3 items]
                [PyList 3 items]
                  [PyTuple 2 items]
                    [PyInt 62665]
                    [PyTuple 2 items]
                      [PyString "PackagedAction"]
                      [PySubStream 270 bytes]
                        [PyList 1 items]
                          [PyTuple 2 items]
                            [PyInt 62665]
                            [PyTuple 2 items]
                              [PyString "AddBalls2"]
                              [PyTuple 1 items]
                                [PyTuple 2 items]
                                  [PyString "??  ?+??Pl?|  ?C????8?B)??u3??As"?E??B     ??@ ????????7)a[?? ?E                          zD  ???"]
                                  [PyList 1 items]
                                    [PyDict 11 kvp]
                                      [PyString "itemID"]
                                      [PyIntegerVar missileEntityID]
                                      [PyString "typeID"]
                                      [PyInt 203]
                                      [PyString "name"]
                                      [PyString "Cruise Missile"]
                                      [PyString "sourceModuleID"]
                                      [PyIntegerVar moduleID]		- the missile launcher module
                                      [PyString "corpID"]
                                      [PyInt corpID]
                                      [PyString "allianceID"]
                                      [PyInt allianceID]
                                      [PyString "warFactionID"]
                                      [PyNone]						- would by PyInt with warFactionID
                                      [PyString "securityStatus"]
                                      [PyFloat 2.65811580082965]
                                      [PyString "ownerID"]
                                      [PyInt ownerID]				- this is probably characterID
                                      [PyString "numLaunchers"]
                                      [PyInt 1]
                                      [PyString "nameID"]
                                      [PyInt 63844]
                  [PyTuple 2 items]
                    [PyInt 62665]
                    [PyTuple 2 items]
                      [PyString "OnDamageStateChange"]
                      [PyTuple 2 items]
                        [PyIntegerVar xxxxxx]
                        [PyList 3 items]
                          [PyTuple 3 items]
                            [PyFloat 0.570742342960805]
                            [PyFloat 200000]
                            [PyIntegerVar xxxxxx]
                          [PyFloat 1]
                          [PyFloat 1]
                  [PyTuple 2 items]
                    [PyInt 62665]
                    [PyTuple 2 items]
                      [PyString "OnDamageStateChange"]
                      [PyTuple 2 items]
                        [PyIntegerVar xxxxxx]
                        [PyList 3 items]
                          [PyTuple 3 items]
                            [PyFloat 6.93745520230552E-06]
                            [PyFloat 425000]
                            [PyIntegerVar timeStamp]
                          [PyFloat 0.810598232437212]
                          [PyFloat 1]
                [PyBool True]
                [PyList 10 items]
                  [PyTuple 7 items]
                    [PyString "OnModuleAttributeChange"]
                    [PyInt xxxxxx]
                    [PyIntegerVar xxxxxx]
                    [PyInt 266]
                    [PyIntegerVar xxxxxx]
                    [PyFloat 3320.81907103188]
                    [PyFloat 3640.81907103188]
                  [PyTuple 7 items]
                    [PyString "OnModuleAttributeChange"]
                    [PyInt xxxxxx]
                    [PyIntegerVar xxxxxx]
                    [PyInt 18]
                    [PyIntegerVar timeStamp]
                    [PyFloat 4077.04701492647]
                    [PyList 4 items]
                      [PyFloat 4077.04701492647]
                      [PyIntegerVar timeStamp]
                      [PyFloat 104400]
                      [PyFloat 4860]
                  [PyTuple 5 items]
                    [PyString "OnEffectHit"]
                    [PyIntegerVar xxxxxx]
                    [PyInt 10]
                    [PyIntegerVar xxxxxx]
                    [PyFloat 42.3334296258933]
                  [PyList 3 items]
                    [PyString "OnDamageMessage"]
                    [PyString "AttackHit2"]
                    [PyDict 4 kvp]
                      [PyString "weapon"]
                      [PyInt 21638]
                      [PyString "splash"]
                      [PyString ""]
                      [PyString "target"]
                      [PyIntegerVar xxxxxx]
                      [PyString "damage"]
                      [PyFloat 42.3334296258933]
                  [PyTuple 5 items]
                    [PyString "OnEffectHit"]
                    [PyIntegerVar xxxxxx]
                    [PyInt 10]
                    [PyIntegerVar xxxxxx]
                    [PyFloat 59.3261381082436]
                  [PyList 3 items]
                    [PyString "OnDamageMessage"]
                    [PyString "AttackHit3"]
                    [PyDict 4 kvp]
                      [PyString "weapon"]
                      [PyInt 21638]
                      [PyString "splash"]
                      [PyString ""]
                      [PyString "target"]
                      [PyIntegerVar xxxxxx]
                      [PyString "damage"]
                      [PyFloat 59.3261381082436]
                  [PyTuple 5 items]
                    [PyString "OnEffectHit"]
                    [PyIntegerVar xxxxxx]
                    [PyInt 10]
                    [PyIntegerVar xxxxxx]
                    [PyFloat 69.8373107175934]
                  [PyList 3 items]
                    [PyString "OnDamageMessage"]
                    [PyString "AttackHit4"]
                    [PyDict 4 kvp]
                      [PyString "weapon"]
                      [PyInt 21638]
                      [PyString "splash"]
                      [PyString ""]
                      [PyString "target"]
                      [PyIntegerVar xxxxxx]
                      [PyString "damage"]
                      [PyFloat 69.8373107175934]
                  [PyTuple 7 items]
                    [PyString "OnModuleAttributeChange"]
                    [PyInt 649670823]
                    [PyTuple 3 items]
                      [PyIntegerVar xxxxxx]
                      [PyInt 31]
                      [PyInt 203]
                    [PyInt 805]
                    [PyIntegerVar timeStamp]
                    [PyInt 23]
                    [PyInt 24]
                  [PyTuple 12 items]
                    [PyString "OnGodmaShipEffect"]
                    [PyIntegerVar xxxxxx]
                    [PyInt 101]
                    [PyIntegerVar timeStamp]
                    [PyInt 1]
                    [PyInt 1]
                    [PyList 7 items]
                      [PyIntegerVar xxxxxx]
                      [PyIntegerVar xxxxxx]
                      [PyIntegerVar xxxxxx]
                      [PyIntegerVar xxxxxx]
                      [PyTuple 3 items]
                        [PyIntegerVar xxxxxx]
                        [PyInt 31]
                        [PyInt 203]
                      [PyList 0 items]
                      [PyInt 101]
                    [PyIntegerVar timeStamp]
                    [PyFloat 10454.4]
                    [PyInt 1000]
                    [PyNone]
                    [PyNone]
    [PyNone]
    [PyNone]


	Packet #2
		[PyObjectData Name: macho.Notification]
		  [PyTuple 7 items]
			[PyInt 12]
			[PyObjectData Name: macho.MachoAddress]
			  [PyTuple 4 items]
				[PyInt 1]
				[PyInt 790408]
				[PyNone]
				[PyNone]
			[PyObjectData Name: macho.MachoAddress]
			  [PyTuple 4 items]
				[PyInt 4]
				[PyString "DoDestinyUpdate"]
				[PyList 0 items]
				[PyString "clientID"]
			[PyInt 5894042]
			[PyTuple 1 items]
			  [PyTuple 2 items]
				[PyInt 0]
				[PySubStream 145 bytes]
				  [PyTuple 2 items]
					[PyInt 0]
					[PyTuple 2 items]
					  [PyInt 1]
					  [PyTuple 2 items]
						[PyList 2 items]
						  [PyTuple 2 items]
							[PyInt 62665]
							[PyTuple 2 items]
							  [PyString "OnSpecialFX"]
							  [PyTuple 14 items]
								[PyIntegerVar shipID]
								[PyIntegerVar moduleID]
								[PyInt 16519]
								[PyIntegerVar targetID]
								[PyInt 203]
								[PyList 0 items]
								[PyString "effects.MissileDeployment"]
								[PyBool True]
								[PyInt 1]
								[PyInt 1]
								[PyFloat 10454.4]
								[PyInt 1000]
								[PyIntegerVar timeStamp]
								[PyNone]
						  [PyTuple 2 items]
							[PyInt 62665]
							[PyTuple 2 items]
							  [PyString "LaunchMissile"]
							  [PyTuple 5 items]
								[PyIntegerVar missileEntityID]
								[PyIntegerVar targetID]
								[PyIntegerVar shipID]
								[PyInt 1]
								[PyInt 1]
						[PyBool False]
			[PyNone]
			[PyNone]
	*/
}
