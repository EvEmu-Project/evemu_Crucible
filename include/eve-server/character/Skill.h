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

#ifndef __SKILL__H__INCL__
#define __SKILL__H__INCL__

#include "inventory/InventoryItem.h"

class Character;

/**
 * \class Skill
 *
 * @brief InventoryItem for skill.
 *
 * Skill class where all skill stuff is placed.
 *
 * @author Bloody.Rabbit
 * @date June 2009
 */
class Skill
: public InventoryItem
{
	friend class InventoryItem;
public:
	/**
	 * Loads skill.
	 *
	 * @param[in] factory
	 * @param[in] skillID ID of skill to load.
	 * @return Pointer to new Skill object; NULL if fails.
	 */
	static SkillRef Load(ItemFactory &factory, uint32 skillID);
	/**
	 * Spawns new skill.
	 *
	 * @param[in] factory
	 * @param[in] data Item data of new skill.
	 * @return Pointer to new Skill object; NULL if fails.
	 */
	static SkillRef Spawn(ItemFactory &factory, ItemData &data);

	/**
	 * Calculates required amount of skillpoints for level.
	 *
	 * @param[in] level Level to calculate SP for.
	 * @return Amount of SP required.
	 */
	EvilNumber GetSPForLevel(EvilNumber level);
	/**
	 * Checks whether requirements of skill has been fulfilled.
	 *
	 * @param[in] ch Character which is checked.
	 * @return True if requirements are OK, false if not.
	 */
	bool SkillPrereqsComplete(Character &ch);

	/**
	 *Performs check on fitting items
	 */
	static bool FitModuleSkillCheck(InventoryItemRef item, CharacterRef character);

protected:
	Skill(
		ItemFactory &_factory,
		uint32 _skillID,
		// InventoryItem stuff:
		const ItemType &_type,
		const ItemData &_data );

	/*
	 * Member functions
	 */
	using InventoryItem::_Load;

	// Template loader:
	template<class _Ty>
	static RefPtr<_Ty> _LoadItem(ItemFactory &factory, uint32 skillID,
		// InventoryItem stuff:
		const ItemType &type, const ItemData &data)
	{
		// check it's a skill
		if( type.categoryID() != EVEDB::invCategories::Skill )
		{
			sLog.Error("Skill", "Trying to load %s as Skill.", type.category().name().c_str() );
			return RefPtr<_Ty>();
		}

		// no additional stuff

		return _Ty::template _LoadSkill<_Ty>( factory, skillID, type, data );
	}

	// Actual loading stuff:
	template<class _Ty>
	static RefPtr<_Ty> _LoadSkill(ItemFactory &factory, uint32 skillID,
		// InventoryItem stuff:
		const ItemType &type, const ItemData &data
	);

	static uint32 _Spawn(ItemFactory &factory,
		// InventoryItem stuff:
		ItemData &data
	);
};

#endif /* !__SKILL__H__INCL__ */

