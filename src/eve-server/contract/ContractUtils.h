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
    Author: AlTahir
*/


/**
 * Wrapper class. Houses utility functions, used by ContractProxy and ContractMgr
 */
class ContractUtils {
public:
    static PyResult GetContractEntry(int contractId);
    static PyList* GetContractEntries(std::vector<int> contractIDList);
    static PyResult GetContractListForOwner(PyCallArgs& call);
    static void GetContractItemIDs(int contractId, std::vector<int>* into);
    static void GetRequestedItems(int contractId, std::map<int, int>* into);
    static void GetContractItemIDsAndQuantities(int contractId, std::map<int, int>* into);
private:
    static void FillItemData(DBResultRow* itemRow, PyPackedRow* targetRow);
    static void FillBidData(DBResultRow* bidRow, PyPackedRow* targetRow);
};

