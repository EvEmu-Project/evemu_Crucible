#ifndef __XMLPACKETGEN_H_INCL__
#define __XMLPACKETGEN_H_INCL__

#include <tinyxml.h>

class XMLPacketGen {
public:
	XMLPacketGen();
	virtual ~XMLPacketGen();

	bool GenPackets(const char *xml_file, const char *out_h, const char *out_cpp);
};

#endif//__XMLPACKETGEN_H_INCL__
