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
    Author:        Bloody.Rabbit,   Allan
*/

#include "eve-server.h"
#include "EntityList.h"

#include "corporation/LPService.h"
#include "account/AccountService.h"

LPService::LPService() :
    Service("LPSvc", eAccessLevel_Character)
{
    this->Add("TakeOffer", &LPService::TakeOffer);
    this->Add("ExchangeConcordLP", &LPService::ExchangeConcordLP);
    this->Add("GetLPExchangeRates", &LPService::GetLPExchangeRates);
    this->Add("GetLPsForCharacter", &LPService::GetLPsForCharacter);
    this->Add("GetLPForCharacterCorp", &LPService::GetLPForCharacterCorp);
    this->Add("GetAvailableOffersFromCorp", &LPService::GetAvailableOffersFromCorp);
}

void LPService::AddLP(uint32 characterID, uint32 corporationID, int amount)
{
  int currentBalance = GetLPBalanceForCorp(characterID, corporationID);
  if (currentBalance == -1) {
    CreateNewLPBalance(characterID, corporationID, amount);
  } else {
    UpdateLPBalance(characterID, corporationID, currentBalance + amount);
  }
  // Notify Client of LP Change
  OnLPChange ac;
  ac.corpID = corporationID;
  PyTuple *answer = ac.Encode();
  sEntityList.FindClientByCharID(characterID)->SendNotification("OnLPChange", "cash", &answer, false);
}

int LPService::GetLPReward(uint16 missionID, uint32 solarsystemID, uint8 agentLevel)
{
  SystemData m_data;
  sDataMgr.GetSystemData(solarsystemID, m_data); // Is there a better way to pull the system security from a systemID?
  int baseLP =  25000; // BaseLP supposedly calculated dynamically based on average mission time and potentially other factors as well.
  for (int i = 5; i > agentLevel; i--) { // This is just a hacky way to create a psudo-realistic BaseLP value.
    baseLP /= i;
  }
  return (1.6288 - m_data.securityRating ) * baseLP;   // LP reward = (1.6288 - System security) × Base LP
}

PyResult LPService::TakeOffer(PyCallArgs& call, PyInt* corpID, PyInt* storeID)
{
  if (!corpID->value() >= 1000000 && !corpID->value() <= 1000200) { // Bounds check valid corpID in call
    return new PyNone;
  }
  if (!storeID->value() >= 1 && !storeID->value() <= 30000) { // Bounds check valid storeID in call
    return new PyNone;
  }
  // Lookup Store Offer
  DBResultRow offer = GetLPOffer(storeID->value());
  int32 typeID = offer.GetInt(0);
  int32 iskCost = offer.GetInt(1);
  int32 quantity = offer.GetInt(2);
  int32 lpCost = offer.GetInt(3);
  // Confirm char has LP & ISK
  int32 lpBalance = GetLPBalanceForCorp(call.client->GetCharacterID(), corpID->value());
  if (lpBalance < lpCost) {
    throw CustomError("You do not have enough LP to make this purchase.");
  }
  int32 charIsk = call.client->GetBalance();
  if (charIsk < iskCost) {
    throw CustomError("You do not have enough ISK to make this purchase.");
  }
  // Check required items & remove if all are present.
  DBQueryResult requiredItems = GetRequiredItemsForOffer(storeID->value());
  if (requiredItems.GetRowCount() > 0) {
    // TODO: Remove required from hangar, the client pre-checks the required items are available or else the button is hidden.
  }
  // Remove LP & ISK
  AddLP(call.client->GetCharacterID(), corpID->value(), -lpCost);
  AccountService::TranserFunds(call.client->GetCharacterID(), corpID->value(), iskCost, "LP Store purchase", Journal::EntryType::PaymentToLPStore, storeID->value());
  // Give char item from store and place in hangar.
  EVEItemFlags flag;
  uint32 locationID = 0;
  locationID = call.client->GetStationID();
  flag = flagHangar;
  ItemData idata(
          typeID,
          call.client->GetCharacterID(),
                    locTemp, //temp location
                    flag,
                    quantity);
  InventoryItemRef iRef = sItemFactory.SpawnItem(idata);
  if (iRef.get() == nullptr)
    throw CustomError ("Unable to create item of type %i.", typeID);
  iRef->Move(locationID, flag, true);
  iRef->SaveItem();
  return new PyNone;
}

PyResult LPService::ExchangeConcordLP(PyCallArgs& call, PyInt* corporationID, PyFloat* amount)
{/**
            ret = sm.RemoteSvc('LPSvc').TakeOffer(self.cache.corpID, data.offerID)
            */
  sLog.White( "LPService::Handle_ExchangeConcordLP()", "size=%lu", call.tuple->size());

  call.Dump(SERVICE__CALL_DUMP);
    return new PyList;
}

//17:09:54 L LPService::Handle_GetLPExchangeRates(): size= 0
PyResult LPService::GetLPExchangeRates(PyCallArgs& call)
{/**
            self.cache.exchangeRates = sm.RemoteSvc('LPSvc').GetLPExchangeRates()

05:42:04 [SvcCall] Service LPSvc: calling GetLPExchangeRates
05:42:04 [SvcCall]   Call Arguments:
05:42:04 [SvcCall]       Tuple: Empty
05:42:04 [SvcCall]   Call Named Arguments:
05:42:04 [SvcCall]     Argument 'machoVersion':
05:42:04 [SvcCall]         Integer field: 1
*/
    return new PyList;
}

//18:46:38 L CharMgrService::Handle_GetLPForCharacterCorp(): size= 1, 0=Integer(1000049)
PyResult LPService::GetLPForCharacterCorp(PyCallArgs& call, PyInt* corporationID)
{/**
                self.cache.lps = sm.RemoteSvc('LPSvc').GetLPForCharacterCorp(corpID)
         self.cache.concordLps = sm.RemoteSvc('LPSvc').GetLPForCharacterCorp(const.corpCONCORD)

04:39:44 [SvcCall] Service LPSvc: calling GetLPForCharacterCorp
04:39:44 L CharMgrService::Handle_GetLPForCharacterCorp(): size= 1, 0=Integer(1000049)
04:39:44 [SvcCall]   Call Arguments:
04:39:44 [SvcCall]       Tuple: 1 elements
04:39:44 [SvcCall]         [ 0] Integer field: 1000049
04:39:44 [SvcCall]   Call Named Arguments:
04:39:44 [SvcCall]     Argument 'machoVersion':
04:39:44 [SvcCall]         Integer field: 1
*/
  int balance = GetLPBalanceForCorp(call.client->GetCharacterID(), corporationID->value());
  if (balance < 1)
    balance = 0;
  return new PyInt(balance);
}

// //06:01:19 LPService::Handle_GetLPsForCharacter(): size= 0
PyResult LPService::GetLPsForCharacter(PyCallArgs& call)
{
  //no args
  sLog.White( "LPService::Handle_GetLPsForCharacter()", "size=%lu", call.tuple->size());
  DBQueryResult dbRes = GetLPRowsForCharacter(call.client->GetCharacterID());
  DBResultRow row;
  PyList *res = new PyList(dbRes.GetRowCount());
  int i = 0;
  while (dbRes.GetRow(row)) {
    PyTuple *tuple = new PyTuple(2);
    tuple->SetItemInt(0, row.GetInt(1));
    tuple->SetItemInt(1, row.GetInt(2));
    res->SetItem(i, tuple);
    i++;
  }
  //call.Dump(SERVICE__CALL_DUMP);
    return res;
}

//18:55:57 L CharMgrService::Handle_GetAvailableOffersFromCorp(): size=2, 0=Integer(), 1=Boolean()
PyResult LPService::GetAvailableOffersFromCorp(PyCallArgs& call, PyInt* corporationID, PyBool* trueValue)
{
  if (!corporationID->value() >= 1000000 && !corporationID->value() <= 1000200) { // Bounds check valid corpID in call, else return empty list.
    return new PyList;
  }
  DBQueryResult dbRes = GetLPOffersForCorp(corporationID->value());
  DBResultRow row;
  PyList *res = new PyList(dbRes.GetRowCount());
  int i = 0;
  while (dbRes.GetRow(row)) {
    PyDict *dict = new PyDict();
    DBQueryResult dbResReqItems = GetRequiredItemsForOffer(row.GetInt(2));
    PyList *list = new PyList(dbResReqItems.GetRowCount());
    int j = 0;
    DBResultRow reqItemsRow;
    while (dbResReqItems.GetRow(reqItemsRow)) {
      PyTuple *tuple2 = new PyTuple(2);
      tuple2->SetItemInt(0, reqItemsRow.GetInt(0));
      tuple2->SetItemInt(1, reqItemsRow.GetInt(1));
      list->SetItem(j, tuple2);
      j++;
    }
    dict->SetItem("typeID", new PyInt(row.GetInt(0)));
    dict->SetItem("iskCost", new PyInt(row.GetInt(1)));
    dict->SetItem("reqItems", list);
    dict->SetItem("offerID", new PyInt(row.GetInt(2)));
    dict->SetItem("qty", new PyInt(row.GetInt(3)));
    dict->SetItem("lpCost", new PyInt(row.GetInt(4)));
    res->SetItem(i, new PyObject("util.KeyVal", dict));
    i++;
  }
  
  return res;
  /**
            self.cache.offers = sm.RemoteSvc('LPSvc').GetAvailableOffersFromCorp(self.cache.corpID, True)

05:42:04 [SvcCall] Service LPSvc: calling GetAvailableOffersFromCorp
05:42:04 [SvcCall]   Call Arguments:
05:42:04 [SvcCall]       Tuple: 2 elements
05:42:04 [SvcCall]         [ 0] Integer field: 1000049
05:42:04 [SvcCall]         [ 1] Boolean field: true
05:42:04 [SvcCall]   Call Named Arguments:
05:42:04 [SvcCall]     Argument 'machoVersion':
05:42:04 [SvcCall]         Integer field: 1
*/
/*
        [PyList 326 items]
          [PyObjectData Name: util.KeyVal]
            [PyDict 6 kvp]
              [PyString "typeID"]
              [PyInt 23047]
              [PyString "iskCost"]
              [PyInt 2400000]
              [PyString "reqItems"]
              [PyList 1 items]
                [PyTuple 2 items]
                  [PyInt 234]
                  [PyInt 5000]
              [PyString "offerID"]
              [PyInt 3584]
              [PyString "qty"]
              [PyInt 5000]
              [PyString "lpCost"]
              [PyInt 2400]
          [PyObjectData Name: util.KeyVal]
            [PyDict 6 kvp]
              [PyString "typeID"]
              [PyInt 23033]
              [PyString "iskCost"]
              [PyInt 1600000]
              [PyString "reqItems"]
              [PyList 1 items]
                [PyTuple 2 items]
                  [PyInt 226]
                  [PyInt 5000]
              [PyString "offerID"]
              [PyInt 3585]
              [PyString "qty"]
              [PyInt 5000]
              [PyString "lpCost"]
              [PyInt 1600]
              */
    // return new PyList;
}

// Database Methods
DBQueryResult LPService::GetLPRowsForCharacter(int32 characterID)
{
  DBQueryResult dbRes;
  if (!sDatabase.RunQuery(dbRes,
      "SELECT "
      " characterID, corporationID, balance"
      " FROM lpWallet"
      " WHERE characterID = %i", \
      characterID ))
  {
      codelog(DATABASE__ERROR, "Error in query: %s", dbRes.error.c_str());
  }
  return dbRes;
}

DBQueryResult LPService::GetLPOffersForCorp(int32 corporationID)
{
  DBQueryResult dbRes;
  if (!sDatabase.RunQuery(dbRes,
      "SELECT "
      " typeID, iskCost, storeID, quantity, lpCost"
      " FROM lpStore"
      " WHERE corporationID = %i", \
      corporationID ))
  {
      codelog(DATABASE__ERROR, "Error in query: %s", dbRes.error.c_str());
  }
  return dbRes;
}

DBResultRow LPService::GetLPOffer(int32 storeID)
{
  DBQueryResult dbResOffer;
  if (!sDatabase.RunQuery(dbResOffer,
      "SELECT "
      " typeID, iskCost, quantity, lpCost"
      " FROM lpStore"
      " WHERE storeID = %i LIMIT 1", \
      storeID ))
  {
      codelog(DATABASE__ERROR, "Error in query: %s", dbResOffer.error.c_str());
  }
  DBResultRow offerRow;
  dbResOffer.GetRow(offerRow);
  return offerRow;
}

DBQueryResult LPService::GetRequiredItemsForOffer(int32 storeID)
{
  DBQueryResult dbRes;
  if (!sDatabase.RunQuery(dbRes,
      "SELECT "
      " typeID, quantity"
      " FROM lpRequiredItems"
      " WHERE parentID = %i", \
      storeID ))
  {
      codelog(DATABASE__ERROR, "Error in query: %s", dbRes.error.c_str());
  }
  return dbRes;
}

int LPService::GetLPBalanceForCorp(int32 characterID, int32 corporationID)
{
  DBQueryResult dbRes;
  if (!sDatabase.RunQuery(dbRes,
      "SELECT "
      " characterID, corporationID, balance"
      " FROM lpWallet"
      " WHERE characterID = %i AND corporationID = %i"
      " LIMIT 1", \
      characterID, corporationID ))
  {
      codelog(DATABASE__ERROR, "Error in query: %s", dbRes.error.c_str());
      return -1;
  }
  if (dbRes.GetRowCount() == 0)
    return -1;
  DBResultRow row;
  dbRes.GetRow(row);
  return row.GetInt(2);
}

void LPService::CreateNewLPBalance(int32 characterID, int32 corporationID, int balance)
{
  DBerror err;
  sDatabase.RunQuery(err,
    "INSERT INTO lpWallet"
    " (characterID, corporationID, balance)"
    " VALUES (%i, %i, %i)", \
    characterID, corporationID, balance );
}

void LPService::UpdateLPBalance(int32 characterID, int32 corporationID, int balance)
{
  DBerror err;
  sDatabase.RunQuery(err,
    "UPDATE lpWallet"
    " SET balance = %i"
    " WHERE characterID = %i AND corporationID = %i", \
    balance, characterID, corporationID );
}