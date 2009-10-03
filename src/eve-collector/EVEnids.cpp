/*
	------------------------------------------------------------------------------------
	LICENSE:
	------------------------------------------------------------------------------------
	This file is part of EVEmu: EVE Online Server Emulator
	Copyright 2006 - 2008 The EVEmu Team
	For the latest information visit http://evemu.mmoforge.org
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
	Author:		Zhur
*/

#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "nids.h"
#include <string>
#include <queue>
#include <map>

#include "../common/packet_dump.h"
#include "../common/packet_functions.h"
#include "../common/packet_types.h"
#include "../common/PyRep.h"
#include "../common/EVEUnmarshal.h"
#include "../common/PyPacket.h"
#include "../common/PyVisitor.h"
#include "../common/logsys.h"
#include "../common/StreamPacketizer.h"
#include "../common/CachedObjectMgr.h"
#include "../common/PyXMLGenerator.h"
#include "../common/PyDumpVisitor.h"
#include "../common/PyLookupDump.h"

#include "EVECollectDisp.h"

#include "../packets/General.h"

using std::queue;


#define int_ntoa(x)	inet_ntoa(*((struct in_addr *)&x))

// struct tuple4 contains addresses and port numbers of the TCP connections
// the following auxiliary function produces a string looking like
// 10.0.0.1,1024,10.0.0.2,23
char *
adres (struct tuple4 addr)
{
  static char buf[256];
  strcpy (buf, int_ntoa (addr.saddr));
  sprintf (buf + strlen (buf), ",%i,", addr.source);
  strcat (buf, int_ntoa (addr.daddr));
  sprintf (buf + strlen (buf), ",%i", addr.dest);
  return buf;
}

void ProcessCallRequest(PyPacket *packet) {
	
}


static EVECollectDispatcher *CollectDispatcher = NULL;

//PyObject *loadfunc = NULL;

StreamPacketizer clientPacketizer;
StreamPacketizer serverPacketizer;
void tcp_callback (struct tcp_stream *a_tcp, void ** this_time_not_needed) {
	char buf[1024];
	strcpy (buf, adres (a_tcp->addr)); // we put conn params into buf
	
	if (a_tcp->nids_state == NIDS_JUST_EST) {

		//see if this is a stream we care about...
		if(a_tcp->addr.source != 26000 && a_tcp->addr.dest != 26000 &&
		   a_tcp->addr.source != 26001 && a_tcp->addr.dest != 26001)
			return;
		
		a_tcp->client.collect++; // we want data received by a client
		a_tcp->server.collect++; // and by a server, too
		_log(COLLECT__TCP, "%s established", buf);
		return;
	}
	if (a_tcp->nids_state == NIDS_CLOSE) {
		// connection has been closed normally
		_log(COLLECT__TCP, "%s closing", buf);
		return;
	}
	if (a_tcp->nids_state == NIDS_RESET) {
		// connection has been closed by RST
		_log(COLLECT__TCP, "%s reset", buf);
		return;
	}

	if (a_tcp->nids_state == NIDS_DATA) {
		// new data has arrived; gotta determine in what direction
		// and if it's urgent or not

		struct half_stream *hlf;
		StreamPacketizer *sp;

		if (a_tcp->client.count_new) {
			// new data for client
			hlf = &a_tcp->client; // from now on, we will deal with hlf var,
			// which will point to client side of conn
			sp = &clientPacketizer;
			strcat (buf, "(<-)"); // symbolic direction of data
		} else {
			sp = &serverPacketizer;
			hlf = &a_tcp->server; // analogical
			strcat (buf, "(->)");
		}
		
		_log(COLLECT__TCP, "Data %s (len %d)", buf, hlf->count_new); // we print the connection parameters
								  // (saddr, daddr, sport, dport) accompanied
								  // by data flow direction (-> or <-)
			
		sp->InputBytes((const byte *) hlf->data, hlf->count_new);
		
		StreamPacketizer::Packet *p;
		while((p = sp->PopPacket()) != NULL) {
			//const PacketHeader *head = (const PacketHeader *) p->data;
			
			uint32 body_len = p->length;
			const byte *body = p->data;
			
			_log(COLLECT__RAW_HEX, "Raw Hex Dump of len %d:", body_len);
			_hex(COLLECT__RAW_HEX, body, body_len);
			
			PyRep *rep = InflateAndUnmarshal(body, body_len);
			if(rep == NULL) {
				printf("Failed to inflate or unmarshal!");
				delete p;
				continue;
			}

			if(is_log_enabled(COLLECT__PYREP_DUMP)) {
				//decode substreams to facilitate dumping better:
				SubStreamDecoder v;
				rep->visit(&v);
				//TODO: make dump use logsys.
				_log(COLLECT__PYREP_DUMP, "Unmarshaled PyRep:");
				PyLookupDump dumper(&CollectDispatcher->lookResolver, COLLECT__PYREP_DUMP);
				rep->visit(&dumper);
			}
			
			PyPacket *packet = new PyPacket;
			if(!packet->Decode(rep)) {
				_log(COLLECT__ERROR, "Failed to decode packet rep");
			} else {
				if(is_log_enabled(COLLECT__PACKET_DUMP)) {
					//decode substreams to facilitate dumping better:
					SubStreamDecoder v;
					packet->payload->visit(&v);
					
					//TODO: make dump use logsys.
					_log(COLLECT__PACKET_DUMP, "Decoded message:");
					PyLookupDump dumper(&CollectDispatcher->lookResolver, COLLECT__PACKET_DUMP);
					packet->Dump(COLLECT__PACKET_DUMP, &dumper);
					
					
					printf("\n\n");
				}
				fflush(stdout);

				CollectDispatcher->DispatchPacket(&packet);
			}
			delete packet;
			
			delete p;
		} //end "while pop packet"
	}
	return ;
}

int EVE_NIDS_main(EVECollectDispatcher *disp, int argc, char *argv[]) {
	if(disp == NULL) {
		fprintf(stderr,"NULL dispatcher provided to NIDS, not running\n",nids_errbuf);
		return(1);
	}
	CollectDispatcher = disp;
	
	// here we can alter libnids params, for instance:
	// nids_params.n_hosts=256;
	if(argc == 2)
		nids_params.filename = strdup(argv[1]);
	if(argc == 3)	//hack
		nids_params.device = strdup(argv[2]);
	if (!nids_init ()) {
		fprintf(stderr,"%s\n",nids_errbuf);
		return(1);
	}
	
	nids_register_tcp ((void *) tcp_callback);
	printf("Starting NIDS loop...\n");
	nids_run ();
	return(0);
}


