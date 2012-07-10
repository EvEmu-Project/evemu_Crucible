/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2011 The EVEmu Team
    For the latest information visit http://evemu.org
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
    Author:        Aknor Jaden, adapted from ImageServerListener.h authored by caytchen
*/

#ifndef __APISERVERLISTENER__H__INCL__
#define __APISERVERLISTENER__H__INCL__

#include "apiserver/APIServerConnection.h"

/**
 * \class APIServerListener
 *
 * @brief Handles listening for new clients
 *
 * Asynchronously listens for new clients and creates new connections for them to process them
 *
 * @author Aknor Jaden
 * @date July 2011
 */
class APIServerListener
{
public:
    APIServerListener(boost::asio::io_service& io);
    ~APIServerListener();

private:
    void StartAccept();
    void HandleAccept(std::tr1::shared_ptr<APIServerConnection> connection);

    boost::asio::ip::tcp::acceptor* _acceptor;
};

#endif // __APISERVERLISTENER__H__INCL__
