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
    Author:        Allan
*/

#include "eve-server.h"


#include "corporation/LPStore.h"

LPStore::LPStore() :
    Service("storeServer")
{
    this->Add("AcceptOffer", &LPStore::AcceptOffer);
    this->Add("GetAvailableOffers", &LPStore::GetAvailableOffers);
    this->Add("GetAvailableOffersFromCorp", &LPStore::GetAvailableOffersFromCorp);
    this->Add("GetLPForCharacterCorp", &LPStore::GetLPForCharacterCorp);
    this->Add("GetLPExchangeRates", &LPStore::GetLPExchangeRates);
    this->Add("ExchangeConcordLP", &LPStore::ExchangeConcordLP);
    this->Add("TakeOffer", &LPStore::TakeOffer);

}

PyResult LPStore::AcceptOffer(PyCallArgs& call, PyInt* offerID, PyInt* quantity) {
  /**
            return sm.RemoteSvc('storeServer').AcceptOffer(offerID, quantity)
            */
  sLog.White( "LPStore::AcceptOffer()", "size=%lu", call.tuple->size());

  call.Dump(SERVICE__CALL_DUMP);
    return new PyList;
}

PyResult LPStore::GetAvailableOffers(PyCallArgs& call) {
  /**
            availableOffers = sm.RemoteSvc('storeServer').GetAvailableOffers()
            */
    //no args
  sLog.White( "LPStore::GetAvailableOffers()", "size=%lu", call.tuple->size());

  call.Dump(SERVICE__CALL_DUMP);
    return new PyList;
}

PyResult LPStore::GetAvailableOffersFromCorp(PyCallArgs& call, PyInt* corpID) {
  /**
            self.cache.offers = sm.RemoteSvc('LPSvc').GetAvailableOffersFromCorp(self.cache.corpID)
            */
  sLog.White( "LPStore::GetAvailableOffersFromCorp()", "size=%lu", call.tuple->size());

  call.Dump(SERVICE__CALL_DUMP);
    return new PyList;
}

PyResult LPStore::GetLPForCharacterCorp(PyCallArgs& call, PyInt* corpID) {
  /**
            self.cache.lps = sm.RemoteSvc('LPSvc').GetLPForCharacterCorp(corpID)
            */
  sLog.White( "LPStore::GetLPForCharacterCorp()", "size=%lu", call.tuple->size());

  call.Dump(SERVICE__CALL_DUMP);
    return new PyInt(0);
}

PyResult LPStore::GetLPExchangeRates(PyCallArgs& call) {
  /**
            self.cache.exchangeRates = sm.RemoteSvc('LPSvc').GetLPExchangeRates()
            */
  sLog.White( "LPStore::GetLPExchangeRates()", "size=%lu", call.tuple->size());

  call.Dump(SERVICE__CALL_DUMP);
    PyDict* result = new PyDict();
    return result;
}

PyResult LPStore::ExchangeConcordLP(PyCallArgs& call, PyInt* corpID, PyInt* amount) {
  /**
            sm.RemoteSvc('LPSvc').ExchangeConcordLP(corpID, amount)
            */
  sLog.White( "LPStore::ExchangeConcordLP()", "size=%lu", call.tuple->size());

  call.Dump(SERVICE__CALL_DUMP);
    return new PyNone;
}

PyResult LPStore::TakeOffer(PyCallArgs& call, PyInt* corpID, PyInt* offerID, PyInt* quantity) {
  /**
            ret = sm.RemoteSvc('LPSvc').TakeOffer(self.cache.corpID, data.offerID, numberOfOffers)
            */
  sLog.White( "LPStore::TakeOffer()", "size=%lu", call.tuple->size());

  call.Dump(SERVICE__CALL_DUMP);
    return new PyBool(true);
}

