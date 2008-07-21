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


#include "StreamPacketizer.h"


StreamPacketizer::Packet::Packet() {
	length = 0;
	data = NULL;
}
StreamPacketizer::Packet::Packet(uint32 len) {
	length = len;
	if(len > 0)
		data = new byte[len];
	else
		data = NULL;
}
StreamPacketizer::Packet::~Packet() {
	delete[] data;
}

StreamPacketizer::Packet *StreamPacketizer::Packet::Clone() const {
	Packet *p = new Packet(length);
	if(length > 0)
		memcpy(p->data, data, length);
	return(p);
}
	
StreamPacketizer::StreamPacketizer() {
	m_partial = NULL;
	m_payloadLength = 0;
}

StreamPacketizer::~StreamPacketizer() {
	ClearBuffers();
}

void StreamPacketizer::ClearBuffers() {
	delete[] m_partial;
	m_partial = NULL;
	
	while(!m_packets.empty()) {
		delete m_packets.front();
		m_packets.pop();
	}
}

void StreamPacketizer::InputBytes(const byte *data, uint32 len) {
	while(len > 0) {
//fprintf(stderr, "%p: IB len %d partial %p %d/%d\n", this, len, m_partial, m_partialLength, m_payloadLength);
		if(m_partial == NULL) {
			//no partial packet to start with...
			if(len < sizeof(uint32)) {
				//not even enough for a length, just save it.
				m_partial = new byte[sizeof(uint32)];
				memcpy(m_partial, data, len);
				m_partialLength = len;
				return;
			}
	
			m_payloadLength = *((const uint32 *) data);
			len -= sizeof(uint32);
			data += sizeof(uint32);

			if(len < m_payloadLength) {
				//we do not have the entire packet.
				m_partial = new byte[m_payloadLength];
				memcpy(m_partial, data, len);
				m_partialLength = len;
//fprintf(stderr, "%p: New partial packet %d/%d received.\n", this, m_partialLength, m_payloadLength);
				break;
			}
			
			//else, we have an entire packet, process it.
			Packet *sp = new Packet(m_payloadLength);
			memcpy(sp->data, data, m_payloadLength);
			m_packets.push(sp);
			len -= m_payloadLength;
			data += m_payloadLength;
			m_partialLength = 0;
			m_payloadLength = 0;
			//let the loop carry us around to hit this code again.
		} else {
			//we have a partial packet to work from.
			if(m_payloadLength == 0) {
				//we have a partial length, no data yet.
				if((m_partialLength+len) < sizeof(uint32)) {
					//STILL do not have an entire length.
					memcpy(m_partial+m_partialLength, data, len);
					m_partialLength += len;
					break;
				}

				//copy in what we need.
				uint32 need_bytes = sizeof(uint32) - m_partialLength;
				memcpy(m_partial+m_partialLength, data, need_bytes);
				len -= need_bytes;
				data += need_bytes;
				
				m_payloadLength = *((const uint32 *) m_partial);
				delete[] m_partial;
				m_partial = NULL;
				m_partialLength = 0;
				
				//let the loop handle the data segment now.
			} else {
				//we have a partial packet body....
				if((m_partialLength+len) < m_payloadLength) {
					//we do not have the entire packet yet, append and keep going.
					memcpy(m_partial+m_partialLength, data, len);
					m_partialLength += len;
					break;
				}

				//we have an entire packet

				//copy in the rest of the partial packet.
				uint32 need_bytes = m_payloadLength - m_partialLength;
				memcpy(m_partial+m_partialLength, data, need_bytes);
				len -= need_bytes;
				data += need_bytes;
				
				Packet *sp = new Packet();
				sp->data = m_partial;
				sp->length = m_payloadLength;
				m_packets.push(sp);
				m_partial = NULL;
				m_partialLength = 0;
				m_payloadLength = 0;

				//let the loop handle the remaining bytes.
			} //end "we have a partial data segment"
		} //end "we have a partial packet"
	} //end "while we have bytes"
}

StreamPacketizer::Packet *StreamPacketizer::PopPacket() {
	if(m_packets.empty())
		return(NULL);
	Packet *r = m_packets.front();
	m_packets.pop();
	return(r);
}






