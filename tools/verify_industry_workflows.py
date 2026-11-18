#!/usr/bin/env python3
"""CI guard for Crucible blueprint research, copying and industry safety."""
from pathlib import Path
root = Path(__file__).resolve().parents[1]
def read(path):
    return (root / path).read_text(encoding="utf-8")
methods = read("src/eve-server/manufacturing/RamMethods.cpp")
proxy = read("src/eve-server/manufacturing/RamProxyService.cpp")
factory = read("src/eve-server/manufacturing/FactoryDB.cpp")
blueprint = read("src/eve-server/manufacturing/Blueprint.h")
math = read("src/eve-common/utils/EvEMath.cpp")
cmake = read("src/eve-test/CMakeLists.txt")
header = read("src/eve-common/EVE_RAM.h")
assert 'copy=VALUES(copy)' in factory
assert 'bool supported = false;' in factory and 'return supported;' in factory
assert 'if (data.activityID != args.activityID)' in methods
assert 'if (args.runs <= 0 || args.runs > INT16_MAX)' in methods
assert methods.count('static_cast<int32>(bpRef->') >= 2
assert ' + args.runs > INT8_MAX' in methods
assert 'Research_LevelModifier(uint8 BlueprintLevel, int32 Runs)' in math
assert 'return Runs > 0 ? static_cast<float>(Runs) : 0.0f;' in math
assert 'LEVELMODIFIERS [11]' not in math
assert 'EVEmuIndustry::ValidCopyRuns(' in methods
assert 'if (args.containerID != data.containerID)' in methods
assert 'data.ownerID != pClient->GetCharacterID()' in methods
assert 'line.containerID' in factory and 'data.containerID    = row.GetUInt(9)' in factory
assert 'uint32 containerID;' in header
assert 'sRamMthd.ItemLocationCheck(call.client, args, installedItem)' in proxy
assert 'bomLocPath.Decode(args.bomLocPath->GetItem(0))' in proxy
assert 'const double cost = calculatedCost;' in proxy
assert 'std::fabs(approvedCost - calculatedCost) > 1.0' in proxy
assert 'static_cast<std::uint64_t>(args.runs) * static_cast<std::uint64_t>(args.copyRuns)' in methods
assert '#include <cstdint>' in methods
assert 'std::map<uint16, std::uint64_t> needed;' in methods
assert 'std::uint64_t available = 0;' in methods
assert 'const std::uint64_t factor' in methods
import re
assert re.search(r'(?<![A-Za-z0-9_:])uint64\\b', methods) is None, 'Avoid undefined uint64 alias; use std::uint64_t'
assert 'into.usageCost *= ceil(into.productionTime / 3600.0f)' in methods
assert methods.count('EVEmuIndustry::RequiredMaterialQuantity(') >= 3
assert 'EVEmuIndustry::RequiredMaterialQuantity(' in proxy
assert 'runs, req.extra)' in methods and 'runs, cur.extra)' in methods
assert 'args.runs, itemItr->extra)' in proxy
assert 'if (!bpRef->infinite())' in proxy
assert 'GetBOMItems( bomLocation, items )' in methods
assert 'static_cast<float>(m_data.pLevel) / (1.0f + m_data.pLevel)' in blueprint
assert '(1.0f - (0.05f * MetallurgyLevel))' in math
assert '(1.0f - (0.05f * ResearchLevel))' in math
assert 'utils/IndustryFormulaTest.cpp' in cmake
# Invention must remain disabled, not silently charge for a fake T2 result.
assert 'case EvERam::Activity::Invention:' in methods
assert 'Blueprint invention is not functional on this server yet' in methods
assert 'requestedOwner != call.client->GetCharacterID()' in proxy
assert 'requestedOwner != call.client->GetCorporationID()' in proxy
assert 'item->ownerID() == bomLocation.ownerID' in methods
assert 'bomLocPath.ownerID != call.client->GetCharacterID()' in proxy
assert 'bomLocPath.locationID != args.lineContainerID' in proxy
assert '(*refItr)->ownerID() == bomLocPath.ownerID' in proxy
assert 'bomLocPath.ownerID != call.client->GetCorporationID()' in proxy
print("PASS: research, copying, industry ownership and invention safety invariants")
