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
    WorldSpace = 26,
    Abstract = 29,
	Apparel = 30,
    Subsystem = 32,
    AncientRelics = 34,
    Decryptors = 35,
	InfrastructureUpgrades = 39,
	SovereigntyStructures = 40,
	PlanetaryInteraction = 41,
	PlanetaryResources = 42,
	PlanetaryCommodities = 43,
	Orbitals = 46,
	Placeables = 49,
	Effects = 53,
	Lights = 54,
	Cells = 59
 } invCategories;

 }
}

#endif

