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
    Author:     EVEmu Team
*/

#include "eve-common.h"
#include "python/PyPacketValidator.h"
#include "python/PyRep.h"
#include "python/PyDumpVisitor.h"

bool PyPacketValidator::ValidateTupleSize(PyTuple* tuple, size_t expectedSize, const char* context) {
    if (tuple == nullptr) {
        codelog(NET__PACKET_ERROR, "%s - tuple is null", context);
        return false;
    }

    if (tuple->items.size() != expectedSize) {
        codelog(NET__PACKET_ERROR, "%s - invalid tuple size: expected %zu, got %zu", 
                context, expectedSize, tuple->items.size());
        tuple->Dump(NET__PACKET_ERROR, "  Invalid tuple: ");
        return false;
    }

    if (tuple->items.size() > MAX_TUPLE_SIZE) {
        codelog(NET__PACKET_ERROR, "%s - tuple size exceeds maximum: %zu > %zu", 
                context, tuple->items.size(), MAX_TUPLE_SIZE);
        return false;
    }

    return true;
}

bool PyPacketValidator::ValidateTupleElements(PyTuple* tuple, const char* context) {
    if (tuple == nullptr) {
        codelog(NET__PACKET_ERROR, "%s - tuple is null", context);
        return false;
    }

    for (size_t i = 0; i < tuple->items.size(); ++i) {
        if (tuple->items[i] == nullptr) {
            codelog(NET__PACKET_ERROR, "%s - tuple element %zu is null", context, i);
            return false;
        }
    }

    return true;
}

bool PyPacketValidator::ValidateType(PyRep* rep, PyRep::PyType expectedType, const char* context) {
    if (rep == nullptr) {
        codelog(NET__PACKET_ERROR, "%s - PyRep is null", context);
        return false;
    }

    if (rep->GetType() != expectedType) {
        codelog(NET__PACKET_ERROR, "%s - invalid type: expected %d, got %s", 
                context, static_cast<int>(expectedType), rep->TypeString());
        rep->Dump(NET__PACKET_ERROR, "  Invalid element: ");
        return false;
    }

    return true;
}

bool PyPacketValidator::ValidateString(const std::string& str, size_t maxLength, const char* context) {
    if (str.empty()) {
        codelog(NET__PACKET_ERROR, "%s - string is empty", context);
        return false;
    }

    if (str.length() > maxLength) {
        codelog(NET__PACKET_ERROR, "%s - string length exceeds maximum: %zu > %zu", 
                context, str.length(), maxLength);
        return false;
    }

    // Check for null bytes in string (potential security issue)
    if (str.find('\0') != std::string::npos) {
        codelog(NET__PACKET_ERROR, "%s - string contains null bytes", context);
        return false;
    }

    return true;
}

bool PyPacketValidator::ValidateIntegerRange(int64 value, int64 minValue, int64 maxValue, const char* context) {
    if (value < minValue || value > maxValue) {
        codelog(NET__PACKET_ERROR, "%s - integer out of range: %lld not in [%lld, %lld]", 
                context, value, minValue, maxValue);
        return false;
    }

    return true;
}

bool PyPacketValidator::ValidatePacketStructure(PyRep* packet, const char* context) {
    if (packet == nullptr) {
        codelog(NET__PACKET_ERROR, "%s - packet is null", context);
        return false;
    }

    // Check if packet is one of the expected types
    if (!packet->IsSubStream() && !packet->IsChecksumedStream() && !packet->IsObject()) {
        codelog(NET__PACKET_ERROR, "%s - packet has invalid type: %s", 
                context, packet->TypeString());
        DumpPacketDebugInfo(packet, context);
        return false;
    }

    return true;
}

void PyPacketValidator::DumpPacketDebugInfo(PyRep* packet, const char* context, LogType logType) {
    if (packet == nullptr) {
        _log(logType, "%s - packet is null, cannot dump debug info", context);
        return;
    }

    _log(logType, "%s - Packet Debug Info:", context);
    _log(logType, "  Type: %s", packet->TypeString());
    
    // Dump packet contents with visitor
    PyLogDumpVisitor dumper(logType, logType, "    ");
    packet->visit(dumper);
}