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
*/

/* Slash commands the Crucible client's GM menus send, which the server didn't have.
 *  /loop               GM: /fit me this type  ->  /loop <n> /fit me <typeID>
 *  /gag /ungag         character menu > Gag User
 *  /gagiskspammer      character menu > Gag ISK Spammer (also offered to legioneers)
 *  /baniskspammer      character menu > Ban ISK Spammer
 *  /nukem              benchmark script
 *  /entity deploy      WM: /Entity deploy this type
 *  /heat               testing: shows or sets rack heat
 */

#include "eve-server.h"

#include "Client.h"
#include "EntityList.h"
#include "admin/AllCommands.h"
#include "admin/CommandDB.h"
#include "admin/CommandDispatcher.h"
#include "admin/SlashService.h"
#include "admin/CommandHelper.h"
#include "chat/LSCService.h"
#include "ship/Ship.h"
#include "services/ServiceManager.h"

namespace {
    // a character by ID or name.  only online characters
    Client* FindCharacter(const Seperator& args, uint32 index) {
        if (args.argCount() <= index)
            return nullptr;
        if (args.isNumber(index))
            return sEntityList.FindClientByCharID(atoi(args.arg(index).c_str()));
        return sEntityList.FindClientByName(args.arg(index).c_str());
    }

    void GagCharacter(Client* pTarget, int64 minutes, const std::string& reason) {
        int64 until(GetFileTimeNow() + minutes * Win32Time_Minute);
        LSCService::Gag(pTarget->GetCharacterID(), until, reason);
        pTarget->SendNotifyMsg("You have been gagged for %lli minutes: %s", minutes, reason.c_str());
    }
}

PyResult Command_loop(Client* pClient, CommandDB* db, EVEServiceManager &services, const Seperator& args)
{
    if ((args.argCount() < 3) or !args.isNumber(1))
        throw CustomError ("Correct Usage: /loop (count) (command...)");
    int count(atoi(args.arg(1).c_str()));
    if (count < 1)
        return nullptr;
    if (count > 100)
        count = 100;

    // rebuild the rest of the line as a command, keeping quoted arguments quoted
    std::string cmd;
    for (uint32 i = 2; i < args.argCount(); ++i) {
        std::string arg(args.arg(i));
        if (i == 2 and (arg.empty() or ((arg[0] != '/') and (arg[0] != '.'))))
            cmd += "/";
        if (arg.find(' ') != std::string::npos)
            arg = "\"" + arg + "\"";
        cmd += arg;
        if (i + 1 < args.argCount())
            cmd += " ";
    }
    Seperator first(&cmd.c_str()[1]);
    if (first.argCount() and (strcasecmp(first.arg(0).c_str(), "loop") == 0))
        throw CustomError ("/loop can't run /loop");

    SlashService* pSlash(services.Lookup <SlashService>("slash"));
    for (int i = 0; i < count; ++i)
        pSlash->SlashCommand(pClient, cmd);

    return new PyString("Done.");
}

PyResult Command_gag(Client* pClient, CommandDB* db, EVEServiceManager &services, const Seperator& args)
{
    // /gag <charID> "<reason>" <minutes>
    Client* pTarget(FindCharacter(args, 1));
    if (pTarget == nullptr)
        throw CustomError ("Correct Usage: /gag (online charID|name) [\"reason\"] [minutes]");
    std::string reason("Gagged by a GM");
    int64 minutes(30);
    if (args.argCount() > 2) {
        if (args.isNumber(2)) {
            minutes = atoll(args.arg(2).c_str());
        } else {
            reason = args.arg(2);
            if ((args.argCount() > 3) and args.isNumber(3))
                minutes = atoll(args.arg(3).c_str());
        }
    }
    if (minutes < 1)
        minutes = 1;
    GagCharacter(pTarget, minutes, reason);
    return new PyString("Gagged.");
}

PyResult Command_ungag(Client* pClient, CommandDB* db, EVEServiceManager &services, const Seperator& args)
{
    if ((args.argCount() < 2) or !args.isNumber(1)) {
        Client* pTarget(FindCharacter(args, 1));
        if (pTarget == nullptr)
            throw CustomError ("Correct Usage: /ungag (charID|name)");
        LSCService::Ungag(pTarget->GetCharacterID());
        pTarget->SendNotifyMsg("You are no longer gagged.");
        return new PyString("Ungagged.");
    }
    uint32 charID(atoi(args.arg(1).c_str()));
    LSCService::Ungag(charID);
    Client* pTarget(sEntityList.FindClientByCharID(charID));
    if (pTarget != nullptr)
        pTarget->SendNotifyMsg("You are no longer gagged.");
    return new PyString("Ungagged.");
}

PyResult Command_gagiskspammer(Client* pClient, CommandDB* db, EVEServiceManager &services, const Seperator& args)
{
    Client* pTarget(FindCharacter(args, 1));
    if (pTarget == nullptr)
        throw CustomError ("Correct Usage: /gagiskspammer (online charID)");
    GagCharacter(pTarget, 30 * 24 * 60, "ISK spamming");
    return new PyString("Gagged for 30 days.");
}

PyResult Command_baniskspammer(Client* pClient, CommandDB* db, EVEServiceManager &services, const Seperator& args)
{
    Client* pTarget(FindCharacter(args, 1));
    if (pTarget == nullptr)
        throw CustomError ("Correct Usage: /baniskspammer (online charID)");
    LSCService::Gag(pTarget->GetCharacterID(), GetFileTimeNow() + 365 * Win32Time_Day, "ISK spamming");
    pTarget->BanClient();
    pTarget->DisconnectClient();
    return new PyString("Banned.");
}

PyResult Command_nukem(Client* pClient, CommandDB* db, EVEServiceManager &services, const Seperator& args)
{
    Seperator cmd("killallnpcs");
    return Command_killallnpcs(pClient, db, services, cmd);
}

// /entity deploy <count> <typeID|"type name">
PyResult Command_entity(Client* pClient, CommandDB* db, EVEServiceManager &services, const Seperator& args)
{
    if ((args.argCount() < 4) or (strcasecmp(args.arg(1).c_str(), "deploy") != 0) or !args.isNumber(2))
        throw CustomError ("Correct Usage: /entity deploy (count) (typeID|\"type name\")");

    uint32 typeID(0);
    if (args.isNumber(3)) {
        typeID = atoi(args.arg(3).c_str());
    } else {
        // this hits the db directly, so refuse anything that looks like sql
        for (const auto cur : badCharsSearch)
            if (EvE::icontains(args.arg(3), cur))
                throw CustomError ("Name contains invalid characters");
        std::map<uint32_t, std::string> matches;
        if (!db->ItemSearch(args.arg(3).c_str(), matches))
            throw CustomError ("No type named %s", args.arg(3).c_str());
        for (auto cur : matches)
            if (strcasecmp(cur.second.c_str(), args.arg(3).c_str()) == 0)
                typeID = cur.first;
        if ((typeID == 0) and (matches.size() == 1))
            typeID = matches.begin()->first;
        if (typeID == 0)
            throw CustomError ("'%s' matches several types.  Use the full name or the typeID", args.arg(3).c_str());
    }

    std::string cmd("spawn " + std::to_string(typeID) + " " + args.arg(2));
    Seperator spawn(cmd.c_str());
    return Command_spawn(pClient, db, services, spawn);
}

// /heat [hi|med|low|all] [percent] [me|charID]
PyResult Command_heat(Client* pClient, CommandDB* db, EVEServiceManager &services, const Seperator& args)
{
    const char* usage("Correct Usage: /heat [hi|med|low|all] [percent] [me|charID]");
    Client* pTarget(pClient);
    if ((args.argCount() > 3) and (args.arg(3) != "me")) {
        pTarget = FindCharacter(args, 3);
        if (pTarget == nullptr)
            throw CustomError ("Character %s is not online", args.arg(3).c_str());
    }
    if (!pTarget->IsInSpace())
        throw CustomError ("%s is not in space", pTarget->GetName());
    ShipItemRef sRef(pTarget->GetShip());

    if (args.argCount() > 1) {
        int8 first(0), last(2);
        std::string rack(args.arg(1));
        if (strcasecmp(rack.c_str(), "hi") == 0) {
            first = last = 0;
        } else if ((strcasecmp(rack.c_str(), "med") == 0) or (strcasecmp(rack.c_str(), "mid") == 0)) {
            first = last = 1;
        } else if ((strcasecmp(rack.c_str(), "low") == 0) or (strcasecmp(rack.c_str(), "lo") == 0)) {
            first = last = 2;
        } else if (strcasecmp(rack.c_str(), "all") != 0) {
            throw CustomError (usage);
        }
        if ((args.argCount() < 3) or !args.isNumber(2))
            throw CustomError (usage);
        double percent(strtod(args.arg(2).c_str(), nullptr));
        if (percent < 0)
            percent = 0;
        if (percent > 100)
            percent = 100;
        const uint16 capacity[3] = { AttrHeatCapacityHi, AttrHeatCapacityMed, AttrHeatCapacityLow };
        for (int8 i = first; i <= last; ++i)
            sRef->SetRackHeat(i, percent / 100.0 * sRef->GetAttribute(capacity[i]).get_double());
    }

    const uint16 capacity[3] = { AttrHeatCapacityHi, AttrHeatCapacityMed, AttrHeatCapacityLow };
    double pct[3];
    for (int8 i = 0; i < 3; ++i) {
        double cap(sRef->GetAttribute(capacity[i]).get_double());
        pct[i] = (cap > 0 ? sRef->GetRackHeat(i) / cap * 100.0 : 0);
    }
    char buf[128];
    snprintf(buf, sizeof(buf), "%s rack heat: high %.1f%%, medium %.1f%%, low %.1f%%", sRef->name(), pct[0], pct[1], pct[2]);
    return new PyString(buf);
}
