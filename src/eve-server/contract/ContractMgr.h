/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://github.com/evemuproject/evemu_server
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
    Author:        Zhur
*/

#ifndef __CONTRACT_MGR__H__INCL__
#define __CONTRACT_MGR__H__INCL__

#include "PyService.h"

class ContractMgr : public PyService {
public:
    ContractMgr(PyServiceMgr *mgr);
    ~ContractMgr();

    PyCallable_DECL_CALL(NumRequiringAttention);

private:
    class Dispatcher;
    Dispatcher *const m_dispatch;
};

#endif /* __CONTRACT_MGR__H__INCL__ */


/*
(232431, `[Want To Buy]`)
(232432, `Same System`)
(232433, `One Jump Away`)
(232434, `Location`)
(232435, `Issuer`)
(232436, `Drop-off Station`)
(232437, `Required Items`)
(232438, `Get Items`)
(232439, `Original`)
(232440, `No Buyout Price`)
(232441, `Items`)
(232442, `Next System`)
(232443, `How many items would you like to transfer to your new stack?`)
(232444, `Ignore Contracts from This Issuer`)
(232445, `Contract Type`)
(232446, `Find Related Contracts`)
(232447, `View Contract`)
(232448, `Dismiss`)
(232449, `Copy`)
(232450, `None`)
(232451, `Current System`)
(232452, `{[numeric]numJumps} Jumps`)
(232453, `Items`)
(232454, `Unreachable`)
(232455, `This is a private contract`)
(232456, `Description By Issuer`)
(232457, `Auction`)
(232458, `Show Route`)
(232459, `Contract contains multiple items. Open it to view them.`)
(232460, `Pickup Station`)
(232461, `Place Bid`)
(232462, `Current Station`)
(232463, `(None)`)
(232805, `Info by Issuer`)
(232806, `jumps from start location`)
(232807, `Station may be inaccessible!`)
(232808, `Retrieving item list...`)
(232809, `Description`)
(232810, `jumps from current location`)
(232811, `Location`)
(232812, `Items For Sale`)
(232813, `Get Contracts`)
(232814, `Add Item`)
(232815, `I will pay`)
(238623, `Pick Up`)
(238624, `Click "SEARCH" to fetch contracts`)
(238625, `Location`)
(238626, `Issuer`)
(238627, `Sort Pages By`)
(238628, `Select location`)
(238629, `Price`)
(238630, `Route`)
(238631, `Contract`)
(238632, `bids`)
(238633, `Zero`)
(238634, `Volume`)
(238635, `Search not started`)
(238636, `Jumps`)
(238637, `Time Left`)
(238638, `Exclude Want To Buy`)
(238639, `Exclude Unreachable`)
(238640, `Select Category`)
(238641, `Want To Sell`)
(238642, `All`)
(238643, `Item Category`)
(238644, `Buy & Sell`)
(238645, `{categoryName} Copy`)
(238646, `Auctions`)
(238647, `{categoryName} Original`)
(238648, `Contract Search`)
(238649, `Item Group`)
(238650, `Exclude Ignored Issuers`)
(238651, `Want To Buy`)
(238652, `Courier`)
(238654, `Reward (million)`)
(238655, `Exact Type Match`)
(238656, `Price (million)`)
(238657, `Show fewer options`)
(238658, `Exclude Multiple Items`)
(238659, `Min`)
(238660, `Max`)
(238661, `My Alliance`)
(238662, `Show More options`)
(238663, `My Corporation`)
(238664, `Drop Off Location`)
(238665, `{property}`)
(238666, `{operator} {value}`)
(238667, `Create {[numeric]shares} shares`)
(238668, `Reactor`)
(238669, `Collateral (million)`)
(238670, `Enter a partial location name`)
(238671, `Collateral`)
(238672, `Created`)
(238673, `Only current security`)
(238674, `Max Jumps`)
(238675, `Search`)
(238676, `Max Route Length`)
(238677, `Solar System / Constellation / Region`)
(238678, `Reward`)
(238679, `Availability`)
*/