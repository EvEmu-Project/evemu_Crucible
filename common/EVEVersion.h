#ifndef __EVE_VERSION_H
#define __EVE_VERSION_H

#ifdef DRAGON
static const uint8 MachoNetVersion = 151;
static const double EVEVersionNumber = 2.13;
static const uint32 EVEBuildVersion = 4561;
static const char *const EVEProjectVersion = "Dragon@ccp";
static const char *const EVEProjectCodename = "Dragon";
static const char *const EVEProjectRegion = "ccp";
#else
static const uint8 MachoNetVersion = 139;
static const double EVEVersionNumber = 2.1;
static const uint32 EVEBuildVersion = 3913;
static const char *const EVEProjectVersion = "Blood";
#endif

static const uint32 EVEBirthday = 170472;


#endif



