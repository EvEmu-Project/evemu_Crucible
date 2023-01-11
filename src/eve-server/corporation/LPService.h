/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://evemu.dev
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
    Author:        Bloody.Rabbit,  Allan
*/

#ifndef __LP_SERVICE__H__INCL__
#define __LP_SERVICE__H__INCL__

#include "services/Service.h"

class LPService : public Service <LPService>
{
public:
    LPService();

    // Adds LP to Characters LP Wallet
    static void AddLP(uint32 characterID, uint32 corporationID, int amount);
    // Calculate LP reward for missions
    static int GetLPReward(uint16 missionID, uint32 solarsystemID, uint8 agentLevel);
    // Return specific LP balance
    static int GetLPBalanceForCorp(int32 characterID, int32 corporationID);

protected:
    PyResult TakeOffer(PyCallArgs& call, PyInt* corpID, PyInt* storeID);
    PyResult ExchangeConcordLP(PyCallArgs& call, PyInt* corporationID, PyFloat* amount);
    PyResult GetLPExchangeRates(PyCallArgs& call);
    PyResult GetLPForCharacterCorp(PyCallArgs& call, PyInt* corporationID);
    PyResult GetLPsForCharacter(PyCallArgs& call);
    PyResult GetAvailableOffersFromCorp(PyCallArgs& call, PyInt* corporationID, PyBool* trueValue);

    static DBQueryResult GetLPRowsForCharacter(int32 characterID);
    static DBQueryResult GetLPOffersForCorp(int32 corporationID);
    static DBQueryResult GetRequiredItemsForOffer(int32 storeID);
    static DBResultRow GetLPOffer(int32 storeID);
    static void CreateNewLPBalance(int32 characterID, int32 corporationID, int balance);
    static void UpdateLPBalance(int32 characterID, int32 corporationID, int balance);
};

#endif /* !__LP_SERVICE__H__INCL__ */
