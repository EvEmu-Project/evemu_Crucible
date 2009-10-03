
#include <stdio.h>
#include <math.h>
#include <vector>

typedef unsigned char byte;
typedef long long int64;
typedef unsigned long long uint64;
typedef signed long long sint64;


typedef struct {
	int64 intval;
	double dval;
	float fval;
} FindSpec;

void found_it(int offset, const char *str) {
	printf("Found at offset %d as a %s\n", offset, str);
}

bool FindNumber(const std::vector<byte> &digits, int byte, const FindSpec *f) {

	int maxlen = digits.size() - byte;
	const unsigned char *bptr = (const unsigned char *) &digits[byte];

	if(maxlen >= sizeof(char)) {
		if(*bptr == f->intval) {
			found_it(byte, "byte");
			return(true);
		}
	}

	if(maxlen >= sizeof(short)) {
		const unsigned short *le = (const unsigned short *) bptr;
		signed short sle = *le;
		unsigned short be = ((*le & 0xFF)<<8) | ((*le &0xFF00)>>8);
		signed short sbe = be;

		if(*le == f->intval) {
			found_it(byte, "native short");
			return(true);
		}
		if(sle == f->intval) {
			found_it(byte, "signed native short");
			return(true);
		}
		if(be == f->intval) {
			found_it(byte, "swapped short");
			return(true);
		}
		if(sbe == f->intval) {
			found_it(byte, "signed swapped short");
			return(true);
		}
	}


	if(maxlen >= sizeof(long)) {
		const unsigned long *le = (const unsigned long *) bptr;
		signed long sle = *le;
		unsigned long be = 
			  ((*le & 0x000000FF) <<24) 
			| ((*le & 0x0000FF00L)<<8)
			| ((*le & 0x00FF0000L)>>8)
			| ((*le & 0xFF000000L)>>24)
			;
		signed long sbe = be;

		if(*le == f->intval) {
			found_it(byte, "native long");
			return(true);
		}
		if(sle == f->intval) {
			found_it(byte, "signed native long");
			return(true);
		}
		if(be == f->intval) {
			found_it(byte, "swapped long");
			return(true);
		}
		if(sbe == f->intval) {
			found_it(byte, "signed swapped long");
			return(true);
		}
	}

	
	if(f->fval != 0.0f && maxlen >= sizeof(float)) {
		const float *fv = (const float *) bptr;

		if(isnormal(*fv)) {
			if(*fv == f->fval) {
				found_it(byte, "float");
				return(true);
			}
			if(int64(*fv) == f->intval) {
				found_it(byte, "rounded float");
				return(true);
			}
		}
	}


	if(f->dval != 0.0f && maxlen >= sizeof(double)) {
		const double *d = (const double *) bptr;
		if(isnormal(*d)) {
			if(*d == f->dval) {
				found_it(byte, "double");
				return(true);
			}
			if(int64(*d) == f->intval) {
				found_it(byte, "rounded double");
				return(true);
			}
		}
	}

	if(maxlen >= sizeof(int64)) {
		const int64 *le = (const int64 *) bptr;
		sint64 sle = *le;
		int64 be = 
			  ((*le & 0x00000000000000FFLL)<<56) 
			| ((*le & 0x000000000000FF00LL)<<40)
			| ((*le & 0x0000000000FF0000LL)<<24)
			| ((*le & 0x00000000FF000000LL)<<8)
			| ((*le & 0x000000FF00000000LL)>>8)
			| ((*le & 0x0000FF0000000000LL)>>24)
			| ((*le & 0x00FF000000000000LL)>>40)
			| ((*le & 0xFF00000000000000LL)>>56)
			;
		sint64 sbe = be;
		
		if(*le == f->intval) {
			found_it(byte, "native int64");
			return(true);
		}
		if(sle == f->intval) {
			found_it(byte, "signed native int64");
			return(true);
		}
		if(be == f->intval) {
			found_it(byte, "swapped int64");
			return(true);
		}
		if(sbe == f->intval) {
			found_it(byte, "signed swapped int64");
			return(true);
		}
	}

	return(false);
}

int main(int argc, char *argv[]) {
	if(argc != 3) {
		printf("Usage: %s [number] [file]\n");
		return(1);
	}

	FindSpec s;
	//we dont handle unsigned 64 bit ints very well.
	s.intval = strtol(argv[1], NULL, 0);
	s.dval = strtod(argv[1], NULL);
	s.fval = s.dval;	//only cast it down once.
	
	FILE *f = fopen(argv[2], "rb");
	if(f == NULL) {
		printf("Unable to open input file\n");
		return(1);
	}
	
	int block_size = 102400;
	int find_unit_length = sizeof(int64);
	
	std::vector<byte> curdata;
	std::vector<byte> tail(find_unit_length);
	curdata.reserve(block_size+sizeof(int64)*2);
	while(!feof(f)) {
		int start = curdata.size();
		curdata.resize(block_size+start);
		int len = fread(&curdata[start], 1, block_size, f);
		if(len < 0) {
			printf("Error reading.\n");
			fclose(f);
			return(1);
		} else if(len == 0) {
			continue;
		}
		len += start;
		curdata.resize(len);

		int offset;
		for(offset = 0; offset < len; offset++) {
			FindNumber(curdata, offset, &s);
		}

		//leave one unit worth of the end of the previous block in the
		//front of the buffer to handle block boundaries.
		if(curdata.size() > find_unit_length) {
			memcpy(&tail[0], &curdata[curdata.size()-tail.size()], tail.size());
			curdata.resize(tail.size()-1);
			memcpy(&curdata[0], &tail[1], tail.size()-1);	//drop the first byte to avoid duplicates
		}
	}

	fclose(f);
	
	return(0);
}
