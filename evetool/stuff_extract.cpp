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

#include "../common/common.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <vector>
#include <string>

#ifdef WIN32 

#define S_IRWXU 0
#define S_IRWXG 0
#define S_IRWXO 0
int mkdir(const char *pathname, int mode) 
{ 
  int result;


  result = CreateDirectory(pathname, NULL); 


  /* mkdir returns 0 for success, opposite of CreateDirectory() */ 
  return result?0:-1; 
} 

#endif



#pragma pack(1)
typedef struct {
	uint32 file_size;
	uint32 path_len;
} FileHeader;
#pragma pack()

class FileHeaderObj {
public:
	uint32 length;
	uint32 offset;
	std::string filename;
};

using std::vector;
using std::string;


void split_path(const string &path, vector<string> &components) {
	components.clear();
	const char *p = path.c_str();
	const char *begin = p;
	int len = 0;
	while(*p != '\0') {
		if(*p == '/' || *p == '\\') {
			components.push_back(string(begin, len));
			len = 0;
			begin = p+1;
		} else {
			len++;
		}
		p++;
	}
	if(begin < p)
		components.push_back(string(begin, len));
}

int main(int argc, char *argv[]) {
	if(argc != 2) {
		printf("Usage: %s [.stuff file]\n", argv[0]);
		return(1);
	}
	
	FILE *in = fopen(argv[1], "rb");
	if(in == NULL) {
		printf("Unable to open %s\n", argv[1]);
		return(1);
	}

	uint32 base_offset = 0;
	uint32 file_count;
	if(fread(&file_count, sizeof(file_count), 1, in) != 1) {
		printf("Unable to read file count\n");
		return(1);
	}
	base_offset += sizeof(file_count);

	printf("There are %d files in this stuff archive\n", file_count);

	vector<FileHeaderObj> headers;
	headers.resize(file_count);

	uint32 offset = 0;
	char *path = new char[2048];
	uint32 r;
	FileHeader head;
	for(r = 0; r < file_count; r++) {
		if(fread(&head, sizeof(head), 1, in) != 1) {
			printf("Unable to read file %d's header\n", r);
			return(1);
		}
		base_offset += sizeof(head);
		head.path_len++;	//NULL terminator
		
		//read path name.
		if(fread(path, 1, head.path_len, in) != head.path_len) {
			printf("Unable to read file %d's path name\n", r);
			return(1);
		}
		base_offset += sizeof(file_count);

		FileHeaderObj &obj = headers[r];
		obj.length = head.file_size;
		obj.filename = path;
		obj.offset = offset;
		offset += head.file_size;

		printf("File %d has length %d and path %s\n", r, obj.length, obj.filename.c_str());
	}
	delete[] path;

	struct stat stat_struct;
	vector<FileHeaderObj>::iterator cur, end;
	cur = headers.begin();
	end = headers.end();
	for(; cur != end; cur++) {

		//split the path into components, make the intermediate directories
		//if needed, and then finally open the file and write it out.
		string pathname = ".";
		vector<string> components;
		split_path(cur->filename, components);
		vector<string>::iterator cc, ec;
		cc = components.begin();
		ec = components.end();
		while(cc != ec) {
			const string &cs = *cc;
			cc++;
			pathname += "/";
			pathname += cs;
			if(cc == ec) {
				//this is the file component.
				FILE *out = fopen(pathname.c_str(), "wb");
				if(out == NULL) {
					printf("Unable to open file %s from archive: %s", pathname.c_str(), strerror(errno));
					break;
				}
				printf("Extracting %s of length %d\n", pathname.c_str(), cur->length);
				char *buffer = new char[cur->length];
				if(fread(buffer, 1, cur->length, in) != cur->length) {
					printf("Unable to read file %s from archive: %s", cur->filename.c_str(), strerror(errno));
					break;
				}

				if(fwrite(buffer, 1, cur->length, out) != cur->length) {
					printf("Unable to write file %s: %s", pathname.c_str(), strerror(errno));
					break;
				}

				fclose(out);
				
			} else {
				//this is an intermediate directory.
				//printf("DIR %s\n", pathname.c_str());
				if(stat(pathname.c_str(), &stat_struct) == -1) {
					if(errno == ENOENT) {
						if(mkdir(pathname.c_str(), S_IRWXU|S_IRWXG|S_IRWXO) == -1) {
							printf("Failed to make intermediate directory %s: %s", pathname.c_str(), strerror(errno));
							break;
						}
					} else {
						printf("Unable to stat %s: %s", pathname.c_str(), strerror(errno));
					}
				}
			}
		}

		
	}
	

	return(0);
}

















