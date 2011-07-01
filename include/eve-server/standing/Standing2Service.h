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


#ifndef __STANDING2_SERVICE_H_INCL__
#define __STANDING2_SERVICE_H_INCL__

#include "standing/StandingDB.h"
#include "PyService.h"

class Standing2Service : public PyService
{
public:
	Standing2Service(PyServiceMgr *mgr);
	virtual ~Standing2Service();

protected:
	class Dispatcher;
	Dispatcher *const m_dispatch;

	StandingDB m_db;

	PyCallable_DECL_CALL(GetMyKillRights)
	PyCallable_DECL_CALL(GetNPCNPCStandings)
	PyCallable_DECL_CALL(GetSecurityRating)
	PyCallable_DECL_CALL(GetMyStandings) /*
      Args:   [ 4]   [ 0]       Args:   [ 1]     Tuple: 6 elements
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 0] Object:
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 0]   Type: String: 'util.Rowset'
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 0]   Args: Dictionary: 3 entries
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 0]   Args:   [ 0] Key: String: 'header'
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 0]   Args:   [ 0] Value: List: 2 elements
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 0]   Args:   [ 0] Value:   [ 0] String: 'toID'
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 0]   Args:   [ 0] Value:   [ 1] String: 'standing'
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 0]   Args:   [ 1] Key: String: 'RowClass'
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 0]   Args:   [ 1] Value: String (Type1): 'util.Row'
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 0]   Args:   [ 2] Key: String: 'lines'
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 0]   Args:   [ 2] Value: List: Empty
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 1] Object:
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 1]   Type: String: 'util.Rowset'
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 1]   Args: Dictionary: 3 entries
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 1]   Args:   [ 0] Key: String: 'header'
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 1]   Args:   [ 0] Value: List: 3 elements
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 1]   Args:   [ 0] Value:   [ 0] String: 'ownerID'
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 1]   Args:   [ 0] Value:   [ 1] String: 'ownerName'
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 1]   Args:   [ 0] Value:   [ 2] String: 'typeID'
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 1]   Args:   [ 1] Key: String: 'RowClass'
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 1]   Args:   [ 1] Value: String (Type1): 'util.Row'
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 1]   Args:   [ 2] Key: String: 'lines'   
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 1]   Args:   [ 2] Value: List: Empty          
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 2] Object:
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 2]   Type: String: 'util.Rowset'         
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 2]   Args: Dictionary: 3 entries
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 2]   Args:   [ 0] Key: String: 'header'
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 2]   Args:   [ 0] Value: List: 2 elements
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 2]   Args:   [ 0] Value:   [ 0] String: 'fromID'
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 2]   Args:   [ 0] Value:   [ 1] String: 'standing'
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 2]   Args:   [ 1] Key: String: 'RowClass'
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 2]   Args:   [ 1] Value: String (Type1): 'util.Row'
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 2]   Args:   [ 2] Key: String: 'lines'   
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 2]   Args:   [ 2] Value: List: 1 elements        
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 2]   Args:   [ 2] Value:   [ 0] List: 2 elements   
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 2]   Args:   [ 2] Value:   [ 0]   [ 0] Integer field: 3012239
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 2]   Args:   [ 2] Value:   [ 0]   [ 1] Real Field: 10.000000
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 3] Dictionary: 0 entries
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 4] Dictionary: 0 entries                 
      Args:   [ 4]   [ 0]       Args:   [ 1]       [ 5] Dictionary: 0 entries                      
	*/
	PyCallable_DECL_CALL(GetStandingTransactions)
	PyCallable_DECL_CALL(GetCharStandings)
	PyCallable_DECL_CALL(GetCorpStandings)
};





#endif


