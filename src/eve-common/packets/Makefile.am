# We must use AM_CXXFLAGS, libpackets_a_CXXFLAGS breaks compilation
AM_CXXFLAGS += \
	$(MYSQL_CFLAGS) \
	-I$(srcdir)/../../../dep/ \
	-I$(srcdir)/../../../include/common/ \
	-I$(srcdir)/../../../include/eve-common/

# We need to define rule for generating packets
SUFFIXES = \
	.xmlp

.xmlp.cpp:
	@echo Generating $^ -\> $@
	$(srcdir)/../../xmlpktgen/xmlpktgen $^

noinst_LIBRARIES = \
	libpackets.a

libpackets_a_SOURCES = \
	General.xmlp \
	ObjectCaching.xmlp \
	AccountPkts.xmlp \
	Character.xmlp \
	CorporationPkts.xmlp \
	Crypto.xmlp \
	Destiny.xmlp \
	DogmaIM.xmlp \
	Inventory.xmlp \
	Language.xmlp \
	LSCPkts.xmlp \
	Manufacturing.xmlp \
	Market.xmlp \
	Missions.xmlp \
	Standing2.xmlp \
	Tutorial.xmlp \
	Wallet.xmlp

MOSTLYCLEANFILES = \
	AccountPkts.cpp \
	AccountPkts.h \
	Character.cpp \
	Character.h \
	CorporationPkts.cpp \
	CorporationPkts.h \
	Crypto.cpp \
	Crypto.h \
	Destiny.cpp \
	Destiny.h \
	DogmaIM.cpp \
	DogmaIM.h \
	Inventory.cpp \
	Inventory.h \
	General.cpp \
	General.h \
	Language.cpp \
	Language.h \
	LSCPkts.cpp \
	LSCPkts.h \
	Manufacturing.cpp \
	Manufacturing.h \
	Market.cpp \
	Market.h \
	Missions.cpp \
	Missions.h \
	ObjectCaching.cpp \
	ObjectCaching.h \
	Standing2.cpp \
	Standing2.h \
	Tutorial.cpp \
	Tutorial.h \
	Wallet.cpp \
	Wallet.h
