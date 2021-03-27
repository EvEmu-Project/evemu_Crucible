#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>
#include <Windows.h>
#include <stdio.h>

#include "util.h"
#include "bluekey.h"
#include "eve.h"

struct Patch {
	char *name;
	char *original;
	char *updated;
	size_t size;
};

char *original_sign_key_hex =
	"\x06\x02\x00\x00\x00\x24\x00\x00\x52\x53\x41\x31\x00\x04\x00\x00\x01\x00\x01\x00\x25\xa1\x20\x0c"
	"\xd9\x4c\xe9\x32\xca\x9b\x7c\x8b\x56\x32\xd2\x63\xb7\x41\x75\xc3\xc7\xf5\xd2\xcd\x07\x3a\xee\x91"
	"\x92\x12\x10\x0d\x1a\x0e\xc3\xab\x84\x61\x94\x2d\xc2\xd3\xcb\x0b\x92\xb3\x10\x87\x57\xd0\x44\xc8"
	"\x9f\x15\x77\x88\x33\x02\x9e\xbf\x25\x12\x32\xdf\x50\x93\x80\x41\x7c\x3c\xc4\x1d\xd5\x04\x83\xf3"
	"\x57\x1c\x15\x09\x0c\x04\xd4\xfd\xaa\xce\x12\xab\xb7\x7f\x74\xa6\x23\xd3\xe3\xa6\x6b\xaa\xfb\xbd"
	"\xb7\xde\xad\x0f\x77\xd7\x23\x92\x7e\x2a\x57\xc7\xb4\xb5\x67\xbd\x65\xeb\x51\x7a\x7f\x0c\x81\x41\xa5\x38\xf5";

char *manifest_verify_func = "\x0F\x85\x1C\x01";

char *manifest_verify_func_org = "\xe9\x1d\x01\x00";


Patch patches[] = {
	{"code signing", original_sign_key_hex, (char *)codeSigKey, 148},
	{"VerifyManifestFile", manifest_verify_func, manifest_verify_func_org, 4},
	{NULL, NULL, NULL, NULL}
};

int unpatch_blue() {
	FILE *f = fopen("bin/blue.dll", "rb");
	assert(f != NULL);

	fseek(f, 0, SEEK_END);
	size_t file_size = ftell(f);
	rewind(f);
	char *buf = (char *)malloc(file_size);
	fread(buf, 1, file_size, f);

	fclose(f);

	Patch *p = patches;
	char logBuf[256];
	while (p->name != NULL) {
		char *loc = (char *)memmem(buf, file_size, p->updated, p->size - 1);
		if (loc == NULL) {
			snprintf(logBuf, 255, "Failed to patch %s pattern not found\n", p->name);
			log_queue.push(logBuf);
		} else {
			memcpy(loc, p->original, p->size);
		}
		p++;
	}
	FILE *b = fopen("bin/blue.dll", "wb");
	fwrite(buf, 1, file_size, b);
	fflush(f);
	fclose(b);

	return 0;
}

int patch_blue() {
	FILE *f = fopen("bin/blue.dll", "rb");
	assert(f != NULL);

	fseek(f, 0, SEEK_END);
	size_t file_size = ftell(f);
	rewind(f);
	char *buf = (char *)malloc(file_size);
	fread(buf, 1, file_size, f);

	fclose(f);

	Patch *p = patches;
	char logBuf[256];
	while (p->name != NULL) {
		char *loc = (char *)memmem(buf, file_size, p->original, p->size - 1);
		if (loc == NULL) {
			snprintf(logBuf, 255, "Failed to patch %s pattern not found\n", p->name);
			log_queue.push(logBuf);
		} else {
			memcpy(loc, p->updated, p->size);
		}
		p++;
	}

	FILE *b = fopen("bin/blue.dll", "wb");
	fwrite(buf, 1, file_size, b);
	fflush(f);
	fclose(b);

	return 0;
}
