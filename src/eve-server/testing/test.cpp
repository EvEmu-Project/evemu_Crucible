
 /**
  * @name test.cpp
  *   Code for testing miscellaneous items in EVEmu
  *
  * @Author:        Allan
  * @date:          19 August 2020
  *
  */

#include "eve-server.h"

#include "Client.h"
#include "system/SystemEntity.h"
#include "testing/test.h"

void testing::posTest(Client* pClient) {
    SystemEntity* mySE(pClient->GetShipSE());

    sLog.Warning("\ttesting","Test competed");
}
