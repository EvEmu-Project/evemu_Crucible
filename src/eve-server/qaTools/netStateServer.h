
 /**
  * @name netStateServer.h
  *     *not sure yet* system for EVEmu
  *
  * @Author:        Allan
  * @date:          13 December 2018
  *
  */



#ifndef _EVEMU_QATOOLS_NETSTATE_SERVER_H
#define _EVEMU_QATOOLS_NETSTATE_SERVER_H

#include "services/Service.h"


class netStateServer : public Service<netStateServer>
{
public:
    netStateServer();
};


#endif  // _EVEMU_QATOOLS_NETSTATE_SERVER_H



