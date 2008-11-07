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

#ifndef __ATTRIBUTE_MGR__H__INCL__
#define __ATTRIBUTE_MGR__H__INCL__

/*
 * This file contains generic definitions of attribute managers.
 */

#include <map>

/*
 * The most basic attribute manager
 */
template<class _Int, class _Real>
class AttributeMgr {
public:
	/*
	 * helper typedefs
	 */
	typedef _Int int_t;
	typedef _Real real_t;

	/*
	 * Attribute definition
	 */
	enum Attr {
		#define ATTR(ID, name, default_value, persistent) \
			Attr_##name = ID,
		#include "EVEAttributes.h"
		Invalid_Attr
	};

	/*
	 * Checks if value stored is integer or not
	 */
	bool IsInt(Attr attr) const {
		real_t v = GetReal(attr);
		return(_IsInt(v));
	}

	/*
	 * Get attribute value as integer
	 * just round the real
	 */
	virtual int_t GetInt(Attr attr) const {
		return(
			int_t(GetReal(attr))
		);
	}
	/*
	 * Get attribute value as real
	 */
	virtual real_t GetReal(Attr attr) const {
		real_t v;
		if(_Get(attr, v)) {
			// we found it, return
			return(v);
		}
		// nothing found ...
		return(0);
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
		// check if we can save memory by saving it as integer instead
		if(_IsInt(v)) {
			// good, we can store it as integer
			SetInt(attr, v);
		} else {
			// delete the integer value (no matter if there really is any)
			m_ints.erase(attr);
			// set the value
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
		// clear integers
		m_ints.clear();
		// clear reals
		m_reals.clear();
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
	#include "EVEAttributes.h"

protected:
	/*
	 * Retrieves the attribute value
	 * - returns true on success
	 * - returns false if not found
	 */
	bool _Get(Attr attr, real_t &into) const {
		// try real value
		std::map<Attr, real_t>::const_iterator ri = m_reals.find(attr);
		if(ri != m_reals.end()) {
			// found real value
			into = ri->second;
			return(true);
		}

		// real not found, try integer
		std::map<Attr, int_t>::const_iterator ii = m_ints.find(attr);
		if(ii != m_ints.end()) {
			// found integer value
			into = ii->second;
			return(true);
		}

		// nothing at all, return false
		return(false);
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
};

#endif /* __ATTRIBUTE_MGR__H__INCL__ */



