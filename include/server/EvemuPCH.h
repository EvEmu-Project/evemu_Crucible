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
    Author:     Zhur
*/

#ifndef __EVEPCH_H
#define __EVEPCH_H

// common files first
#include "../common/common.h"

// commented because why would we do this?....
//#define IMPLEMENT_SIGEXCEPT
//#include "../common/sigexcept/sigexcept.h"

#include "../common/dbcore.h"
#include "../common/logsys.h"

#include "../common/PyVisitor.h"
#include "../common/PyRep.h"
#include "../common/PyPacket.h"
#include "../common/PyDumpVisitor.h"
#include "../common/PyLookupDump.h"

#include "../common/PyXMLGenerator.h"

#include "../common/packet_functions.h"

#include "../common/timer.h"
#include "../common/gpoint.h"
#include "../common/StreamPacketizer.h"

#include "../common/TCPConnection.h"

#include "../common/EVETCPConnection.h" // eve client socket
#include "../common/EVEUtils.h"
#include "../common/EVEDBUtils.h"
#include "../common/EVEPresentation.h"
#include "../common/EVEmuRevision.h"

#include "../common/TCPServer.h"

#include "../common/EVETCPServer.h"

#include "../common/Base64.h"
#include "../common/CachedObjectMgr.h"
#include "../common/common.h"
#include "../common/counted_ptr.h"
#include "../common/crc32.h"
#include "../common/dbcore.h"
#include "../common/DestinyBinDump.h"
#include "../common/DestinyStructs.h"
#include "../common/DirWalker.h"
#include "../common/EVEDBUtils.h"
#include "../common/EVEMarshal.h"
#include "../common/EVEMarshalOpcodes.h"
#include "../common/EVEmuRevision.h"
#include "../common/EVEPktDispatch.h"
#include "../common/EVEPresentation.h"
#include "../common/EVETCPConnection.h"
#include "../common/EVETCPServer.h"
#include "../common/EVEUnmarshal.h"
#include "../common/EVEUtils.h"
#include "../common/EVEZeroCompress.h"
#include "../common/GaPreReqs.h"
#include "../common/GaMath.h"
#include "../common/GaTypes.h"
#include "../common/gpoint.h"
#include "../common/logsys.h"
#include "../common/logtypes.h"
#include "../common/misc.h"
#include "../common/MiscFunctions.h"
#include "../common/Mutex.h"

#include "../common/DestinyBinDump.h"
#include "../common/DestinyStructs.h"

//#include "../common/misc.h"


#include "../server/ClientSession.h"

#include "../server/PyCallable.h"
#include "../server/PyServiceMgr.h"
#include "../server/EVEmuServerConfig.h"
#include "../server/EntityList.h"
#include "../server/NetService.h"

#include "../server/PyBoundObject.h"


#include "../../src/packets/AccountPkts.h"

#include "../../src/packets/Crypto.h"
#include "../../src/packets/Character.h"
#include "../../src/packets/Destiny.h"
#include "../../src/packets/DogmaIM.h"

#include "../../src/packets/General.h"
#include "../../src/packets/Inventory.h"
#include "../../src/packets/Wallet.h"
#include "../../src/packets/Missions.h"
#include "../../src/packets/Language.h"
#include "../../src/packets/LSCPkts.h"
#include "../../src/packets/Manufacturing.h"
#include "../../src/packets/ObjectCaching.h"

#include "../../src/packets/Market.h"
#include "../../src/packets/Standing2.h"
#include "../../src/packets/Tutorial.h"


// then class files
#include "../server/account/AccountDB.h"
#include "../server/account/AccountService.h"
#include "../server/account/AuthService.h"
#include "../server/account/UserService.h"

#include "../server/admin/AlertService.h"
#include "../server/admin/AllCommands.h"
#include "../server/admin/CommandDispatcher.h"
#include "../server/admin/CommandDB.h"
#include "../server/admin/PetitionerService.h"
#include "../server/admin/SlashService.h"

// client object cache stuff
#include "../server/cache/ObjCacheService.h"
#include "../server/cache/ObjCacheDB.h"

// character stuff
#include "../server/character/CertificateMgrDB.h"
#include "../server/character/CertificateMgrService.h"
#include "../server/character/Character.h"
#include "../server/character/CharacterAppearance_fields.h"
#include "../server/character/CharacterDB.h"
#include "../server/character/CharacterService.h"
#include "../server/character/CharMgrService.h"
#include "../server/character/Skill.h"
#include "../server/character/SkillMgrService.h"


// chat stuff
#include "../server/chat/LookupService.h"
#include "../server/chat/LSCChannel.h"
#include "../server/chat/LSCDB.h"
#include "../server/chat/LSCService.h"
#include "../server/chat/OnlineStatusService.h"
#include "../server/chat/VoiceMgrService.h"

// server config stuff
#include "../server/config/ConfigDB.h"
#include "../server/config/ConfigService.h"
#include "../server/config/LanguageService.h"

// corporation stuff
#include "../server/corporation/CorpMgrService.h"
#include "../server/corporation/CorporationCarrier.h"
#include "../server/corporation/CorporationDB.h"
#include "../server/corporation/CorporationService.h"
#include "../server/corporation/CorpRegistryService.h"
#include "../server/corporation/CorpStationMgrService.h"
#include "../server/corporation/LPService.h"

// dogma crap dono what this is
#include "../server/dogmaim/DogmaIMDB.h"
#include "../server/dogmaim/DogmaIMService.h"

// item stuff
#include "../server/inventory/AttributeMgr.h"
#include "../server/inventory/EVEAttributeMgr.h"
#include "../server/inventory/InvBrokerService.h"
#include "../server/inventory/Inventory.h"
#include "../server/inventory/InventoryDB.h"
#include "../server/inventory/InventoryItem.h"
#include "../server/inventory/ItemDB.h"
#include "../server/inventory/ItemFactory.h"
#include "../server/inventory/ItemRef.h"
#include "../server/inventory/ItemType.h"
#include "../server/inventory/Owner.h"

// factory stuff
#include "../server/manufacturing/Blueprint.h"
#include "../server/manufacturing/FactoryDB.h"
#include "../server/manufacturing/FactoryService.h"
#include "../server/manufacturing/RamProxyDB.h"
#include "../server/manufacturing/RamProxyService.h"


//services:
#include "../server/missions/AgentMgrService.h"
#include "../server/missions/MissionMgrService.h"
#include "../server/missions/Agent.h"
#include "../server/missions/MissionDB.h"


#include "../server/mining/Asteroid.h"
#include "../server/mining/AsteroidBeltManager.h"
#include "../server/mining/MiningDB.h"
#include "../server/mining/ReprocessingDB.h"
#include "../server/mining/ReprocessingService.h"

#include "../server/map/MapDB.h"
#include "../server/map/MapService.h"

#include "../server/market/BillMgrService.h"
#include "../server/market/ContractMgrService.h"
#include "../server/market/MarketDB.h"
#include "../server/market/MarketProxyService.h"

#include "../server/posmgr/PosMgrDB.h"
#include "../server/posmgr/PosMgrService.h"

#include "../server/npc/NPC.h"
#include "../server/npc/NPCAI.h"

#include "../server/system/BookmarkDB.h"
#include "../server/system/BookmarkService.h"
#include "../server/system/BubbleManager.h"
#include "../server/system/Celestial.h"
#include "../server/system/Damage.h"
#include "../server/system/DungeonService.h"
#include "../server/system/KeeperService.h"
#include "../server/system/ScenarioService.h"
#include "../server/system/SolarSystem.h"
#include "../server/system/SystemBubble.h"
#include "../server/system/SystemDB.h"
#include "../server/system/SystemEntities.h"
#include "../server/system/SystemEntity.h"
#include "../server/system/SystemManager.h"


#include "../server/ship/ModuleManager.h"
#include "../server/ship/BeyonceService.h"
#include "../server/ship/DestinyManager.h"
#include "../server/ship/InsuranceService.h"
#include "../server/ship/Ship.h"
#include "../server/ship/ShipDB.h"
#include "../server/ship/ShipService.h"
#include "../server/ship/TargetManager.h"

#include "../server/spawn/SpawnDB.h"
#include "../server/spawn/SpawnManager.h"

#include "../server/standing/FactionWarMgrDB.h"
#include "../server/standing/FactionWarMgrService.h"
#include "../server/standing/Standing2Service.h"
#include "../server/standing/StandingDB.h"
#include "../server/standing/WarRegistryService.h"


#include "../server/station/JumpCloneService.h"
#include "../server/station/Station.h"
#include "../server/station/StationDB.h"
#include "../server/station/StationService.h"
#include "../server/station/StationSvcService.h"

#include "../server/tutorial/TutorialDB.h"
#include "../server/tutorial/TutorialService.h"

// client stuff
#include "../server/Client.h"
#include "../server/PyServiceCD.h"

#endif//__EVEPCH_H
