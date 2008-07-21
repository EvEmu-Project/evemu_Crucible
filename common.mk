HCC=$(shell ccache -V 2>/dev/null)
ifneq (,$(findstring version,$(HCC)))
	CC=ccache g++
else
	CC=g++
endif

$(A): $(GENOBJS) $(OBJS) $(EXTRA_STATIC_LIBS)
	g++ $(OBJS) $(GENOBJS) $(LDFLAGS) $(MYSQL_LIBS) $(EXTRA_STATIC_LIBS) -o $@ 

$(L): $(OBJS)
	ar rc $(L) $(OBJS)
	ranlib $(L)

clean:
	rm -f $(A) $(OBJS) $(GENOBJS)

.obj/%.o: ../common/%.cpp
	mkdir -p .obj
	$(CC) $(CXXFLAGS) -c $< -o $@

%.o: %.cpp
	$(CC) $(CXXFLAGS) -c $< -o $@

%.cpp: %.xmlp ../xmlpktgen/xmlpktgen
	../xmlpktgen/xmlpktgen $<

%.h: %.xmlp ../xmlpktgen/xmlpktgen
	../xmlpktgen/xmlpktgen $<

.depend depend:
	for f in $(OBJS) $(GENOBJS); \
	do \
		i=`echo $$f | sed "s#\\.o#.cpp#" `; \
		gcc -MM $(CXXFLAGS) $$i | sed "s#^[^ ]*:#$$f:#g"; \
	done  >.depend

