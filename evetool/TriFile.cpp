/*
 * Adapted from Selvin's TriExporter application
 *
 * http://www.agronom.pl/selvin/EVE/TriExporter/tabid/66/Default.aspx
 *
 *
*/

#include "TriFile.h"
#include <fstream>
#include <iostream>


namespace TriExporter {
	using namespace std;


void TriFile::Clear()
{
	numTriangles = 0;
	if(m_vertices != NULL)
		delete [] m_vertices;
	m_vertices = NULL;
}

TriFile::~TriFile()
{
	Clear();
}

bool TriFile::LoadFile(string filename)
{
	ifstream is;
	is.sync_with_stdio(false);
	is.open (filename.c_str(), ios::binary|ios::in);
	
	Clear();
	is.read(reinterpret_cast<char*>(&header), sizeof(header));
	if(header.versionLo != 4 && header.versionHi != 1)
		return false;
	surfaces.resize(header.numSurfaces);
	switch(header.sizeVertex)
	{
		case 32:
			m_vertices = new Vertex[header.numVertices];
			break;
		case 40:
			m_vertices = new CVertex<40>[header.numVertices];
			break;
		case 48:
			m_vertices = new CVertex<48>[header.numVertices];
			break;
		case 56:
			m_vertices = new CVertex<56>[header.numVertices];
			break;
		case 64:
			m_vertices = new CVertex<64>[header.numVertices];
			break;
		case 72:
			m_vertices = new CVertex<72>[header.numVertices];
			break;
		default:
			return false;
	}
	
	int r = header.sizeVertex*header.numVertices;
	is.read(reinterpret_cast<char*>(m_vertices), r);
	r = sizeof(surfaces[0])*header.numSurfaces;
	is.read(reinterpret_cast<char*>(&surfaces[0]), r);
	triangles.resize(header.numSurfaces);
	textures.resize(header.numSurfaces);
	for(dword i = 0; i < header.numSurfaces; i++)
	{
		int len = 0;
		r = 0;
		if(surfaces[i].surfaceType == 5)
		{
			len = 2;
			r = len* (surfaces[i].numTriangles+ 2);
		}
		else
		{
			len = 3;
			r = len* surfaces[i].numTriangles*2;
		}
		vector<word> list;
		list.resize(surfaces[i].numTriangles*len);
		is.read(reinterpret_cast<char*>(&list[0]), r);
		triangles[i].resize(surfaces[i].numTriangles);
		int flush = 0;
		if(surfaces[i].surfaceType == 5)
		{
			dword count = 0;
			for(dword c = 0; c <  surfaces[i].numTriangles; c++)
			{

				if(list[c] != list[c+1] && list[c] != list[c+2] && list[c+1] != list[c+2])
				{
				if((flush%2) ==0)
				{
					triangles[i][count].vertices[0] = list[c];
					triangles[i][count].vertices[1] = list[c+1];
					triangles[i][count].vertices[2] = list[c+2];
				}
				else
				{
					triangles[i][count].vertices[0] = list[c+2];
					triangles[i][count].vertices[1] = list[c+1];
					triangles[i][count].vertices[2] = list[c];
				}
				count++;
				}
				flush++;

			}
			surfaces[i].numTriangles = count;
		}
		else
		{
			for(dword c = 0; c <  surfaces[i].numTriangles; c++)
			{
				triangles[i][c].vertices[0] = list[c*3];
				triangles[i][c].vertices[1] = list[c*3+1];
				triangles[i][c].vertices[2] = list[c*3+2];
			}
		}
		numTriangles += surfaces[i].numTriangles;
	}
	is.close();
	return true;
}

/*
void TriFile::ExportX(float size, string file, string dir)
{
	string xheader = "xof 0303txt 0032\ntemplate Vector {\n <3d82ab5e-62da-11cf-ab39-0020af71e433>\n FLOAT x;\n FLOAT y;\n FLOAT z;\n}\n\ntemplate MeshFace {\n <3d82ab5f-62da-11cf-ab39-0020af71e433>\n DWORD nFaceVertexIndices;\n array DWORD faceVertexIndices[nFaceVertexIndices];\n}\n\ntemplate Mesh {\n <3d82ab44-62da-11cf-ab39-0020af71e433>\n DWORD nVertices;\n array Vector vertices[nVertices];\n DWORD nFaces;\n array MeshFace faces[nFaces];\n [...]\n}\n\ntemplate MeshNormals {\n <f6f23f43-7686-11cf-8f52-0040333594a3>\n DWORD nNormals;\n array Vector normals[nNormals];\n DWORD nFaceNormals;\n array MeshFace faceNormals[nFaceNormals];\n}\n\ntemplate Coords2d {\n <f6f23f44-7686-11cf-8f52-0040333594a3>\n FLOAT u;\n FLOAT v;\n}\n\ntemplate MeshTextureCoords {\n <f6f23f40-7686-11cf-8f52-0040333594a3>\n DWORD nTextureCoords;\n array Coords2d textureCoords[nTextureCoords];\n}\n\ntemplate ColorRGBA {\n <35ff44e0-6c7c-11cf-8f52-0040333594a3>\n FLOAT red;\n FLOAT green;\n FLOAT blue;\n FLOAT alpha;\n}\n\ntemplate ColorRGB {\n <d3e16e81-7835-11cf-8f52-0040333594a3>\n FLOAT red;\n FLOAT green;\n FLOAT blue;\n}\n\ntemplate Material {\n <3d82ab4d-62da-11cf-ab39-0020af71e433>\n ColorRGBA faceColor;\n FLOAT power;\n ColorRGB specularColor;\n ColorRGB emissiveColor;\n [...]\n}\n\ntemplate MeshMaterialList {\n <f6f23f42-7686-11cf-8f52-0040333594a3>\n DWORD nMaterials;\n DWORD nFaceIndexes;\n array DWORD faceIndexes[nFaceIndexes];\n [Material <3d82ab4d-62da-11cf-ab39-0020af71e433>]\n}\n\ntemplate TextureFilename {\n <a42790e1-7810-11cf-8f52-0040333594a3>\n STRING filename;\n}\n\n\nMesh {\n";
	string xmaterial1 = "  Material {\n   1.000000;1.000000;1.000000;1.000000;;\n   256.000000;\n   1.000000;1.000000;1.000000;;\n   0.000000;0.000000;0.000000;;\n\n   TextureFilename {\n    \"";
	string xmaterial2 = "\";\n   }\n  }\n";
	string xfooter = " }\n}";
	string xnormals = " MeshNormals {";
	string xlendlast = ";;";
	string xlend = ";,";
	ofstream out;
	out.sync_with_stdio(false);
	out.open((dir + file + ".x").c_str());
	out << xheader << endl;
	out << header.numVertices << ";" << endl;
	for(dword i = 0; i < header.numVertices; i ++)
	{
		if((i+1) != header.numVertices)
			out << " " << fixed << setprecision(6) << vertices(i)->vertexPosition[0]*size << ";" << setprecision(6) << vertices(i)->vertexPosition[1]*size << ";" << setprecision(6) << vertices(i)->vertexPosition[2]*size << xlend <<endl;
		else
			out << " " << setprecision(6) << vertices(i)->vertexPosition[0]*size << ";" << setprecision(6) << vertices(i)->vertexPosition[1]*size << ";" << setprecision(6) << vertices(i)->vertexPosition[2]*size << xlendlast <<endl;
	}
	out << numTriangles << ";" << endl;
	for(dword i = 0; i < header.numSurfaces; i++)
	{
		for(dword c = 0; c < surfaces[i].numTriangles; c ++)
		{
			if(((i+1) == header.numSurfaces) && ((c+1) == surfaces[i].numTriangles))
				out << " 3;" << triangles[i][c].vertices[0] << "," << triangles[i][c].vertices[1] << "," << triangles[i][c].vertices[2] << xlendlast << endl;
			else
				out << " 3;" << triangles[i][c].vertices[0] << "," << triangles[i][c].vertices[1] << "," << triangles[i][c].vertices[2] << xlend << endl;
		}
	}
	out << xnormals << endl;
	out << "  " << header.numVertices << ";" << endl;
	for(dword i = 0; i < header.numVertices; i ++)
	{
		if((i+1) != header.numVertices)
			out << "  "  << fixed << setprecision(6) << vertices(i)->vertexNormal[0] << ";" << setprecision(6) << vertices(i)->vertexNormal[1] << ";" << setprecision(6) << vertices(i)->vertexNormal[2] << xlend << endl;
		else
			out << "  " << setprecision(6) << vertices(i)->vertexNormal[0] << ";" << setprecision(6) << vertices(i)->vertexNormal[1] << ";" << setprecision(6) << vertices(i)->vertexNormal[2] << xlendlast << endl;
	}
	out << numTriangles << ";" << endl;
	for(dword i = 0; i < header.numSurfaces; i++)
	{
		for(dword c = 0; c < surfaces[i].numTriangles; c ++)
		{
			if(((i+1) == header.numSurfaces) && ((c+1) == surfaces[i].numTriangles))
				out << " 3;" << triangles[i][c].vertices[0] << "," << triangles[i][c].vertices[1] << "," << triangles[i][c].vertices[2] << xlendlast << endl;
			else
				out << " 3;" << triangles[i][c].vertices[0] << "," << triangles[i][c].vertices[1] << "," << triangles[i][c].vertices[2] << xlend << endl;
		}
	}
	out << "}\n\n MeshTextureCoords {" << endl;
	out << "  " <<header.numVertices << ";" << endl;
	for(dword i = 0; i < header.numVertices; i ++)
	{
		if((i+1) != header.numVertices)
			out << fixed << setprecision(6) <<  vertices(i)->vertexUV[0] << "; " << setprecision(6) <<  vertices(i)->vertexUV[1] << xlend << endl;
		else
			out << setprecision(6) << vertices(i)->vertexUV[0] << "; " << setprecision(6) <<  vertices(i)->vertexUV[1] << xlendlast << endl;
	}
	out << "}\n\nMeshMaterialList {\n" << header.numSurfaces << ";\n" << numTriangles << ";" << endl;
	for(dword i = 0; i < header.numSurfaces; i++)
	{
		for(dword c = 0; c < surfaces[i].numTriangles; c++)
		{
			if((c+1) != surfaces[i].numTriangles)
				out << i << ","  << endl;
			else
				out << i << ";" << endl;
		}
	}
	for(dword c = 0; c < header.numSurfaces; c++)
	{
		out << xmaterial1 << textures[c] << xmaterial2;
	}
	out << xfooter;
	out.close();
}*/


void TriFile::ExportObj(string outobj, string outmtln, float size)
{
	ofstream out;
	out.sync_with_stdio(false);
	ofstream outmtl;
	outmtl.sync_with_stdio(false);
	out.open(outobj.c_str());
	outmtl.open(outmtln.c_str());

	
	out << "mtllib " << outmtln << endl;
	out << "g shape" << endl;
	for(dword i = 0; i < header.numVertices; i ++)
		out << "v " << vertices(i)->vertexPosition[0]*size << " " << vertices(i)->vertexPosition[1]*size << " " << vertices(i)->vertexPosition[2]*size << endl;
	for(dword i = 0; i < header.numVertices; i ++)
		out << "vt " << vertices(i)->vertexUV[0] << " " << vertices(i)->vertexUV[1] << endl;
	for(dword i = 0; i < header.numVertices; i ++)
		out << "vn " << vertices(i)->vertexNormal[0] << " " << vertices(i)->vertexNormal[1] << " " << vertices(i)->vertexNormal[2] << endl;
	for(dword i = 0; i < header.numSurfaces; i++)
	{
		out << "usemtl shape" << i << endl;
		outmtl <<"newmtl shape" << i << "\nKa 0.5 0.5 0.5\nKd 1 1 1\nKs 1 1 1\nillum 7\nNs 256\nmap_Kd " << textures[i] << "\n";
		for(dword c = 0; c < surfaces[i].numTriangles; c ++)
		{
			word t1 = triangles[i][c].vertices[0] + 1;
			word t2 = triangles[i][c].vertices[1] + 1;
			word t3 = triangles[i][c].vertices[2] + 1;
			if(! (t1 == t2  || t2 == t3 || t1 == t3))
				out << "f " <<  t1 << "/" << t1 << "/" << t1 << " " << t2 << "/" << t2 << "/" << t2 << " " << t3 << "/" << t3 << "/" << t3 << endl;
		}
	}
	out.close();
	outmtl.close();
}

/*
void TriFile::Export3ds(float size, string file, string dir)
{
	Lib3dsFile *ds3 = lib3ds_file_new();
	Lib3dsMesh *mesh = lib3ds_mesh_new("shape");
	lib3ds_mesh_new_point_list(mesh, header.numVertices);
	lib3ds_mesh_new_texel_list(mesh, header.numVertices);
	Lib3dsPoint* pointL = mesh->pointL;
	for(dword i = 0; i < header.numVertices; i ++)
	{
        mesh->pointL[i].pos[0] = vertices(i)->vertexPosition[0]*size;
        mesh->pointL[i].pos[1] = vertices(i)->vertexPosition[1]*size;
        mesh->pointL[i].pos[2] = vertices(i)->vertexPosition[2]*size;
		mesh->texelL[i][0] = vertices(i)->vertexUV[0];
		mesh->texelL[i][1] = vertices(i)->vertexUV[1];
	}
	int count = 0;
	lib3ds_mesh_new_face_list(mesh, numTriangles);
	for(dword i = 0; i < header.numSurfaces; i++)
	{
		Lib3dsMaterial *canvas = lib3ds_material_new();
		stringstream tmp;
		string name;
		tmp << i;
		tmp >> name;
		name = "shape" + name;
		strncpy(canvas->name, name.c_str(), sizeof(canvas->name));
		canvas->ambient[0] = canvas->ambient[1] = canvas->ambient[2] = 1.f;
		canvas->diffuse[0] = canvas->diffuse[1] = canvas->diffuse[2] = 1.f;
		canvas->specular[0] = canvas->specular[1] = canvas->specular[2] = 0.5f;
		canvas->two_sided = 0;
		strncpy(canvas->texture1_map.name, textures[i].c_str(), sizeof(canvas->texture1_map.name));
		strncpy(canvas->texture2_map.name, textures[i].c_str(), sizeof(canvas->texture2_map.name));
		lib3ds_file_insert_material(ds3, canvas);
		for(dword c = 0; c < surfaces[i].numTriangles; c ++)
		{
				strncpy(mesh->faceL[count].material, name.c_str(), sizeof(mesh->faceL[count].material));
				mesh->faceL[count].points[0] = triangles[i][c].vertices[0];
				mesh->faceL[count].points[1] = triangles[i][c].vertices[1];
				mesh->faceL[count].points[2] = triangles[i][c].vertices[2];	
				count++;
		}
	}
	lib3ds_file_insert_mesh(ds3, mesh);
	ds3->meshes = mesh;
	lib3ds_file_save(ds3, (dir + file + ".3ds").c_str());
	lib3ds_file_free(ds3);
}*/

void TriFile::DumpHeaders() const {
	cout << "TRI Headers:" << endl;
	cout << "    sizeVertex: " << header.sizeVertex << endl;
	cout << "    numVertices: " << header.numVertices << endl;
	cout << "    numSurfaces: " << header.numSurfaces << endl;
	cout << "    numTriangles: " << header.numTriangles << endl;
	cout << "    unknown30: " << header.unknown30 << endl;
	cout << "    unknown34: " << header.unknown34 << endl;
	cout << "    unknown38: " << header.unknown38 << endl;
	cout << "    unknown42: " << header.unknown42 << endl;
	cout << "    X range: " << 
		header.minx << " -> " << header.maxx << endl;
	cout << "    Y range: " << 
		header.miny << " -> " << header.maxy << endl;
	cout << "    Z range: " << 
		header.minz << " -> " << header.maxz << endl;
	cout << "    unknown70: " << header.unknown70 << endl;
	cout << "    unknown74: " << header.unknown74 << endl;
	cout << "    sizeHeader: " << header.sizeHeader << endl;
	cout << "    unknown82: " << header.unknown82 << endl;

	std::vector<Surface>::const_iterator cur, end;
	cur = surfaces.begin();
	end = surfaces.end();
	int r;
	for(r = 0; cur != end; cur++, r++) {
		cout << "Surface #" << r << endl;
		cout << "    Type: " << cur->surfaceType << endl;
		cout << "    u4: " << cur->unknown[0] << endl;
		cout << "    u8: " << cur->unknown[1] << endl;
		cout << "    u12: " << cur->unknown[2] << endl;
		cout << "    startIndex: " << cur->startIndex << endl;
		cout << "    numTriangles: " << cur->numTriangles << endl;
	}
}


}



