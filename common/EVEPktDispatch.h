/*  EVEmu: EVE Online Server Emulator

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY except by those people which sell it, which
  are required to give you total support for your newly bought product;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef __EVEDISPATCHERS_H_INCL__
#define __EVEDISPATCHERS_H_INCL__

class PyPacket;
class PyRep;
class PyRepTuple;
class AuthenticationReq;
class AuthenticationRsp;
class PyCallStream;
class EVENotificationStream;
class SessionChangeNotification;
class ErrorResponseBody;

class EVEPacketDispatcher {
public:
	EVEPacketDispatcher();
	virtual ~EVEPacketDispatcher();
	
    virtual void DispatchPacket(PyPacket **packet); //consumes the packet
	
protected:
    /* these methods can choose to consume the ** arguments */
	virtual void Handle_AuthenticationReq(const PyPacket *packet, AuthenticationReq **call);
	virtual void Handle_AuthenticationRsp(const PyPacket *packet, AuthenticationRsp **rsp);
	virtual void Handle_CallReq(const PyPacket *packet, PyCallStream **call);
	virtual void Handle_CallRsp(const PyPacket *packet, PyRepTuple **res);
	virtual void Handle_Notify(const PyPacket *packet, EVENotificationStream **notify);
	virtual void Handle_SessionChange(const PyPacket *packet, SessionChangeNotification **notify);
	virtual void Handle_ErrorResponse(const PyPacket *packet, ErrorResponseBody **body);
	virtual void Handle_Other(PyPacket **packet);
};





#endif


