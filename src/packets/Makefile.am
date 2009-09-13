INCLUDES += -I$(srcdir)/../../include/common/

# Define rule for turning .xmlp files into source files
SUFFIXES = \
	.xmlp

.xmlp.cpp:
	@echo Generating $^ -\> $@
	../xmlpktgen/xmlpktgen $^

noinst_LIBRARIES = \
	libpackets.a
# Tmp disable, not needed and screws things up
#	libauth.a

#libauth_a_SOURCES = \
#	Authentication.xmlp

libpackets_a_SOURCES = \
	AccountPkts.xmlp \
	Character.xmlp \
	CorporationPkts.xmlp \
	Crypto.xmlp \
	Destiny.xmlp \
	DogmaIM.xmlp \
	Inventory.xmlp \
	General.xmlp \
	Language.xmlp \
	LSCPkts.xmlp \
	Manufacturing.xmlp \
	Market.xmlp \
	Missions.xmlp \
	ObjectCaching.xmlp \
	Standing2.xmlp \
	Tutorial.xmlp \
	Wallet.xmlp
#	SkillPkts.xmlp

# All generated source files should be cleanable
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
#	SkillPkts.cpp
#	SkillPkts.h

