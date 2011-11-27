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
	Author:		Bloody.Rabbit
*/

#include "EVEServerPCH.h"

/*
 * Skill
 */
Skill::Skill(
	ItemFactory &_factory,
	uint32 _skillID,
	// InventoryItem stuff:
	const ItemType &_type,
	const ItemData &_data )
: InventoryItem(_factory, _skillID, _type, _data)
{
}

SkillRef Skill::Load(ItemFactory &factory, uint32 skillID)
{
	return InventoryItem::Load<Skill>( factory, skillID );
}

template<class _Ty>
RefPtr<_Ty> Skill::_LoadSkill(ItemFactory &factory, uint32 skillID,
	// InventoryItem stuff:
	const ItemType &type, const ItemData &data)
{
	return SkillRef( new Skill( factory, skillID, type, data ) );
}

SkillRef Skill::Spawn(ItemFactory &factory, ItemData &data)
{
	uint32 skillID = _Spawn( factory, data );
	if( skillID == 0 )
		return SkillRef();

    SkillRef skillRef = Skill::Load( factory, skillID );

    skillRef->SetAttribute(AttrIsOnline, 1);      // Is Online

    return skillRef;
}

uint32 Skill::_Spawn(ItemFactory &factory, ItemData &data)
{
	// check it's a skill
	const ItemType *type = factory.GetType( data.typeID );
	if( type == NULL )
		return 0;

	if( type->categoryID() != EVEDB::invCategories::Skill )
	{
		_log( ITEM__ERROR, "Trying to spawn %s as Skill.", type->category().name().c_str() );
		return 0;
	}

	// spawn item, nothing else
	return InventoryItem::_Spawn( factory, data );
}

//uint32 Skill::GetSPForLevel(uint8 level)
//{
//	return(SKILL_BASE_POINTS * skillTimeConstant() * pow(32, (level - 1) / 2.0));
//}

EvilNumber Skill::GetSPForLevel( EvilNumber level )
{
    //return EVIL_SKILL_BASE_POINTS * GetAttribute(AttrSkillTimeConstant) * e_pow(32, (level - 1) / 2.0);
    return EVIL_SKILL_BASE_POINTS * GetAttribute(AttrSkillTimeConstant) * e_pow(2, (2.5*(level - 1)));
}

bool Skill::SkillPrereqsComplete(Character &ch)
{
	SkillRef requiredSkill;

    
    if( GetAttribute(AttrRequiredSkill1).get_int() != 0 )
	{
		requiredSkill = ch.GetSkill(static_cast<uint32>(GetAttribute(AttrRequiredSkill1).get_int()));
		if( !requiredSkill )
			return false;

		if( GetAttribute(AttrRequiredSkill1Level) > requiredSkill->GetAttribute(AttrSkillLevel) )
			return false;
	}

	if( GetAttribute(AttrRequiredSkill2).get_int() != 0 )
	{
		requiredSkill = ch.GetSkill(static_cast<uint32>(GetAttribute(AttrRequiredSkill2).get_int()));
		if( !requiredSkill )
			return false;

		if( GetAttribute(AttrRequiredSkill2Level) > requiredSkill->GetAttribute(AttrSkillLevel) )
			return false;
	}

    if( GetAttribute(AttrRequiredSkill3).get_int() != 0 )
    {
        requiredSkill = ch.GetSkill(static_cast<uint32>(GetAttribute(AttrRequiredSkill3).get_int()));
        if( !requiredSkill )
            return false;

        if( GetAttribute(AttrRequiredSkill3Level) > requiredSkill->GetAttribute(AttrSkillLevel) )
            return false;
    }

	return true;
}

bool Skill::FitModuleSkillCheck(InventoryItemRef item, CharacterRef character)
{	
	//TODO: move to skills
	SkillRef requiredSkill;
	
	//Primary Skill
	if(item->GetAttribute(AttrRequiredSkill1) != 0)
	{
		requiredSkill = character->GetSkill( static_cast<uint32>(item->GetAttribute(AttrRequiredSkill1).get_int()) );
		if( !requiredSkill )
			return false;

		if( item->GetAttribute(AttrRequiredSkill1Level) > requiredSkill->GetAttribute(AttrSkillLevel) )
			return false;
	}

	//Secondary Skill
    if(item->GetAttribute(AttrRequiredSkill2) != 0)
    {
        requiredSkill = character->GetSkill( static_cast<uint32>(item->GetAttribute(AttrRequiredSkill2).get_int()) );
        if( !requiredSkill )
            return false;

        if( item->GetAttribute(AttrRequiredSkill2Level) > requiredSkill->GetAttribute(AttrSkillLevel) )
            return false;
    }
	
	//Tertiary Skill
    if(item->GetAttribute(AttrRequiredSkill3) != 0)
    {
        requiredSkill = character->GetSkill( static_cast<uint32>(item->GetAttribute(AttrRequiredSkill3).get_int()) );
        if( !requiredSkill )
            return false;

        if( item->GetAttribute(AttrRequiredSkill3Level) > requiredSkill->GetAttribute(AttrSkillLevel) )
            return false;
    }
	
	//Quarternary Skill
    if(item->GetAttribute(AttrRequiredSkill4) != 0)
    {
        requiredSkill = character->GetSkill( static_cast<uint32>(item->GetAttribute(AttrRequiredSkill4).get_int()) );
        if( !requiredSkill )
            return false;

        if( item->GetAttribute(AttrRequiredSkill4Level) > requiredSkill->GetAttribute(AttrSkillLevel) )
            return false;
    }
	
	//Quinary Skill
    if(item->GetAttribute(AttrRequiredSkill5) != 0)
    {
        requiredSkill = character->GetSkill(static_cast<uint32>(item->GetAttribute(AttrRequiredSkill5).get_int()));
        if( !requiredSkill )
            return false;

        if( item->GetAttribute(AttrRequiredSkill5Level) > requiredSkill->GetAttribute(AttrSkillLevel) )
            return false;
    }
	
	//Senary Skill
    if(item->GetAttribute(AttrRequiredSkill6) != 0)
    {
        requiredSkill = character->GetSkill(static_cast<uint32>(item->GetAttribute(AttrRequiredSkill6).get_int()));
        if( !requiredSkill )
            return false;

        if( item->GetAttribute(AttrRequiredSkill6Level) > requiredSkill->GetAttribute(AttrSkillLevel) )
            return false;
    }

	return true;
}




