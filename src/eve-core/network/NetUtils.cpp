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
    Author:        Zhur
*/

#include "eve-core.h"

#include "network/NetUtils.h"

uint32 ResolveIP(const char* hostname, char* errbuf) {
#ifdef HAVE_WINSOCK2_H
    static InitWinsock ws;
#endif /* !HAVE_WINSOCK2_H */
    if (errbuf)
        errbuf[0] = 0;

    if (hostname == NULL) {
        if (errbuf)
            snprintf(errbuf, ERRBUF_SIZE, "ResolveIP(): hostname == NULL");
        return 0;
    }

    struct addrinfo hints = {0};
    hints.ai_family = AF_INET;      // IPv4
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    struct addrinfo* result = NULL;
    int ret = getaddrinfo(hostname, NULL, &hints, &result);
    
    if (ret != 0) {
        if (errbuf) {
#ifdef HAVE_WINSOCK2_H
            snprintf(errbuf, ERRBUF_SIZE, "Unable to get the host name. Error: %d", WSAGetLastError());
#else /* !HAVE_WINSOCK2_H */
            snprintf(errbuf, ERRBUF_SIZE, "Unable to get the host name. Error: %s", gai_strerror(ret));
#endif /* !HAVE_WINSOCK2_H */
        }
        return 0;
    }

    // 获取第一个 IPv4 地址
    uint32 addr = 0;
    for (struct addrinfo* ptr = result; ptr != NULL; ptr = ptr->ai_next) {
        if (ptr->ai_family == AF_INET) {
            struct sockaddr_in* ipv4 = (struct sockaddr_in*)ptr->ai_addr;
            addr = ipv4->sin_addr.s_addr;
            break;
        }
    }

    freeaddrinfo(result);
    return addr;
}

/*bool ParseAddress(const char* iAddress, int32* oIP, int16* oPort, char* errbuf) {
    Seperator sep(iAddress, ':', 2, 250, false, 0, 0);
    if (sep.argnum == 1 && sep.IsNumber(1)) {
        *oIP = ResolveIP(sep.arg[0], errbuf);
        if (*oIP == 0)
            return false;
        if (oPort)
            *oPort = atoi(sep.arg[1]);
        return true;
    }
    return false;
}*/

