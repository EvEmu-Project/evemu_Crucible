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
	Author:		Zhur
*/

#ifndef __ATTRIBUTE_MGR__H__INCL__
#define __ATTRIBUTE_MGR__H__INCL__

/*
 * This file contains generic definitions of attribute managers.
 */

/*
 * The most basic attribute manager, supports default values
 */
template<class _Int, class _Real>
class AttributeMgr {
public:
	/*
	 * helper typedefs
	 */
	typedef _Int		int_t;
	typedef _Real		real_t;

	/*
	 * Attribute definition
	 */
	enum Attr {
		#define ATTR(ID, name, default_value, persistent) \
			Attr_##name = ID,
		#include "inventory/EVEAttributes.h"
		Invalid_Attr
	};

	/*
	 * Checks if value stored is integer or not
     * @note totally useless function because a attribute doesn't have a single type.
	 */
	bool IsInt(Attr attr) const
    {
		real_t v = GetReal(attr);
		return _IsInt(v);
	}

	/*
	 * Get attribute value as integer
	 * just round the real
	 */
	virtual int_t GetInt(Attr attr) const 
    {
		return (int_t)GetReal(attr);
	}
	/*
	 * Get attribute value as real
	 */
	virtual real_t GetReal(Attr attr) const {
		real_t v;
		if(!_Get(attr, v))
			v = GetDefault(attr);
		return(v);
	}

	/*
	 * Set attribute value to integer
	 */
	virtual void SetInt(Attr attr, const int_t &v) {
		
        // delete the real value (no matter if there really is any)
		m_reals.erase(attr);
		// set the value
		m_ints[attr] = v;
	}
	/*
	 * Set attribute value to real
	 */
	virtual void SetReal(Attr attr, const real_t &v) {

        /* check if we need to store it as a integer or as a float */
		if(_IsInt(v) == true)
        {
			SetInt(attr, v);
		}
        else
        {
			// delete the integer value (no matter if there really is any)
			m_ints.erase(attr);
			
			m_reals[attr] = v;
		}
	}

	/*
	 * Clears the attribute value
	 */
	virtual void Clear(Attr attr) {
		// delete the integer value (no matter if there really is any)
		m_ints.erase(attr);
		// delete the real value (no matter if there really is any)
		m_reals.erase(attr);
	}

	/*
	 * Complete deletion of all contents
	 */
	virtual void Delete() {
		Attr attr;
		// clear integers first
		{
			typename std::map<Attr, int_t>::const_iterator cur, end;
			cur = m_ints.begin();
			end = m_ints.end();
			while(cur != end) {
				attr = cur->first;
				cur++;
				Clear(attr);
			}
		}
		// then clear reals
		{
			typename std::map<Attr, real_t>::const_iterator cur, end;
			cur = m_reals.begin();
			end = m_reals.end();
			while(cur != end) {
				attr = cur->first;
				cur++;
				Clear(attr);
			}
		}
	}

	/*
	 * Attribute access by name
	 */
	#define ATTRI(ID, name, default_value, persistent) \
		inline int_t name() const { \
			return(GetInt(Attr_##name)); \
		} \
		inline void Set_##name(const int_t &v) { \
			SetInt(Attr_##name, v); \
		} \
		inline void Clear_##name() { \
			Clear(Attr_##name); \
		}
	#define ATTRD(ID, name, default_value, persistent) \
		inline real_t name() const { \
			return(GetReal(Attr_##name)); \
		} \
		inline void Set_##name(const real_t &v) { \
			SetReal(Attr_##name, v); \
		} \
		inline void Clear_##name() { \
			Clear(Attr_##name); \
		}
	#include "inventory/EVEAttributes.h"

	/*
	 * Returns default value of attribute
	 */
	static int_t GetDefault(Attr attr) {
		_LoadDefault();
		return(m_default[attr]);
	}

protected:
	/*
	 * Retrieves the attribute value
	 * - returns true on success
	 * - returns false if not found
	 */
	bool _Get(Attr attr, real_t &into) const {
		// try real value
		typename std::map<Attr, real_t>::const_iterator ri = m_reals.find(attr);
		if(ri != m_reals.end()) {
			// found real value
			into = ri->second;
			return true;
		}

		// real not found, try integer
		typename std::map<Attr, int_t>::const_iterator ii = m_ints.find(attr);
		if(ii != m_ints.end()) {
			// found integer value
			into = ii->second;
			return true;
		}

		// nothing at all, return false
		return false;
	}

	/*
	 * Checks if real given can be rounded to integer without looses
	 */
	static bool _IsInt(const real_t &v) {
		return(v == int_t(v));
	}

	/*
	 * Store the actual attribute values
	 */
	std::map<Attr, int_t> m_ints;
	std::map<Attr, real_t> m_reals;

	/*
	 * Default value stuff
	 */
	static void _LoadDefault() {
		if(!m_defaultLoaded) {
			memset(m_default, 0, sizeof(m_default));

			#define ATTR(ID, name, default_value, persistent) \
				m_default[Attr_##name] = default_value;
			#include "inventory/EVEAttributes.h"

			m_defaultLoaded = true;
		}
	}

	static bool m_defaultLoaded;
	static int_t m_default[Invalid_Attr];
};

template<class _Int, class _Real>
bool AttributeMgr<_Int, _Real>::m_defaultLoaded = false;

template<class _Int, class _Real>
typename AttributeMgr<_Int, _Real>::int_t
	AttributeMgr<_Int, _Real>::m_default[AttributeMgr<_Int, _Real>::Invalid_Attr];

/*
 * Attribute manager which supports rechargable attributes
 */
template<class _Int, class _Real>
class AdvancedAttributeMgr
: virtual public AttributeMgr<_Int, _Real>
{
public:
	/*
 	 * Access to our parent
	 */
	typedef AttributeMgr<_Int, _Real>	_Base;

	typedef typename _Base::Attr		Attr;
	typedef typename _Base::int_t		int_t;
	typedef typename _Base::real_t		real_t;

	/*
	 * Constructor
	 */
	AdvancedAttributeMgr() {
		_LoadTauCap();

		typename std::map<Attr, TauCap>::const_iterator cur, end;
		cur = m_tauCap.begin();
		end = m_tauCap.end();
		for(; cur != end; cur++)
			_SetLastChange(cur->first, Win32TimeNow());
	}

	/*
	 * Common functions overloads
	 */
	real_t GetReal(Attr attr) const
    {
		real_t v = _Base::GetReal(attr);
		_CalcTauCap(attr, v);
		return(v);
	}

	void SetReal(Attr attr, const real_t &v) {
		_Base::SetReal(attr, v);
		if(IsRechargable(attr))
			_SetLastChange(attr, Win32TimeNow());
	}
	void SetInt(Attr attr, const int_t &v) {
		_Base::SetInt(attr, v);
		if(IsRechargable(attr))
			_SetLastChange(attr, Win32TimeNow());
	}

	void Clear(Attr attr) {
		_Base::Clear(attr);
		if(IsRechargable(attr))
			_SetLastChange(attr, Win32TimeNow());
	}

	/*
	 * Checks whether given attribute is rechargable
	 */
	static bool IsRechargable(Attr attr) {
		_LoadTauCap();
		return(m_tauCap.find(attr) != m_tauCap.end());
	}

protected:
	/*
	 * recharge calculation
	 */
	void _CalcTauCap(Attr attr, real_t &v) const {
		_LoadTauCap();

		/*
		def GetChargeValue(self, oldVal, oldTime, tau, Ec):
			if (Ec == 0):
				return 0
			newTime = blue.os.GetTime(1)
			sq = math.sqrt((oldVal / Ec))
			timePassed = ((oldTime - newTime) / float(const.dgmTauConstant))
			exp = math.exp((timePassed / tau))
			ret = (((1.0 + ((sq - 1.0) * exp)) ** 2) * Ec)
			return ret
		*/

		typename std::map<Attr, TauCap>::const_iterator i = m_tauCap.find(attr);
		if(i != m_tauCap.end()) {
			real_t cap = GetReal(i->second.cap);
			if(cap == 0) {
				v = 0;
				return;
			}
			real_t tau = GetReal(i->second.tau) / 5.0;

			real_t sq = sqrt(v / cap);
			int64 time = int64(_GetLastChange(attr) - Win32TimeNow()) / 10000/* = dgmTauConstant */; 
			real_t e = exp(time / tau);

			v = pow(1.0 + (sq - 1.0) * e, 2.0) * cap;
		}
	}

	/*
	 * last change stuff
	 */
	uint64 _GetLastChange(Attr attr) const {
		typename std::map<Attr, uint64>::const_iterator res = m_lastChange.find(attr);
		if(res != m_lastChange.end())
			return(res->second);
		return(Win32TimeNow());
	}
	void _SetLastChange(Attr attr, const uint64 &time) {
		m_lastChange[attr] = time;
	}

	std::map<Attr, uint64> m_lastChange;

	/*
	 * tau/cap stuff
	 */
	static void _LoadTauCap() {
		if(!m_tauCapLoaded) {
			// hacking for now
			m_tauCap[_Base::Attr_charge].tau = _Base::Attr_rechargeRate;
			m_tauCap[_Base::Attr_charge].cap = _Base::Attr_capacitorCapacity;

			m_tauCap[_Base::Attr_shieldCharge].tau = _Base::Attr_shieldRechargeRate;
			m_tauCap[_Base::Attr_shieldCharge].cap = _Base::Attr_shieldCapacity;

			m_tauCapLoaded = true;
		}
	}

	static bool m_tauCapLoaded;
	struct TauCap {
		Attr tau, cap;
	};
	static std::map<Attr, TauCap> m_tauCap;
};

template<class _Int, class _Real>
bool AdvancedAttributeMgr<_Int, _Real>::m_tauCapLoaded = false;

template<class _Int, class _Real>
std::map<
	typename AdvancedAttributeMgr<_Int, _Real>::Attr,
	typename AdvancedAttributeMgr<_Int, _Real>::TauCap
> AdvancedAttributeMgr<_Int, _Real>::m_tauCap;

#endif /* __ATTRIBUTE_MGR__H__INCL__ */



