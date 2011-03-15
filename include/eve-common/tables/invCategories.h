#ifndef EVEDB_invCategories_H__
#define EVEDB_invCategories_H__

namespace EVEDB {
 namespace invCategories {

typedef enum {
	_System = 0,
	Owner = 1,
	Celestial = 2,
	Station = 3,
	Material = 4,
	Accessories = 5,
	Ship = 6,
	Module = 7,
	Charge = 8,
	Blueprint = 9,
	Trading = 10,
	Entity = 11,
	Bonus = 14,
	Skill = 16,
	Commodity = 17,
	Drone = 18,
	Implant = 20,
	Deployable = 22,
	Structure = 23,
	Reaction = 24,
	Asteroid = 25,
    Interiors = 26,
    Placeables = 27,
    Abstract = 29,
    Subsystem = 32,
    AncientRelics = 34,
    Decryptors = 35
 } invCategories;

 }
}

#endif

