/*  EVEmu: EVE Online Server Emulator

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY except by those people which sell it, which
  are required to give you total support for your newly bought product;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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




