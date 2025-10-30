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

#ifndef __PYPACKET_VALIDATOR_H_INCL__
#define __PYPACKET_VALIDATOR_H_INCL__

#include "python/PyRep.h"
#include "log/LogNew.h"

/**
 * @brief Enhanced packet validation utilities for improved packet parsing accuracy
 * 
 * This class provides centralized validation methods for PyPacket parsing,
 * improving error detection and handling for malformed or invalid packets.
 */
class PyPacketValidator {
public:
    /**
     * @brief Validates a tuple has the expected number of elements
     * @param tuple The tuple to validate
     * @param expectedSize Expected number of elements
     * @param context Context string for error messages
     * @return true if valid, false otherwise
     */
    static bool ValidateTupleSize(PyTuple* tuple, size_t expectedSize, const char* context);

    /**
     * @brief Validates all elements in a tuple are non-null
     * @param tuple The tuple to validate
     * @param context Context string for error messages
     * @return true if all elements are valid, false otherwise
     */
    static bool ValidateTupleElements(PyTuple* tuple, const char* context);

    /**
     * @brief Validates a PyRep has the expected type
     * @param rep The PyRep to validate
     * @param expectedType The expected type
     * @param context Context string for error messages
     * @return true if type matches, false otherwise
     */
    static bool ValidateType(PyRep* rep, PyRep::PyType expectedType, const char* context);

    /**
     * @brief Validates a string is not empty and within reasonable length
     * @param str The string to validate
     * @param maxLength Maximum allowed length (default 1024)
     * @param context Context string for error messages
     * @return true if valid, false otherwise
     */
    static bool ValidateString(const std::string& str, size_t maxLength = 1024, const char* context = "string");

    /**
     * @brief Validates an integer is within expected range
     * @param value The integer value to validate
     * @param minValue Minimum allowed value
     * @param maxValue Maximum allowed value
     * @param context Context string for error messages
     * @return true if within range, false otherwise
     */
    static bool ValidateIntegerRange(int64 value, int64 minValue, int64 maxValue, const char* context);

    /**
     * @brief Validates packet structure integrity before detailed parsing
     * @param packet The packet to validate
     * @param context Context string for error messages
     * @return true if packet structure is valid, false otherwise
     */
    static bool ValidatePacketStructure(PyRep* packet, const char* context);

    /**
     * @brief Logs detailed packet information for debugging
     * @param packet The packet to dump
     * @param context Context string for the dump
     * @param logType Log type to use
     */
    static void DumpPacketDebugInfo(PyRep* packet, const char* context, LogType logType = NET__PACKET_ERROR);

private:
    // Maximum reasonable sizes for various packet elements
    static const size_t MAX_SERVICE_NAME_LENGTH = 256;
    static const size_t MAX_METHOD_NAME_LENGTH = 256;
    static const size_t MAX_TUPLE_SIZE = 1000;
    static const int64 MAX_OBJECT_ID = 0x7FFFFFFFFFFFFFFF;
    static const int64 MAX_CALL_ID = 0x7FFFFFFFFFFFFFFF;
};

#endif /* !__PYPACKET_VALIDATOR_H_INCL__ */