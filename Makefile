
all:
	$(MAKE) -C collector
	$(MAKE) -C evetool
	$(MAKE) -C server

wine:
	$(MAKE) -C collector -f makefile.wine
	$(MAKE) -C evetool -f makefile.wine
	$(MAKE) -C server -f makefile.wine

wine_clean:
	$(MAKE) -C collector -f makefile.wine clean
	$(MAKE) -C evetool -f makefile.wine clean
	$(MAKE) -C server -f makefile.wine clean

clean:
	$(MAKE) -C collector clean
	$(MAKE) -C evetool clean
	$(MAKE) -C server clean


