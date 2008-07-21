/*
 * Adapted from Selvin's TriExporter application
 *
 * http://www.agronom.pl/selvin/EVE/TriExporter/tabid/66/Default.aspx
 *
 *
*/

#ifndef TRIEXPORTER_TRIFILE_H
#define TRIEXPORTER_TRIFILE_H

#include <vector>
#include <string>

namespace TriExporter {
	
	typedef unsigned char  byte;
	typedef unsigned short word;
	typedef unsigned int  dword;

#pragma pack(push, 1)

struct Header
{
	
/*000*/	unsigned char versionHi; // always 01
/*001*/	unsigned char versionLo; // always 04
/*002*/	unsigned char unknown1[12];
/*014*/	unsigned long sizeVertex; 
/*018*/	unsigned long numVertices;
/*022*/	unsigned long numSurfaces;
/*026*/	unsigned long numTriangles;
/*030*/	float         unknown30;	//saw 11499.2
/*034*/	float         unknown34;	//saw -772.09
/*038*/	float         unknown38;	//saw -172.11
/*042*/	float         unknown42;	//saw 461.85
/*046*/	float         minx;
/*050*/	float         miny;
/*054*/	float         minz;
/*058*/	float         maxx;
/*062*/	float         maxy;
/*066*/	float         maxz;
/*070*/	unsigned long unknown70;	//saw 3
/*074*/	unsigned long unknown74;	//saw 377462
/*078*/	unsigned long sizeHeader;	//saw 86
/*082*/	unsigned long unknown82;	//saw 377486
/*086*/
};

struct Vertex
{
	float vertexPosition[3];
	float vertexNormal[3];
	float vertexUV[2];
};
//static const word vmaxsize = 128;
template<int size> struct CVertex: Vertex
{
	float unused[(size-32)/4];
};

struct Surface
{
	dword surfaceType;
	dword unknown[3];
	dword startIndex;
	dword numTriangles;
};

struct Triangle
{
	word vertices[3];
};

#pragma pack()

class TriFile
{
public:
	Header header;
	Vertex *m_vertices;
	std::vector<Surface> surfaces;
	std::vector< std::vector <Triangle> > triangles;
	std::vector<std::string> textures;
	dword numTriangles;

	TriFile():m_vertices(NULL), numTriangles(0){}
	void Clear();
	virtual ~TriFile();
	bool LoadFile(std::string filename);
	bool LoadFile(std::ifstream &is);
	//void ExportX(float size, string file, string dir);
	void ExportObj(std::string outobj, std::string outmtl, float size = 1.0);	
	//void Export3ds(float size, string file, string dir);

	void DumpHeaders() const;

protected:
	inline Vertex* vertices(int i) const
	{
		//return &m_vertices[i];
		return reinterpret_cast<Vertex *>(reinterpret_cast<char *>(m_vertices) + i * header.sizeVertex);
	}
};

}


#endif


