#!/usr/bin/python

import base64
import dircache
import os
import os.path
import pprint

def lsdir(dir):
    res = []
    directories = [dir]
    while len(directories)>0:
        directory = directories.pop()
        for name in os.listdir(directory):
            fullpath = os.path.join(directory,name)
            if os.path.isfile(fullpath):
                res.append(fullpath)                # That's a file. Do something with it.
            elif os.path.isdir(fullpath):
                directories.append(fullpath)  # It's a directory, store it.
    return res

pp = pprint.PrettyPrinter(indent=4)

for p in lsdir("/home/jonesa/eve/data/cache/MachoNet"):
	f = os.path.basename(p)
	f = f[:-6];
	try:
		decoded = base64.decodestring(f+"=======")
	except:
		decoded = "--------ERROR"
#	decoded = decoded[7:]
	print f + " -> " + pp.pformat(decoded)

