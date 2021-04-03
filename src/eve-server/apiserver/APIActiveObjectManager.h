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
    Author:        Aknor Jaden
*/

#ifndef __APIACTIVEOBJECTMANAGER__H__INCL__
#define __APIACTIVEOBJECTMANAGER__H__INCL__

#include "apiserver/APIServiceManager.h"

/**
 * \class APIActiveObjectManager
 *
 * @brief ???
 *
 * ???
 * ???
 * ???
 *
 * @author Aknor Jaden
 * @date January 2014
 */
class APIActiveObjectManager
: public APIServiceManager
{
public:
    APIActiveObjectManager(const PyServiceMgr &services);

    // Common call shared to all derived classes called via polymorphism
    std::tr1::shared_ptr<std::string> ProcessCall(const APICommandCall * pAPICommandCall);

protected:
    // Character List, Summary, Complete information calls:
    std::tr1::shared_ptr<std::string> _CharacterList(const APICommandCall * pAPICommandCall);
    std::tr1::shared_ptr<std::string> _CharacterSummary(const APICommandCall * pAPICommandCall);
    std::tr1::shared_ptr<std::string> _Character(const APICommandCall * pAPICommandCall);

    // Solar System List, Solar System Bubble List, Bubble Ship/Entity List, etc space related lists:
    std::tr1::shared_ptr<std::string> _SystemList(const APICommandCall * pAPICommandCall);
    std::tr1::shared_ptr<std::string> _SystemSummary(const APICommandCall * pAPICommandCall);
    std::tr1::shared_ptr<std::string> _SystemBubbleList(const APICommandCall * pAPICommandCall);
    std::tr1::shared_ptr<std::string> _BubbleEntityList(const APICommandCall * pAPICommandCall);

    // Ship Info, Ship Fitting List, Fitted Module Info, Charge Info, etc
    std::tr1::shared_ptr<std::string> _ShipInfo(const APICommandCall * pAPICommandCall);
    std::tr1::shared_ptr<std::string> _ShipFittingList(const APICommandCall * pAPICommandCall);
    std::tr1::shared_ptr<std::string> _FittedModuleInfo(const APICommandCall * pAPICommandCall);
    std::tr1::shared_ptr<std::string> _FittedModuleChargeInfo(const APICommandCall * pAPICommandCall);

    // Combat Tactical Data: Targets of Selected Character/Ship, List of Entities targeting Selected Character/Ship,
    // Ship tactical information, etc
    std::tr1::shared_ptr<std::string> _ShipTargetList(const APICommandCall * pAPICommandCall);
    std::tr1::shared_ptr<std::string> _EntitiesTargetingShipList(const APICommandCall * pAPICommandCall);
    std::tr1::shared_ptr<std::string> _ShipTacticalInfo(const APICommandCall * pAPICommandCall);

    // Utility Functions:
    // none

    //APIxxxxxDB m_xxxxxDB;

};

#endif // __APIACCOUNTMANAGER__H__INCL__
