/*
	------------------------------------------------------------------------------------
	LICENSE:
	------------------------------------------------------------------------------------
	This file is part of EVEmu: EVE Online Server Emulator
	Copyright 2006 - 2008 The EVEmu Team
	For the latest information visit http://evemu.mmoforge.org
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
	Author:		Zhur
*/
#ifndef __DAMAGE_H_INCL__
#define __DAMAGE_H_INCL__

#include "../common/packet_types.h"

class InventoryItem;
class SystemEntity;

class Damage {
public:
	Damage(double _kinetic,
		double _thermal,
		double _em,
		double _explosive,
		SystemEntity *_source,
		InventoryItem *_weapon,
		EVEEffectID _effect);
	Damage(InventoryItem *_weapon,	//damage derrived directly from weapon.
		SystemEntity *_source,
		EVEEffectID _effect
		);
	Damage(InventoryItem *_weapon,	//damage derrived directly from weapon+charge
		InventoryItem *_charge,
		SystemEntity *_source,
		EVEEffectID _effect
		);
	  
	~Damage();
	double GetTotal() const { return(kinetic+thermal+em+explosive); }
	Damage MultiplyDup(double _kinetic_multiplier,
					 double _thermal_multiplier,
					 double _em_multiplier,
					 double _explosive_multiplier) const;
	void ReduceTo(double total_amount);
	Damage &operator *=(double factor);
	
	double kinetic;
	double thermal;
	double em;
	double explosive;
	SystemEntity *const  source;	//we do not own this.
	InventoryItem *const weapon;	//we own a ref to this.
	InventoryItem *const charge;	//we own a ref to this. May be null.
	const EVEEffectID    effect;
};


inline Damage Damage::MultiplyDup(
						 double _kinetic,
				 double _thermal,
				 double _em,
						 double _explosive) const {
	return( Damage(
			 kinetic * _kinetic,
			 thermal * _thermal,
			 em * _em,
			 explosive * _explosive,
				  source, weapon, effect ) );
}


inline void Damage::ReduceTo(double total_amount) {
	double t = GetTotal();
	double factor = total_amount / t;
	*this *= factor;
}

inline Damage &Damage::operator *=(double factor) {
	kinetic *= factor;
	thermal *= factor;
	em *= factor;
	explosive *= factor;
	return(*this);
}



#endif




